#include "ModulesManager.h"
#include "Module.h"
#include "json.hpp"
#include "Path.h"

#include "UnmanagedModule.h"
#include "ManagedModule.h"

#include <fstream>
#include <regex>

using namespace nlohmann;

static void SplitString(const std::string& src, std::vector<std::string>& result, const char* separators)
{
	char* localString = new char[src.length() + 1];
	memset(localString, 0, src.length() + 1);

	strcpy(localString, src.c_str());

	char *p = strtok(localString, separators);
	while (p)
	{
		result.push_back(p);
		p = strtok(nullptr, separators);
	}

	delete[] localString;
}

ModulesManager::ModulesManager()
	: _loadedModules(new ModulesMap())
{ }

ModulesManager::~ModulesManager()
{
	_loadedModules->clear();
	delete _loadedModules;

	ManagedModule::ShutDownMono();
}

ModulesManager& ModulesManager::GetInstance()
{
	static ModulesManager s_instance;
	return s_instance;
}

bool ModulesManager::Initialize(const std::string& monoDomainName, const std::string& monoAssemblyDir, const std::string& monoConfigDir, std::string* error)
{
	std::string localError = "";
	std::string* actualError = error != nullptr ? error : &localError;

	return ManagedModule::InitializeMono(monoDomainName, monoAssemblyDir, monoConfigDir, *actualError);
}

std::shared_ptr<Module> ModulesManager::GetModule(const std::string& name)
{
	auto& pair = _loadedModules->find(name);

	if (pair != _loadedModules->end())
		return pair->second;
	return nullptr;
}

std::shared_ptr<Module> ModulesManager::LoadModule(const std::string& path, std::string* error)
{
	std::string localError;
	std::string* actualError = error == nullptr ? &localError : error;

	Path mainPath = path;
	mainPath.MakeAbsolute();

	if (!mainPath.IsDirectory())
		mainPath = mainPath.GetParent();

	Path moduleJSONPath = mainPath + Path("package.json");

	// Check if package.json exists
	if (!moduleJSONPath.Exists())
	{
		*actualError = "The file package.json wasn't found inside the given path";
		return nullptr;
	}

	// Read the module informations from package.json
	ModuleInfo moduleInfos;
	ReadModuleJSON(moduleJSONPath, moduleInfos);

	// The module's name is required
	if (moduleInfos._name == nullptr || strcmp(moduleInfos._name, "") == 0)
	{
		*actualError = "The module's name wasn't specified";
		return nullptr;
	}

	std::string moduleName = moduleInfos._name;

	// Check if a module with the requested name is already present
	auto& module = _loadedModules->find(moduleName);
	if (module != _loadedModules->end())
		return module->second;

	Module* loadedModule = nullptr;

	if (moduleInfos._moduleType == MT_NotValid)
	{
		*actualError = "Module type not specified. Set to 'managed' or 'unmanaged'";
		return nullptr;
	}
	else if (moduleInfos._moduleType == MT_Unmanaged)
		loadedModule = new UnmanagedModule(moduleInfos, mainPath.GetPath());
	else if (moduleInfos._moduleType == MT_Managed)
		loadedModule = new ManagedModule(moduleInfos, mainPath.GetPath());

	DestroyModuleInfo(moduleInfos);
	
	// Let the module know its creator
	loadedModule->_manager = this;

	// Add the entry to the loaded modules map
	(*_loadedModules)[moduleName] = std::shared_ptr<Module>();

	if(loadedModule == nullptr || loadedModule->Load(*actualError) == false)
	{
		// If the module couldn't be loaded remove its entry
		_loadedModules->erase(moduleName);

		delete loadedModule;
		return nullptr;
	}
	
	return (*_loadedModules)[moduleName];
}

bool ModulesManager::ReloadModule(std::shared_ptr<Module> module, std::function<void()> moduleUnloaded, std::string* error)
{
	std::string localError;
	std::string* actualError = error == nullptr ? &localError : error;

	if (!module)
	{
		*actualError = "Invalid module";
		return false;
	}
		
	return module->Reload(moduleUnloaded, *actualError);
}

