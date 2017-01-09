#include "Module.h"
#include "ModulesManager.h"

#include "UnmanagedModule.h"
#include "ManagedModule.h"

#include <sstream>

Module* Module::FromModuleInfo(ModuleInfo* moduleInfo)
{
	if (moduleInfo == nullptr || moduleInfo->_reserved_module == nullptr || moduleInfo->_moduleType == MT_NotValid)
		return nullptr;
	return (Module*)moduleInfo->_reserved_module;
}

bool Module::CheckDependencies(std::string& error)
{
	_dependencies.clear();

	for (int i = 0; i <_infos._dependenciesCount; i++)
	{
		const ModuleDependencyInfo& dependencyInfo = _infos._dependencies[i];

		std::shared_ptr<Module> module = ModulesManager::GetInstance().GetModule(dependencyInfo._name);

		// We haven't found one of the requested modules
		if (!module)
		{
			std::stringstream ss;
			ss << "The module " << dependencyInfo._name << " wasn't loaded";
			error = ss.str();
			return false;
		}

		int major = module->GetVersionMajor();
		int minor = module->GetVersionMinor();
		int patch = module->GetVersionPatch();

		bool versionError = false;

		if (dependencyInfo._versionDependencyFlag & MDVT_CheckMajor && major < dependencyInfo._versionMajor)
			versionError = true;

		if (versionError == false && dependencyInfo._versionDependencyFlag & MDVT_CheckMinor && minor < dependencyInfo._versionMinor)
			versionError = true;

		if (versionError == false && dependencyInfo._versionDependencyFlag & MDVT_CheckPatch && patch < dependencyInfo._versionPatch)
			versionError = true;

		if (versionError)
		{
			std::stringstream ss;
			ss << "The module's " << dependencyInfo._name << " version is too old. Requested version is "
				<< dependencyInfo._versionMajor << "."
				<< dependencyInfo._versionMinor << "."
				<< dependencyInfo._versionPatch
				<< " but the loaded version is "
				<< major << "."
				<< minor << "."
				<< patch;
			error = ss.str();
			return false;
		}

		module->_dependantModules.push_back(this);
		_dependencies.push_back(&module->_infos);
	}

	if (_dependencies.size() > 0)
	{
		_infos._reserved_dependencies = _dependencies[0];
		_infos._reserved_dependenciesCount = (int)_dependencies.size();
	}
	else
	{
		_infos._reserved_dependencies = nullptr;
		_infos._reserved_dependenciesCount = 0;
	}

	return true;
}

/**
* Chooses the correct binary info based on the system architecture, giving priority to the actual architecture.
* If a binary for the current artchitecture isn't found then it looks for a binary marked as compatible with all architectures.
*/
void Module::ChooseBinaryInfos()
{
#if WIN32
	OperatingSystem wantedSystem = OS_Windows;
#endif

#if CppPlug_ENVIRONMENT_32_BIT
	ArchType wantedArch = AT_x86;
#else
	ArchType wantedArch = AT_x64;
#endif

	std::vector<ModuleBinaryInfo*> matchingArchInfos;

	for (int i = 0; i < _infos._binariesCount; i++)
	{
		if (_infos._binaries[i]._archType == wantedArch)
			matchingArchInfos.push_back(&_infos._binaries[i]);
	}

	if (matchingArchInfos.size() == 0)
	{
		for (int i = 0; i < _infos._binariesCount && _binaryInfo == nullptr; i++)
		{
			if (_infos._binaries[i]._archType == AT_All)
				matchingArchInfos.push_back(&_infos._binaries[i]);
		}
	}

	for (size_t i = 0; i < matchingArchInfos.size() && _binaryInfo == nullptr; i++)
	{
		if (matchingArchInfos[i]->_os == wantedSystem)
			_binaryInfo = matchingArchInfos[i];
	}

	if (_binaryInfo == nullptr)
	{
		for (size_t i = 0; i < matchingArchInfos.size() && _binaryInfo == nullptr; i++)
		{
			if (matchingArchInfos[i]->_os == OS_All)
				_binaryInfo = matchingArchInfos[i];
		}
	}
}

void Module::NotifyUnloadToManager()
{
	if (_manager != nullptr)
		_manager->OnModuleUnloaded(this);
}

void Module::NotifyUnloadToDependencies()
{
	for (auto& el : _dependencies)
	{
		Module* depencencyModule = (Module*)(el->_reserved_module);
		depencencyModule->OnDependantModuleUnloaded(this);
	}
}

