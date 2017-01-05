#include "UnmanagedModule.h"
#include "UnmanagedPlugin.h"
#include "ModulesManager.h"
#include "DynamicLibrary.h"
#include "Path.h"

#include <sstream>

UnmanagedModule::UnmanagedModule(const ModuleInfo& moduleInfo, const std::string& basePath)
	: Module(moduleInfo, basePath)
	, _loadModuleFunc(nullptr)
	, _unloadModuleFunc(nullptr)
	, _reloadModuleFunc(nullptr)
	, _moduleBinary(nullptr)
	, _givenServices(new GivenServicesMap())
	, _registeredPluginsMap(new RegisteredPluginsMap())
	, _registeredPluginsVec(new RegisteredPluginsVector())
	, _createdPlugins(new CreatedPluginsVector())
	, _pluginsDataToRestoreAfterReload(new PluginDataToRestoreAfterReloadVector())
{
	_infos._reserved_createPluginFunc = CreatePlugin;
	_infos._reserved_destroyPluginFunc = DestroyPlugin;
	_infos._reserved_sendMessageToPluginFunc = SendMessageToPlugin;
	_infos._reserved_module = this;
}

UnmanagedModule::~UnmanagedModule()
{
	std::vector<Module*> localDependantModules = _dependantModules;

	Unload();

	for (auto& el : localDependantModules)
		el->OnDependencyDestructed(this);
	
	for (auto& el : *_pluginsDataToRestoreAfterReload)
		delete el;

	delete _pluginsDataToRestoreAfterReload;
	delete _givenServices;
	delete _registeredPluginsMap;
	delete _registeredPluginsVec;
	delete _createdPlugins;
}

bool UnmanagedModule::Load(std::string& error, bool reloadPluginsFromStoredData)
{
	// First of all check for the dependencies
	if (CheckDependencies(error) == false)
		return false;

	// Return immediately if the module has been already loaded
	if (_moduleBinary)
		return true;

	// If there is not valid info about the binary we can't go on
	if (_binaryInfo == nullptr)
		return false;

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

	// Load the library containing the module
	_moduleBinary = DynamicLibrary::Load(binPath.GetPath(), error);
	if (_moduleBinary == nullptr)
		return false;

	// Get the module's load function pointer
	_loadModuleFunc = (__loadModuleFunc)_moduleBinary->GetSymbol("LoadModule");

	if (_loadModuleFunc == nullptr)
	{
		delete _moduleBinary;
		_moduleBinary = nullptr;

		error = "The module doesn't contain a LoadModule function";
		return false;
	}

	// Call the module's load function
	LoadModuleResult moduleLoadingResult = _loadModuleFunc();

	if (moduleLoadingResult._unloadModuleFunc == nullptr)
	{
		delete _moduleBinary;
		_moduleBinary = nullptr;

		error = "The module's LoadModule function returned invalid data: UnloadModule function not specified";
		return false;
	}

	_unloadModuleFunc = moduleLoadingResult._unloadModuleFunc;
	_reloadModuleFunc = moduleLoadingResult._reloadModuleFunc;

	PluginInfo* registeredPlugins = nullptr;
	int registeredPluginsCount = 0;

	// Get the module's registered plugins
	__getRegisteredPluginsFunc getRegisteredPluginsFunc = (__getRegisteredPluginsFunc)_moduleBinary->GetSymbol("__get_registered_plugins");
	if (getRegisteredPluginsFunc)
	{
		getRegisteredPluginsFunc(&registeredPlugins, registeredPluginsCount);

		for (int i = 0; i < registeredPluginsCount; i++)
		{
			PluginInfo& pi = registeredPlugins[i];
			
			auto& rpIt = _registeredPluginsMap->find(pi._name);
			if (rpIt == _registeredPluginsMap->end())
			{
				PluginInfo* newInfo = new PluginInfo();
				CopyPluginInfo(*newInfo, pi);

				newInfo->_reserved_module = this;

				(*_registeredPluginsMap)[pi._name] = newInfo;
				_registeredPluginsVec->push_back(newInfo);

				auto& gsIt = _givenServices->find(pi._service);
				if (gsIt != _givenServices->end())
					gsIt->second.push_back(newInfo);
				else
				{
					std::vector<PluginInfo*> tmpVec;
					tmpVec.push_back(newInfo);

					(*_givenServices)[pi._service] = tmpVec;
				}
			}
		}

		ClearPluginInfos(&registeredPlugins, registeredPluginsCount);
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
				pluginToReload->_reserved_plugin = new UnmanagedPlugin(&this->_infos, rpIt->second, nullptr);

				((UnmanagedPlugin*)(pluginToReload->_reserved_plugin))->LoadDataAfterReload(data->_storedData.c_str());
			}

			// Destroy the stored data
			delete data;
		}

		_pluginsDataToRestoreAfterReload->clear();
	}

	return true;
}

