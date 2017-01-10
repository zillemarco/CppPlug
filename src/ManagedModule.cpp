#ifdef SUPPORT_MANAGED

#include "ManagedModule.h"
#include "ManagedPlugin.h"
#include "ModulesManager.h"
#include "Path.h"

#include "ModuleTools.hpp"

#include <fstream>
#include <sstream>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

struct ManagedModuleInternalData
{
	MonoMethod* _loadModuleFunc;		/** Pointer to the function that gets called to load the module */
	MonoMethod* _unloadModuleFunc;		/** Pointer to the function that gets called to unload the module */
	MonoMethod* _reloadModuleFunc;		/** Pointer to the function that gets called to reload the module */

	MonoAssembly* _moduleAssembly;		/** Assembly of the loaded module */
	MonoImage* _moduleImage;			/** Image of the loaded module */
};

struct MonoInitilizationData
{
	MonoDomain* _domain;
};

MonoInitilizationData* ManagedModule::s_monoInitilizationData = nullptr;
bool ManagedModule::s_monoInitialized = false;

static char* ReadFile(const char* path, size_t& fileSize)
{
	std::ifstream file(path, std::ifstream::binary);
	if (file)
	{
		// Get the file size
		file.seekg(0, file.end);
		auto length = file.tellg();
		fileSize = length;
		file.seekg(0, file.beg);

		// Read the actual data
		char* content = new char[length];
		file.read(content, length);
		file.close();

		return content;
	}

	return nullptr;
}

static MonoMethod* FindMethod(MonoClass* klass, const char* name, int paramsCount = -1)
{
	return mono_class_get_method_from_name(klass, name, paramsCount);
}

ManagedModule::ManagedModule(const ModuleInfo& moduleInfo, const std::string& basePath)
	: Module(moduleInfo, basePath)
	, _internalData(new ManagedModuleInternalData())
{
	_internalData->_loadModuleFunc = nullptr;
	_internalData->_unloadModuleFunc = nullptr;
	_internalData->_reloadModuleFunc = nullptr;
	_internalData->_moduleAssembly = nullptr;
	_internalData->_moduleImage = nullptr;

	_infos._reserved_createPluginFunc = CreatePlugin;
	_infos._reserved_destroyPluginFunc = DestroyPlugin;
	_infos._reserved_sendMessageToPluginFunc = SendMessageToPlugin;
	_infos._reserved_module = this;
}

ManagedModule::~ManagedModule()
{
	std::vector<Module*> localDependantModules = _dependantModules;

	Unload();

	for (auto& el : localDependantModules)
		el->OnDependencyDestructed(this);

	for (auto& el : *_pluginsDataToRestoreAfterReload)
		delete el;

	delete _internalData;
}

bool ManagedModule::InitializeMono(const std::string& domainName, const std::string& monoAssemblyDir, const std::string& monoConfigDir, std::string& error)
{
	if (s_monoInitialized)
		return true;

	s_monoInitilizationData = new MonoInitilizationData();

	mono_set_dirs(monoAssemblyDir.c_str(), monoConfigDir.c_str());
	s_monoInitilizationData->_domain = mono_jit_init_version(domainName.c_str(), "v4.0.30319");

	if (s_monoInitilizationData->_domain != nullptr)
	{
		s_monoInitialized = true;
		return true;
	}

	error = "Unable to initialize the Mono domain";
	s_monoInitialized = false;
	return false;
}

void ManagedModule::ShutDownMono()
{
	if (s_monoInitialized)
	{
		if (s_monoInitilizationData)
		{
			mono_jit_cleanup(s_monoInitilizationData->_domain);
			s_monoInitilizationData->_domain = nullptr;
		}
	}

	if (s_monoInitilizationData)
		delete s_monoInitilizationData;

	s_monoInitialized = false;
}