void ModulesManager::ReadModuleJSON(const Path& path, ModuleInfo& moduleInfos)
{
	InitializeModuleInfo(moduleInfos);

	std::ifstream i(path.GetPath());

	json moduleJSON;
	i >> moduleJSON;

	// Find the module's name (REQUIRED)
	json::iterator nameIt = moduleJSON.find("name");
	if (nameIt != moduleJSON.end() && nameIt.value().is_string())
		SetModuleInfoName(moduleInfos, nameIt.value().get<std::string>().c_str());

	// Find the module's type (REQUIRED)
	json::iterator typeIt = moduleJSON.find("type");
	if (typeIt != moduleJSON.end() && typeIt.value().is_string())
	{
		std::string type = typeIt.value().get<std::string>();

		if (std::regex_match(type, std::regex("[Mm][Aa][Nn][Aa][Gg][Ee][Dd]")))
			SetModuleInfoModuleType(moduleInfos, MT_Managed);
		else if (std::regex_match(type, std::regex("[Uu][Nn][Mm][Aa][Nn][Aa][Gg][Ee][Dd]")))
			SetModuleInfoModuleType(moduleInfos, MT_Unmanaged);
	}

	// Find the module's binaries
	json::iterator binariesIt = moduleJSON.find("binaries");
	if (binariesIt != moduleJSON.end() && binariesIt.value().is_array())
	{
		std::vector<json> binariesJSON = binariesIt.value();

		if (binariesJSON.size() > 0)
		{
			int binariesInfoCount = binariesJSON.size();
			ModuleBinaryInfo* binariesInfo = new ModuleBinaryInfo[binariesInfoCount];

			for (int i = 0; i < binariesInfoCount; i++)
			{
				const json& binaryInfoJSON = binariesJSON[i];

				InitializeModuleBinaryInfo(binariesInfo[i]);

				json::const_iterator binaryPathIt = binaryInfoJSON.find("path");
				json::const_iterator binaryOperatingSystemIt = binaryInfoJSON.find("os");
				json::const_iterator binaryArchIt = binaryInfoJSON.find("arch");
				json::const_iterator binaryAutoCompileIt = binaryInfoJSON.find("auto_compile");
				json::const_iterator binaryEntryPointNamespaceIt = binaryInfoJSON.find("entry_point_namespace");
				json::const_iterator binaryEntryPointClassIt = binaryInfoJSON.find("entry_point_class");
				json::const_iterator binaryCompileCommandIt = binaryInfoJSON.find("compile_command");
				json::const_iterator binarySourceFilesIt = binaryInfoJSON.find("source_files");
				json::const_iterator binaryCompilerIt = binaryInfoJSON.find("compiler");
				json::const_iterator binaryCompilerArgsIt = binaryInfoJSON.find("compiler_args");
				json::const_iterator binaryLinkerIt = binaryInfoJSON.find("linker");
				json::const_iterator binaryLinkerArgsIt = binaryInfoJSON.find("linker_args");

				if (binaryPathIt != binaryInfoJSON.end() && binaryPathIt.value().is_string())
					SetModuleBinaryInfoPath(binariesInfo[i], binaryPathIt.value().get<std::string>().c_str());

				if (binaryOperatingSystemIt != binaryInfoJSON.end() && binaryOperatingSystemIt.value().is_string())
				{
					std::string os = binaryOperatingSystemIt.value().get<std::string>();

					if(os == "*")
						SetModuleBinaryInfoOperatingSystem(binariesInfo[i], OS_All);
					else if (std::regex_match(os, std::regex("([Ww][Ii][Nn])|([Ww][Ii][Nn][Dd][Oo][Ww][Ss])")))
						SetModuleBinaryInfoOperatingSystem(binariesInfo[i], OS_Windows);
					else if (std::regex_match(os, std::regex("([Mm][Aa][Cc])|([Mm][Aa][Cc][Ii][Nn][Tt][Oo][Ss][Hh])")))
						SetModuleBinaryInfoOperatingSystem(binariesInfo[i], OS_Mac);
					else if (std::regex_match(os, std::regex("([Ll][Ii][Nn][Uu][Xx])")))
						SetModuleBinaryInfoOperatingSystem(binariesInfo[i], OS_Linux);
				}
				
				if (binaryArchIt != binaryInfoJSON.end() && binaryArchIt.value().is_string())
				{
					std::string arch = binaryArchIt.value().get<std::string>();

					if (arch == "*")
						SetModuleBinaryInfoArchType(binariesInfo[i], AT_All);
					else if (std::regex_match(arch, std::regex("([Xx]86)|(32[Bb][Ii][Tt])")))
						SetModuleBinaryInfoArchType(binariesInfo[i], AT_x86);
					else if (std::regex_match(arch, std::regex("([Xx]64)|(64[Bb][Ii][Tt])")))
						SetModuleBinaryInfoArchType(binariesInfo[i], AT_x64);
				}

				if(binaryAutoCompileIt != binaryInfoJSON.end() && binaryAutoCompileIt.value().is_boolean())
					SetModuleBinaryInfoAutoCompile(binariesInfo[i], binaryAutoCompileIt.value().get<bool>());

				if (binaryEntryPointNamespaceIt != binaryInfoJSON.end() && binaryEntryPointNamespaceIt.value().is_string())
					SetModuleBinaryInfoEntryPointNamespace(binariesInfo[i], binaryEntryPointNamespaceIt.value().get<std::string>().c_str());

				if (binaryEntryPointClassIt != binaryInfoJSON.end() && binaryEntryPointClassIt.value().is_string())
					SetModuleBinaryInfoEntryPointClass(binariesInfo[i], binaryEntryPointClassIt.value().get<std::string>().c_str());

				if (binaryCompileCommandIt != binaryInfoJSON.end() && binaryCompileCommandIt.value().is_string())
					SetModuleBinaryInfoCompileCommand(binariesInfo[i], binaryCompileCommandIt.value().get<std::string>().c_str());

				if (binaryCompilerIt != binaryInfoJSON.end() && binaryCompilerIt.value().is_string())
					SetModuleBinaryInfoCompiler(binariesInfo[i], binaryCompilerIt.value().get<std::string>().c_str());

				if (binaryCompilerArgsIt != binaryInfoJSON.end() && binaryCompilerArgsIt.value().is_string())
					SetModuleBinaryInfoCompilerArgs(binariesInfo[i], binaryCompilerArgsIt.value().get<std::string>().c_str());

				if (binaryLinkerIt != binaryInfoJSON.end() && binaryLinkerIt.value().is_string())
					SetModuleBinaryInfoLinker(binariesInfo[i], binaryLinkerIt.value().get<std::string>().c_str());

				if (binaryLinkerArgsIt != binaryInfoJSON.end() && binaryLinkerArgsIt.value().is_string())
					SetModuleBinaryInfoLinkerArgs(binariesInfo[i], binaryLinkerArgsIt.value().get<std::string>().c_str());

				if (binarySourceFilesIt != binaryInfoJSON.end())
				{
					if(binarySourceFilesIt.value().is_string())
						SetModuleBinaryInfoSourceFiles(binariesInfo[i], binarySourceFilesIt.value().get<std::string>().c_str());
					else if (binarySourceFilesIt.value().is_array())
					{
						std::vector<std::string> sourceFiles = binarySourceFilesIt.value();

						std::string strSourceFiles = "";
						for (auto& el : sourceFiles)
							strSourceFiles += el + ";";

						SetModuleBinaryInfoSourceFiles(binariesInfo[i], strSourceFiles.c_str());
					}
				}
			}

			SetModuleInfoBinaries(moduleInfos, binariesInfo, binariesInfoCount);

			for (int i = 0; i < binariesInfoCount; i++)
				DestroyModuleBinaryInfo(binariesInfo[i]);
			delete[] binariesInfo;
		}
	}
	
	// Find the module's dependencies
	json::iterator dependenciesIt = moduleJSON.find("dependencies");
	if (dependenciesIt != moduleJSON.end())
	{
		if (dependenciesIt.value().is_array())
		{
			std::vector<json> dependenciesJSON = dependenciesIt.value();

			if (dependenciesJSON.size() > 0)
			{
				int dependenciesInfoCount = dependenciesJSON.size();
				ModuleDependencyInfo* dependenciesInfo = new ModuleDependencyInfo[dependenciesInfoCount];

				for (int i = 0; i < dependenciesInfoCount; i++)
				{
					const json& dependencyInfoJSON = dependenciesJSON[i];

					InitializeModuleDependencyInfo(dependenciesInfo[i]);

					json::const_iterator dependencyNameIt = dependencyInfoJSON.find("name");
					json::const_iterator dependencyVersionIt = dependencyInfoJSON.find("version");

					if (dependencyNameIt != dependencyInfoJSON.end() && dependencyNameIt.value().is_string())
						SetModuleDependencyInfoName(dependenciesInfo[i], dependencyNameIt.value().get<std::string>().c_str());

					if (dependencyVersionIt != dependencyInfoJSON.end() && dependencyVersionIt.value().is_string())
					{
						int major = 0;
						int minor = 0;
						int patch = 0;
						int flags = MDVT_CheckMajor | MDVT_CheckMinor;

						std::string version = dependencyVersionIt.value().get<std::string>();
						std::vector<std::string> versionParts;

						SplitString(version, versionParts, ".");

						if (versionParts.size() > 0)
						{
							if (versionParts[0] == "x" || versionParts[0] == "X")
								flags &= ~(MDVT_CheckMajor);
							else
							{
								major = atoi(versionParts[0].c_str());
								flags |= MDVT_CheckMajor;
							}
						}

						if (versionParts.size() > 1)
						{
							if (versionParts[1] == "x" || versionParts[1] == "X")
								flags &= ~(MDVT_CheckMinor);
							else
							{
								minor = atoi(versionParts[1].c_str());
								flags |= MDVT_CheckMinor;
							}
						}

						if (versionParts.size() > 2)
						{
							if (versionParts[2] == "x" || versionParts[2] == "X")
								flags &= ~(MDVT_CheckPatch);
							else
							{
								patch = atoi(versionParts[2].c_str());
								flags |= MDVT_CheckPatch;
							}
						}

						SetModuleDependencyInfoVersion(dependenciesInfo[i], major, minor, patch, flags);
					}
				}

				SetModuleInfoDependencies(moduleInfos, dependenciesInfo, dependenciesInfoCount);

				for (int i = 0; i < dependenciesInfoCount; i++)
					DestroyModuleDependencyInfo(dependenciesInfo[i]);
				delete[] dependenciesInfo;
			}
		}
		else if (dependenciesIt.value().is_object())
		{
			const json& dependenciesJSON = dependenciesIt.value();

			if (dependenciesJSON.size() > 0)
			{
				int dependenciesInfoCount = dependenciesJSON.size();
				ModuleDependencyInfo* dependenciesInfo = new ModuleDependencyInfo[dependenciesInfoCount];
				int i = 0;

				for (json::const_iterator dependencyInfoJSONIt = dependenciesJSON.begin(); dependencyInfoJSONIt != dependenciesJSON.end(); ++dependencyInfoJSONIt)
				{
					InitializeModuleDependencyInfo(dependenciesInfo[i]);

					std::string name = dependencyInfoJSONIt.key();
					std::string version = dependencyInfoJSONIt.value().get<std::string>();
					
					int major = 0;
					int minor = 0;
					int patch = 0;
					int flags = MDVT_CheckMajor | MDVT_CheckMinor;

					std::vector<std::string> versionParts;
					SplitString(version, versionParts, ".");

					if (versionParts.size() > 0)
					{
						if (versionParts[0] == "x" || versionParts[0] == "X")
							flags &= ~(MDVT_CheckMajor);
						else
						{
							major = atoi(versionParts[0].c_str());
							flags |= MDVT_CheckMajor;
						}
					}

					if (versionParts.size() > 1)
					{
						if (versionParts[1] == "x" || versionParts[1] == "X")
							flags &= ~(MDVT_CheckMinor);
						else
						{
							minor = atoi(versionParts[1].c_str());
							flags |= MDVT_CheckMinor;
						}
					}

					if (versionParts.size() > 2)
					{
						if (versionParts[2] == "x" || versionParts[2] == "X")
							flags &= ~(MDVT_CheckPatch);
						else
						{
							patch = atoi(versionParts[2].c_str());
							flags |= MDVT_CheckPatch;
						}
					}

					SetModuleDependencyInfoName(dependenciesInfo[i], name.c_str());
					SetModuleDependencyInfoVersion(dependenciesInfo[i], major, minor, patch, flags);

					i++;
				}

				SetModuleInfoDependencies(moduleInfos, dependenciesInfo, dependenciesInfoCount);

				for (int i = 0; i < dependenciesInfoCount; i++)
					DestroyModuleDependencyInfo(dependenciesInfo[i]);
				delete[] dependenciesInfo;
			}
		}
	}

	// Find the module's description
	json::iterator descriptionIt = moduleJSON.find("description");
	if (descriptionIt != moduleJSON.end() && descriptionIt.value().is_string())
		SetModuleInfoDescription(moduleInfos, descriptionIt.value().get<std::string>().c_str());

	// Find the module's author
	json::iterator authorIt = moduleJSON.find("author");
	if (authorIt != moduleJSON.end() && authorIt.value().is_string())
		SetModuleInfoAuthor(moduleInfos, authorIt.value().get<std::string>().c_str());

	// Find the module's website
	json::iterator websiteIt = moduleJSON.find("website");
	if (websiteIt != moduleJSON.end() && websiteIt.value().is_string())
		SetModuleInfoWebsite(moduleInfos, websiteIt.value().get<std::string>().c_str());

	// Find the module's issues
	json::iterator issuesIt = moduleJSON.find("issues");
	if (issuesIt != moduleJSON.end() && issuesIt.value().is_string())
		SetModuleInfoIssues(moduleInfos, issuesIt.value().get<std::string>().c_str());

	// Find the module's license
	json::iterator licenseIt = moduleJSON.find("license");
	if (licenseIt != moduleJSON.end() && licenseIt.value().is_string())
		SetModuleInfoLicense(moduleInfos, licenseIt.value().get<std::string>().c_str());

	// Find the module's version
	json::iterator versionIt = moduleJSON.find("version");
	if (versionIt != moduleJSON.end())
	{
		int major = 0;
		int minor = 0;
		int patch = 0;

		if (versionIt.value().is_string())
		{
			std::string version = versionIt.value().get<std::string>();
			std::vector<std::string> versionParts;

			SplitString(version, versionParts, ".");

			if (versionParts.size() > 0)
				major = atoi(versionParts[0].c_str());

			if (versionParts.size() > 1)
				minor = atoi(versionParts[1].c_str());

			if (versionParts.size() > 2)
				patch = atoi(versionParts[2].c_str());
		}
		else if (versionIt.value().is_object())
		{
			const json& version = versionIt.value();

			json::const_iterator majorIt = version.find("major");
			json::const_iterator minorIt = version.find("minor");
			json::const_iterator patchIt = version.find("patch");

			if (majorIt != version.end() && majorIt.value().is_number_integer())
				major = majorIt.value().get<int>();

			if (minorIt != version.end() && minorIt.value().is_number_integer())
				minor = minorIt.value().get<int>();

			if (patchIt != version.end() && patchIt.value().is_number_integer())
				patch = majorIt.value().get<int>();
		}

		SetModuleInfoVersion(moduleInfos, major, minor, patch);
	}
}

void ModulesManager::OnModuleLoaded(Module* module)
{
	if (module == nullptr)
		return;

	auto& pair = _loadedModules->find(module->_infos._name);
	if (pair != _loadedModules->end())
	{
		if(!pair->second)
			pair->second.reset(module);
	}
}

void ModulesManager::OnModuleUnloaded(Module* module)
{
	if (module == nullptr)
		return;

	auto& pair = _loadedModules->find(module->_infos._name);
	if (pair != _loadedModules->end())
	{
		if(pair->second.use_count() > 0)
			pair->second.reset();
	}
}