void UnmanagedModule::Unload(bool unloadDependantModules, bool storePluginsForReload)
{
	if (_moduleBinary)
	{
		// Unload all the modules that depend on this one
		// Only when reloading the module the dependant plugins don't get unloaded
		if (unloadDependantModules)
		{
			for (size_t i = 0; i < _dependantModules.size(); i++)
				_dependantModules[i]->Unload();
		}

		for (size_t i = 0; i < _createdPlugins->size(); i++)
		{
			CreatedPlugin* pl = _createdPlugins->at(i);

			if (storePluginsForReload == false)
			{
				delete ((UnmanagedPlugin*)(pl->_reserved_plugin));
				delete pl;
			}
			else
			{
				// Memorize the plugin data to be loaded again later
				PluginDataToRestoreAfterReload* pluginData = new PluginDataToRestoreAfterReload();
				pluginData->_pluginName = pl->_reserved_plugin_info->_name;
				pluginData->_referencePlugin = pl;

				if (pl->_reserved_plugin_info->_saveDataForPluginReloadFunc)
				{
					const char* savedData = ((UnmanagedPlugin*)(pl->_reserved_plugin))->SaveDataForReload();

					if (savedData != nullptr)
						pluginData->_storedData = savedData;
					else
						pluginData->_storedData = "";
				}

				_pluginsDataToRestoreAfterReload->push_back(pluginData);

				// Delete the plugin
				delete ((UnmanagedPlugin*)(pl->_reserved_plugin));
				pl->_reserved_plugin = nullptr;
				pl->_reserved_plugin_info = nullptr;
			}
		}

		if(storePluginsForReload == false)
			_createdPlugins->clear();

		for (auto& el : (*_registeredPluginsVec))
		{
			DestroyPluginInfo(*el);
			delete el;
		}

		_registeredPluginsVec->clear();
		_registeredPluginsMap->clear();
		_givenServices->clear();

		_unloadModuleFunc();

		delete _moduleBinary;
		_moduleBinary = nullptr;
	}

	NotifyUnloadToDependencies();
}

bool UnmanagedModule::Reload(std::function<void()> moduleUnloaded, std::string& error)
{
	if (_moduleBinary == nullptr)
	{
		error = "The module wasn't loaded";
		return false;
	}
	else
	{
		// Call the reload to give the module the chanche to save its data if it needs
		if (_reloadModuleFunc == nullptr)
		{
			error = "The module hasn't got a reload method";
			return false;
		}
		
		int result = _reloadModuleFunc();
		if (result != 0)
		{
			std::stringstream ss;
			ss << "The module's reload method returned an error code: " << result;
			error = ss.str();
			return false;
		}

		// Unload the module
		Unload(false, true);

		// Let the called know that the mobule has been unloaded
		if (moduleUnloaded)
			moduleUnloaded();

		// Load the module again
		return Load(error, true);
	}
}

std::vector<std::string> UnmanagedModule::GetGivenServices() const
{
	std::vector<std::string> result;
	result.reserve(_givenServices->size());

	for (auto& el : (*_givenServices))
		result.push_back(el.first);

	return result;
}