bool ManagedModule::Load(std::string& error, bool reloadPluginsFromStoredData)
{
	if (s_monoInitialized == false)
	{
		error = "Cannot load the managed module: Mono isn't initialized";
		return false;
	}

	// First of all check for the dependencies
	if (CheckDependencies(error) == false)
		return false;
	
	// Return immediately if the module has been already loaded
	if (_internalData->_moduleAssembly != nullptr && _internalData->_moduleImage != nullptr)
		return true;

	// If there is not valid info about the binary we can't go on
	if (_binaryInfo == nullptr)
		return false;

	// Check if the binary data is valid
	if (_binaryInfo->_entryPointNamespace == nullptr || strcmp(_binaryInfo->_entryPointNamespace, "") == 0)
	{
		error = "Cannot load the managed module: invalid entry point namespace specified";
		return false;
	}

	if (_binaryInfo->_entryPointClass == nullptr || strcmp(_binaryInfo->_entryPointClass, "") == 0)
	{
		error = "Cannot load the managed module: invalid entry point class specified";
		return false;
	}

	// Get the path to the binary
	Path binPath(_binaryInfo->_path);
	if (binPath.IsAbsolute() == false)
	{
		binPath = Path(_basePath) + binPath;
		binPath.MakeAbsolute();
	}

	// Compile the binary (if auto compile is turned off this returns true)
	if (Compile(error) == false)
		return false;

	// Check if the binary if found
	if (binPath.IsEmpty() || !binPath.Exists())
	{
		error = "Cannot find the module's binary";
		return false;
	}
	
	MonoImageOpenStatus status;

	size_t fileSize;
	char* fileContent = ReadFile(binPath.GetPath().c_str(), fileSize);
	
	if (fileContent == nullptr)
	{
		error = "Cannot load the managed module: unable to read the module's assembly file";
		return false;
	}

	_internalData->_moduleImage = mono_image_open_from_data_with_name(fileContent, fileSize, true, &status, false, binPath.GetPath().c_str());
	if(status != MONO_IMAGE_OK || _internalData->_moduleImage == nullptr)
	{
		_internalData->_moduleImage = nullptr;

		error = "Cannot load the managed module: unable to load the module's image";
		return false;
	}

	_internalData->_moduleAssembly = mono_assembly_load_from_full(_internalData->_moduleImage, binPath.GetPath().c_str(), &status, false);
	if (status != MONO_IMAGE_OK || _internalData->_moduleAssembly == nullptr)
	{
		mono_image_close(_internalData->_moduleImage);
		_internalData->_moduleImage = nullptr;
		_internalData->_moduleAssembly = nullptr;

		error = "Cannot load the managed module: unable to load the module's assembly file";
		return false;
	}

	MonoClass* klass = mono_class_from_name(_internalData->_moduleImage, _binaryInfo->_entryPointNamespace, _binaryInfo->_entryPointClass);
	if (klass = nullptr)
	{
		mono_image_close(_internalData->_moduleImage);
		_internalData->_moduleImage = nullptr;
		_internalData->_moduleAssembly = nullptr;

		error = "Cannot load the managed module: unable to get the entry point class";
		return false;
	}

	// Get the module's methods
	_internalData->_loadModuleFunc = FindMethod(klass, "LoadModule", 0);
	_internalData->_reloadModuleFunc = FindMethod(klass, "ReloadModule", 0);
	_internalData->_unloadModuleFunc = FindMethod(klass, "UnloadModule", 0);

	if (_internalData->_loadModuleFunc == nullptr)
	{
		mono_image_close(_internalData->_moduleImage);
		_internalData->_moduleImage = nullptr;
		_internalData->_moduleAssembly = nullptr;

		error = "Cannot load the managed module: the entry point class doesn't implement the LoadModule method";
		return false;
	}

	if (_internalData->_unloadModuleFunc == nullptr)
	{
		mono_image_close(_internalData->_moduleImage);
		_internalData->_moduleImage = nullptr;
		_internalData->_moduleAssembly = nullptr;

		error = "Cannot load the managed module: the entry point class doesn't implement the UnloadModule method";
		return false;
	}
	
	PluginInfo* registeredPlugins = nullptr;
	int registeredPluginsCount = 0;

	// Get the module's registered plugins
	MonoMethod* registerPluginsFunc = FindMethod(klass, "RegisterPlugins", 0);
	if (registerPluginsFunc)
	{
		void* args = nullptr;
		MonoObject* registeredPluginsObj = mono_runtime_invoke(registerPluginsFunc, nullptr, &args, nullptr);

		if (registeredPluginsObj != nullptr)
		{
			MonoArray* registeredPlugins = (MonoArray*)mono_object_unbox(registeredPluginsObj);

			if (registeredPlugins != nullptr)
			{
				unsigned int len = mono_array_length(registeredPlugins);

				for (unsigned int i = 0; i < len; i++)
				{
					MonoObject* obj = mono_array_get(registeredPlugins, MonoObject*, i);

					if (obj)
					{
						CPluginInfo* infos = (CPluginInfo*)mono_object_unbox(obj);

						if (infos && infos->_pluginInfo != nullptr)
						{
							auto& rpIt = _registeredPluginsMap->find(infos->Name());
							if (rpIt == _registeredPluginsMap->end())
							{
								PluginInfo* newInfo = new PluginInfo();
								CopyPluginInfo(*newInfo, *infos->_pluginInfo);

								newInfo->_reserved_module = this;

								(*_registeredPluginsMap)[infos->Name()] = newInfo;
								_registeredPluginsVec->push_back(newInfo);

								auto& gsIt = _givenServices->find(infos->Service());
								if (gsIt != _givenServices->end())
									gsIt->second.push_back(newInfo);
								else
								{
									std::vector<PluginInfo*> tmpVec;
									tmpVec.push_back(newInfo);

									(*_givenServices)[infos->Service()] = tmpVec;
								}
							}
						}
					}
				}
			}
		}
	}

	if (_registeredPluginsVec->size() > 0)
	{
		_infos._reserved_registeredPlugins = _registeredPluginsVec->at(0);
		_infos._reserved_registeredPluginsCount = (int)_registeredPluginsVec->size();
	}

	if (_manager != nullptr)
		_manager->OnModuleLoaded(this);

	// Now that the module has been loaded correctly we can reload the plugins if needed
	if (reloadPluginsFromStoredData)
	{
		for (auto& data : *_pluginsDataToRestoreAfterReload)
		{
			CreatedPlugin* pluginToReload = data->_referencePlugin;

			auto& rpIt = _registeredPluginsMap->find(data->_pluginName);
			if (rpIt != _registeredPluginsMap->end())
			{
				pluginToReload->_reserved_plugin_info = rpIt->second;
				pluginToReload->_reserved_plugin = new ManagedPlugin(&this->_infos, rpIt->second, nullptr);

				((ManagedPlugin*)(pluginToReload->_reserved_plugin))->LoadDataAfterReload(data->_storedData.c_str());
			}

			// Destroy the stored data
			delete data;
		}

		_pluginsDataToRestoreAfterReload->clear();
	}

	return true;
}