void Module::OnDependantModuleUnloaded(Module* module)
{
	auto& el = std::find(_dependantModules.begin(), _dependantModules.end(), module);
	if (el != _dependantModules.end())
		_dependantModules.erase(el);
}

void Module::OnDependencyDestructed(Module* module)
{
	for(size_t i = 0; i < _dependencies.size(); i++)
	{
		if (_dependencies[i]->_reserved_module == module)
		{
			_dependencies.erase(_dependencies.begin() + i);
			return;
		}
	}
}

extern _C_EXPORT_ CppPlug_API void InitializeModuleDependencyInfo(ModuleDependencyInfo& info)
{
	memset(&info, 0, sizeof(ModuleDependencyInfo));
	info._versionDependencyFlag = MDVT_CheckMajor | MDVT_CheckMinor;
}

extern _C_EXPORT_ CppPlug_API void DestroyModuleDependencyInfo(ModuleDependencyInfo& info)
{
	if (info._name != nullptr)
		free((void*)info._name);

	memset(&info, 0, sizeof(ModuleDependencyInfo));
	info._versionDependencyFlag = MDVT_CheckMajor | MDVT_CheckMinor;
}

extern _C_EXPORT_ CppPlug_API void CopyModuleDependencyInfo(ModuleDependencyInfo& dst, const ModuleDependencyInfo& src)
{
	DestroyModuleDependencyInfo(dst);

	SetModuleDependencyInfoName(dst, src._name);
	SetModuleDependencyInfoVersion(dst, src._versionMajor, src._versionMinor, src._versionPatch, src._versionDependencyFlag);
}

extern _C_EXPORT_ CppPlug_API void SetModuleDependencyInfoName(ModuleDependencyInfo& info, const char* value)
{
	if (value == nullptr)
		return;

	if (info._name != nullptr)
		free((void*)info._name);

	int len = strlen(value) + 1;
	info._name = (char*)malloc(len);

	strcpy(info._name, value);
}

extern _C_EXPORT_ CppPlug_API void SetModuleDependencyInfoVersion(ModuleDependencyInfo& info, int major, int minor, int patch, int flag)
{
	info._versionMajor = major;
	info._versionMinor = minor;
	info._versionPatch = patch;
	info._versionDependencyFlag = flag;
}

extern _C_EXPORT_ CppPlug_API void InitializeModuleBinaryInfo(ModuleBinaryInfo& info)
{
	memset(&info, 0, sizeof(ModuleBinaryInfo));
}

extern _C_EXPORT_ CppPlug_API void DestroyModuleBinaryInfo(ModuleBinaryInfo& info)
{
	if (info._entryPointNamespace != nullptr)
		free((void*)info._entryPointNamespace);

	if (info._entryPointClass != nullptr)
		free((void*)info._entryPointClass);

	if (info._path != nullptr)
		free((void*)info._path);

	if (info._compileCommand != nullptr)
		free((void*)info._compileCommand);

	if (info._sourceFiles != nullptr)
		free((void*)info._sourceFiles);

	if (info._compiler != nullptr)
		free((void*)info._compiler);

	if (info._compilerArgs != nullptr)
		free((void*)info._compilerArgs);

	if (info._linker != nullptr)
		free((void*)info._linker);

	if (info._linkerArgs != nullptr)
		free((void*)info._linkerArgs);

	memset(&info, 0, sizeof(ModuleBinaryInfo));
}

extern _C_EXPORT_ CppPlug_API void CopyModuleBinaryInfo(ModuleBinaryInfo& dst, const ModuleBinaryInfo& src)
{
	DestroyModuleBinaryInfo(dst);

	SetModuleBinaryInfoPath(dst, src._path);
	SetModuleBinaryInfoOperatingSystem(dst, src._os);
	SetModuleBinaryInfoArchType(dst, src._archType);
	SetModuleBinaryInfoAutoCompile(dst, src._autoCompile);
	SetModuleBinaryInfoEntryPointNamespace(dst, src._entryPointNamespace);
	SetModuleBinaryInfoEntryPointClass(dst, src._entryPointClass);
	SetModuleBinaryInfoCompileCommand(dst, src._compileCommand);
	SetModuleBinaryInfoSourceFiles(dst, src._sourceFiles);
	SetModuleBinaryInfoCompiler(dst, src._compiler);
	SetModuleBinaryInfoCompilerArgs(dst, src._compilerArgs);
	SetModuleBinaryInfoLinker(dst, src._linker);
	SetModuleBinaryInfoLinkerArgs(dst, src._linkerArgs);
}

extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoPath(ModuleBinaryInfo& info, const char* value)
{
	if (value == nullptr)
		return;

	if (info._path != nullptr)
		free((void*)info._path);

	int len = strlen(value) + 1;
	info._path = (char*)malloc(len);

	strcpy(info._path, value);
}

extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoOperatingSystem(ModuleBinaryInfo& info, OperatingSystem value)
{
	info._os = value;
}

extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoArchType(ModuleBinaryInfo& info, ArchType value)
{
	info._archType = value;
}

extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoAutoCompile(ModuleBinaryInfo& info, bool value)
{
	info._autoCompile = value;
}

extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoEntryPointNamespace(ModuleBinaryInfo& info, const char* value)
{
	if (value == nullptr)
		return;

	if (info._entryPointNamespace != nullptr)
		free((void*)info._entryPointNamespace);

	int len = strlen(value) + 1;
	info._entryPointNamespace = (char*)malloc(len);

	strcpy(info._entryPointNamespace, value);
}

extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoEntryPointClass(ModuleBinaryInfo& info, const char* value)
{
	if (value == nullptr)
		return;

	if (info._entryPointClass != nullptr)
		free((void*)info._entryPointClass);

	int len = strlen(value) + 1;
	info._entryPointClass = (char*)malloc(len);

	strcpy(info._entryPointClass, value);
}

extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoCompileCommand(ModuleBinaryInfo& info, const char* value)
{
	if (value == nullptr)
		return;

	if (info._compileCommand != nullptr)
		free((void*)info._compileCommand);

	int len = strlen(value) + 1;
	info._compileCommand = (char*)malloc(len);

	strcpy(info._compileCommand, value);
}

extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoSourceFiles(ModuleBinaryInfo& info, const char* value)
{
	if (value == nullptr)
		return;

	if (info._sourceFiles != nullptr)
		free((void*)info._sourceFiles);

	int len = strlen(value) + 1;
	info._sourceFiles = (char*)malloc(len);

	strcpy(info._sourceFiles, value);
}

extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoCompiler(ModuleBinaryInfo& info, const char* value)
{
	if (value == nullptr)
		return;

	if (info._compiler != nullptr)
		free((void*)info._compiler);

	int len = strlen(value) + 1;
	info._compiler = (char*)malloc(len);

	strcpy(info._compiler, value);
}

extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoCompilerArgs(ModuleBinaryInfo& info, const char* value)
{
	if (value == nullptr)
		return;

	if (info._compilerArgs != nullptr)
		free((void*)info._compilerArgs);

	int len = strlen(value) + 1;
	info._compilerArgs = (char*)malloc(len);

	strcpy(info._compilerArgs, value);
}

extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoLinker(ModuleBinaryInfo& info, const char* value)
{
	if (value == nullptr)
		return;

	if (info._linker != nullptr)
		free((void*)info._linker);

	int len = strlen(value) + 1;
	info._linker = (char*)malloc(len);

	strcpy(info._linker, value);
}

extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoLinkerArgs(ModuleBinaryInfo& info, const char* value)
{
	if (value == nullptr)
		return;

	if (info._linkerArgs != nullptr)
		free((void*)info._linkerArgs);

	int len = strlen(value) + 1;
	info._linkerArgs = (char*)malloc(len);

	strcpy(info._linkerArgs, value);
}

extern _C_EXPORT_ CppPlug_API void InitializeModuleInfo(ModuleInfo& info)
{
	memset(&info, 0, sizeof(ModuleInfo));
}

extern _C_EXPORT_ CppPlug_API void DestroyModuleInfo(ModuleInfo& info)
{
	if (info._name != nullptr)
		free((void*)info._name);

	if (info._binaries != nullptr)
	{
		for (int i = 0; i < info._binariesCount; i++)
			DestroyModuleBinaryInfo(info._binaries[i]);

		free((void*)info._binaries);
	}

	if (info._description != nullptr)
		free((void*)info._description);

	if (info._author != nullptr)
		free((void*)info._author);

	if (info._website != nullptr)
		free((void*)info._website);

	if (info._issues != nullptr)
		free((void*)info._issues);

	if (info._license != nullptr)
		free((void*)info._license);

	if (info._dependencies != nullptr)
	{
		for (int i = 0; i < info._dependenciesCount; i++)
			DestroyModuleDependencyInfo(info._dependencies[i]);

		free((void*)info._dependencies);
	}

	memset(&info, 0, sizeof(ModuleInfo));
}