std::vector<std::string> UnmanagedModule::GetRegisteredPluginsForService(const std::string& serviceName) const
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

std::vector<std::string> UnmanagedModule::GetRegisteredPlugins() const
{
	std::vector<std::string> result;
	result.reserve(_registeredPluginsVec->size());

	for (auto& el : (*_registeredPluginsVec))
		result.push_back(el->_name);

	return result;
}

Plugin* UnmanagedModule::CreatePlugin(const std::string& pluginName, void* creationData)
{
	auto& rpIt = _registeredPluginsMap->find(pluginName);
	if (rpIt == _registeredPluginsMap->end())
		return nullptr;

	CreatedPlugin* newPlugin = new CreatedPlugin();
	newPlugin->_reserved_module_info = &this->_infos;
	newPlugin->_reserved_plugin_info = rpIt->second;
	newPlugin->_reserved_plugin = new UnmanagedPlugin(&this->_infos, rpIt->second, creationData);

	_createdPlugins->push_back(newPlugin);
	return (Plugin*)(newPlugin->_reserved_plugin);
}

bool UnmanagedModule::DestroyPlugin(Plugin* plugin)
{
	for (size_t i = 0; i < _createdPlugins->size(); i++)
	{
		CreatedPlugin* pl = _createdPlugins->at(i);

		if (pl->_reserved_plugin == plugin)
		{
			delete ((UnmanagedPlugin*)(pl->_reserved_plugin));
			delete pl;

			_createdPlugins->erase(_createdPlugins->begin() + i);
			return true;
		}
	}

	return false;
}

bool UnmanagedModule::Compile(std::string& error)
{
	if (_binaryInfo->_autoCompile == false)
		return true;

	error = "Unmanaged modules cannot be auto-compiled yet";
	return false;
}

CreatedPlugin* UnmanagedModule::CreatePlugin(void* module, const char* pluginName, void* creationData)
{
	if (module == nullptr || pluginName == nullptr)
		return nullptr;

	UnmanagedModule* unmanagedModule = (UnmanagedModule*)module;

	auto& rpIt = unmanagedModule->_registeredPluginsMap->find(pluginName);
	if (rpIt == unmanagedModule->_registeredPluginsMap->end())
		return nullptr;
		
	CreatedPlugin* newPlugin = new CreatedPlugin();
	newPlugin->_reserved_module_info = &unmanagedModule->_infos;
	newPlugin->_reserved_plugin_info = rpIt->second;
	newPlugin->_reserved_plugin = new UnmanagedPlugin(&unmanagedModule->_infos, rpIt->second, creationData);

	unmanagedModule->_createdPlugins->push_back(newPlugin);

	return newPlugin;
}

bool UnmanagedModule::DestroyPlugin(void* moduleInfo, CreatedPlugin* plugin)
{
	if (moduleInfo == nullptr || plugin == nullptr)
		return false;

	ModuleInfo* localModuleInfos = (ModuleInfo*)moduleInfo;
	UnmanagedModule* unmanagedModule = (UnmanagedModule*)localModuleInfos->_reserved_module;

	for(size_t i = 0; i < unmanagedModule->_createdPlugins->size(); i++)
	{
		CreatedPlugin* pl = unmanagedModule->_createdPlugins->at(i);

		if (pl == plugin)
		{
			delete ((UnmanagedPlugin*)(pl->_reserved_plugin));
			delete pl;

			unmanagedModule->_createdPlugins->erase(unmanagedModule->_createdPlugins->begin() + i);
			return true;
		}
	}

	return false;
}

bool UnmanagedModule::SendMessageToPlugin(CreatedPlugin* plugin, const char* messageName, void* messageData)
{
	if (plugin == nullptr || plugin->_reserved_plugin == nullptr || messageName == nullptr)
		return false;

	UnmanagedPlugin* unmanagedPlugin = (UnmanagedPlugin*)plugin->_reserved_plugin;
	return unmanagedPlugin->SendMessage(messageName, messageData);
}