void ManagedModule::Unload(bool unloadDependantModules, bool storePluginsForReload)
{
	//if (_moduleAssembly != nullptr && _moduleImage != nullptr)
	//{
	//	// Unload all the modules that depend on this one
	//	// Only when reloading the module the dependant plugins don't get unloaded
	//	if (unloadDependantModules)
	//	{
	//		for (size_t i = 0; i < _dependantModules.size(); i++)
	//			_dependantModules[i]->Unload();
	//	}

	//	for (size_t i = 0; i < _createdPlugins->size(); i++)
	//	{
	//		CreatedPlugin* pl = _createdPlugins->at(i);

	//		if (storePluginsForReload == false)
	//		{
	//			delete ((ManagedPlugin*)(pl->_reserved_plugin));
	//			delete pl;
	//		}
	//		else
	//		{
	//			// Memorize the plugin data to be loaded again later
	//			PluginDataToRestoreAfterReload* pluginData = new PluginDataToRestoreAfterReload();
	//			pluginData->_pluginName = pl->_reserved_plugin_info->_name;
	//			pluginData->_referencePlugin = pl;

	//			if (pl->_reserved_plugin_info->_saveDataForPluginReloadFunc)
	//			{
	//				const char* savedData = ((ManagedPlugin*)(pl->_reserved_plugin))->SaveDataForReload();

	//				if (savedData != nullptr)
	//					pluginData->_storedData = savedData;
	//				else
	//					pluginData->_storedData = "";
	//			}

	//			_pluginsDataToRestoreAfterReload->push_back(pluginData);

	//			// Delete the plugin
	//			delete ((ManagedPlugin*)(pl->_reserved_plugin));
	//			pl->_reserved_plugin = nullptr;
	//			pl->_reserved_plugin_info = nullptr;
	//		}
	//	}

	//	if (storePluginsForReload == false)
	//		_createdPlugins->clear();

	//	for (auto& el : (*_registeredPluginsVec))
	//	{
	//		DestroyPluginInfo(*el);
	//		delete el;
	//	}

	//	_registeredPluginsVec->clear();
	//	_registeredPluginsMap->clear();
	//	_givenServices->clear();

	//	_unloadModuleFunc();

	//	delete _moduleBinary;
	//	_moduleBinary = nullptr;
	//}

	NotifyUnloadToDependencies();
}

bool ManagedModule::Reload(std::function<void()> moduleUnloaded, std::string& error)
{
	//if (_moduleAssembly == nullptr || _moduleImage == nullptr)
	//{
	//	error = "The module wasn't loaded";
	//	return false;
	//}
	//else
	//{
	//	// Call the reload to give the module the chanche to save its data if it needs
	//	if (_reloadModuleFunc == nullptr)
	//	{
	//		error = "The module hasn't got a reload method";
	//		return false;
	//	}

	//	int result = _reloadModuleFunc();
	//	if (result != 0)
	//	{
	//		std::stringstream ss;
	//		ss << "The module's reload method returned an error code: " << result;
	//		error = ss.str();
	//		return false;
	//	}

	//	// Unload the module
	//	Unload(false, true);

	//	// Let the called know that the mobule has been unloaded
	//	if (moduleUnloaded)
	//		moduleUnloaded();

	//	// Load the module again
	//	return Load(error, true);
	//}

	return true;
}