extern _C_EXPORT_ CppPlug_API void CopyModuleInfo(ModuleInfo& dst, const ModuleInfo& src)
{
	DestroyModuleInfo(dst);

	SetModuleInfoName(dst, src._name);
	SetModuleInfoModuleType(dst, src._moduleType);
	SetModuleInfoBinaries(dst, src._binaries, src._binariesCount);
	SetModuleInfoDescription(dst, src._description);
	SetModuleInfoAuthor(dst, src._author);
	SetModuleInfoWebsite(dst, src._website);
	SetModuleInfoIssues(dst, src._issues);
	SetModuleInfoLicense(dst, src._license);
	SetModuleInfoDependencies(dst, src._dependencies, src._dependenciesCount);
	SetModuleInfoVersion(dst, src._versionMajor, src._versionMinor, src._versionPatch);
}

extern _C_EXPORT_ CppPlug_API void SetModuleInfoName(ModuleInfo& info, const char* value)
{
	if (value == nullptr)
		return;

	if (info._name != nullptr)
		free((void*)info._name);

	int len = strlen(value) + 1;
	info._name = (char*)malloc(len);

	strcpy(info._name, value);
}

extern _C_EXPORT_ CppPlug_API void SetModuleInfoModuleType(ModuleInfo& info, ModuleType value)
{
	info._moduleType = value;
}

extern _C_EXPORT_ CppPlug_API void SetModuleInfoBinaries(ModuleInfo& info, ModuleBinaryInfo* value, int valueCount)
{
	if (value == nullptr || valueCount <= 0)
		return;

	if (info._binaries != nullptr)
		free((void*)info._binaries);

	info._binaries = (ModuleBinaryInfo*)malloc(sizeof(ModuleBinaryInfo) * valueCount);
	info._binariesCount = valueCount;

	for (int i = 0; i < valueCount; i++)
	{
		InitializeModuleBinaryInfo(info._binaries[i]);
		CopyModuleBinaryInfo(info._binaries[i], value[i]);
	}
}

extern _C_EXPORT_ CppPlug_API void SetModuleInfoDescription(ModuleInfo& info, const char* value)
{
	if (value == nullptr)
		return;

	if (info._description != nullptr)
		free((void*)info._description);

	int len = strlen(value) + 1;
	info._description = (char*)malloc(len);

	strcpy(info._description, value);
}

extern _C_EXPORT_ CppPlug_API void SetModuleInfoAuthor(ModuleInfo& info, const char* value)
{
	if (value == nullptr)
		return;

	if (info._author != nullptr)
		free((void*)info._author);

	int len = strlen(value) + 1;
	info._author = (char*)malloc(len);

	strcpy(info._author, value);
}

extern _C_EXPORT_ CppPlug_API void SetModuleInfoWebsite(ModuleInfo& info, const char* value)
{
	if (value == nullptr)
		return;

	if (info._website != nullptr)
		free((void*)info._website);

	int len = strlen(value) + 1;
	info._website = (char*)malloc(len);

	strcpy(info._website, value);
}

extern _C_EXPORT_ CppPlug_API void SetModuleInfoIssues(ModuleInfo& info, const char* value)
{
	if (value == nullptr)
		return;

	if (info._issues != nullptr)
		free((void*)info._issues);

	int len = strlen(value) + 1;
	info._issues = (char*)malloc(len);

	strcpy(info._issues, value);
}

extern _C_EXPORT_ CppPlug_API void SetModuleInfoLicense(ModuleInfo& info, const char* value)
{
	if (value == nullptr)
		return;

	if (info._license != nullptr)
		free((void*)info._license);

	int len = strlen(value) + 1;
	info._license = (char*)malloc(len);

	strcpy(info._license, value);
}

extern _C_EXPORT_ CppPlug_API void SetModuleInfoDependencies(ModuleInfo& info, ModuleDependencyInfo* value, int valueCount)
{
	if (value == nullptr || valueCount <= 0)
		return;

	if (info._dependencies != nullptr)
		free((void*)info._dependencies);
	
	info._dependencies = (ModuleDependencyInfo*)malloc(sizeof(ModuleDependencyInfo) * valueCount);
	info._dependenciesCount = valueCount;

	for (int i = 0; i < valueCount; i++)
	{
		InitializeModuleDependencyInfo(info._dependencies[i]);
		CopyModuleDependencyInfo(info._dependencies[i], value[i]);
	}
}

extern _C_EXPORT_ CppPlug_API void SetModuleInfoVersion(ModuleInfo& info, int major, int minor, int patch)
{
	info._versionMajor = major;
	info._versionMinor = minor;
	info._versionPatch = patch;
}