std::vector<std::string> ManagedModule::GetGivenServices() const
{
	std::vector<std::string> result;
	result.reserve(_givenServices->size());

	for (auto& el : (*_givenServices))
		result.push_back(el.first);

	return result;
}

std::vector<std::string> ManagedModule::GetRegisteredPluginsForService(const std::string& serviceName) const
{
	std::vector<std::string> result;

	auto& gsIt = _givenServices->find(serviceName);

	if (gsIt != _givenServices->end())
	{
		result.reserve(gsIt->second.size());

		for (auto& el : gsIt->second)
			result.push_back(el->_name);
	}

	return result;
}

std::vector<std::string> ManagedModule::GetRegisteredPlugins() const
{
	std::vector<std::string> result;
	result.reserve(_registeredPluginsVec->size());

	for (auto& el : (*_registeredPluginsVec))
		result.push_back(el->_name);

	return result;
}

Plugin* ManagedModule::CreatePlugin(const std::string& pluginName, void* creationData)
{
	auto& rpIt = _registeredPluginsMap->find(pluginName);
	if (rpIt == _registeredPluginsMap->end())
		return nullptr;

	CreatedPlugin* newPlugin = new CreatedPlugin();
	newPlugin->_reserved_module_info = &this->_infos;
	newPlugin->_reserved_plugin_info = rpIt->second;
	newPlugin->_reserved_plugin = new ManagedPlugin(&this->_infos, rpIt->second, creationData);

	_createdPlugins->push_back(newPlugin);
	return (Plugin*)(newPlugin->_reserved_plugin);
}

bool ManagedModule::DestroyPlugin(Plugin* plugin)
{
	for (size_t i = 0; i < _createdPlugins->size(); i++)
	{
		CreatedPlugin* pl = _createdPlugins->at(i);

		if (pl->_reserved_plugin == plugin)
		{
			delete ((ManagedPlugin*)(pl->_reserved_plugin));
			delete pl;

			_createdPlugins->erase(_createdPlugins->begin() + i);
			return true;
		}
	}

	return false;
}

bool ManagedModule::Compile(std::string& error)
{
	if (_binaryInfo->_autoCompile == false)
		return true;

	error = "Managed modules cannot be auto-compiled yet";
	return false;
}

CreatedPlugin* ManagedModule::CreatePlugin(void* module, const char* pluginName, void* creationData)
{
	if (module == nullptr || pluginName == nullptr)
		return nullptr;

	ManagedModule* managedModule = (ManagedModule*)module;

	auto& rpIt = managedModule->_registeredPluginsMap->find(pluginName);
	if (rpIt == managedModule->_registeredPluginsMap->end())
		return nullptr;

	CreatedPlugin* newPlugin = new CreatedPlugin();
	newPlugin->_reserved_module_info = &managedModule->_infos;
	newPlugin->_reserved_plugin_info = rpIt->second;
	newPlugin->_reserved_plugin = new ManagedPlugin(&managedModule->_infos, rpIt->second, creationData);

	managedModule->_createdPlugins->push_back(newPlugin);

	return newPlugin;
}

bool ManagedModule::DestroyPlugin(void* moduleInfo, CreatedPlugin* plugin)
{
	if (moduleInfo == nullptr || plugin == nullptr)
		return false;

	ModuleInfo* localModuleInfos = (ModuleInfo*)moduleInfo;
	ManagedModule* managedModule = (ManagedModule*)localModuleInfos->_reserved_module;

	for (size_t i = 0; i < managedModule->_createdPlugins->size(); i++)
	{
		CreatedPlugin* pl = managedModule->_createdPlugins->at(i);

		if (pl == plugin)
		{
			delete ((ManagedPlugin*)(pl->_reserved_plugin));
			delete pl;

			managedModule->_createdPlugins->erase(managedModule->_createdPlugins->begin() + i);
			return true;
		}
	}

	return false;
}

bool ManagedModule::SendMessageToPlugin(CreatedPlugin* plugin, const char* messageName, void* messageData)
{
	if (plugin == nullptr || plugin->_reserved_plugin == nullptr || messageName == nullptr)
		return false;

	ManagedPlugin* managedPlugin = (ManagedPlugin*)plugin->_reserved_plugin;
	return managedPlugin->SendMessage(messageName, messageData);
}

#endif