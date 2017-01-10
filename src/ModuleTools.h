#ifndef __ModuleTools_INCLUDE_H__
#define __ModuleTools_INCLUDE_H__

#include "CppPlugDefs.h"

#include <stdlib.h>
#include <string.h>

#define CppPlug_INTERNAL_DECLARE_PLUGIN_COMMON(Name)								\
	static void _cpp_plug_Destroy(void* obj) { delete ((Name*)obj); }				\
	static const char* _cpp_plug_SaveDataForReload(void* obj)						\
	{ return ((Name*)obj)->SaveDataForReload(); }									\
	static void _cpp_plug_LoadDataAfterReload(void* obj, const char* data)			\
	{ ((Name*)obj)->LoadDataAfterReload(data); }									\
	static bool _cpp_plug_OnMessage(void* obj, const char* msgName, void* msgData)	\
	{ return ((Name*)obj)->OnMessage(msgName, msgData); }

#define DECLARE_PLUGIN(Name)																		\
	public:																							\
		static void* _cpp_plug_Create(ModuleInfo* dependencies, int dependenciesCount, void* data)	\
		{ return new Name(dependencies, dependenciesCount, data); }									\
		CppPlug_INTERNAL_DECLARE_PLUGIN_COMMON(Name)

#define DECLARE_PLUGIN_NO_CREATION_DATA(Name)														\
	public:																							\
		static void* _cpp_plug_Create(ModuleInfo* dependencies, int dependenciesCount, void* data)	\
		{ return new Name(dependencies, dependenciesCount); }										\
		CppPlug_INTERNAL_DECLARE_PLUGIN_COMMON(Name)

#define DECLARE_PLUGIN_NO_DEPENDENCIES(Name)														\
	public:																							\
		static void* _cpp_plug_Create(ModuleInfo* dependencies, int dependenciesCount, void* data)	\
		{ return new Name(data); }																	\
		CppPlug_INTERNAL_DECLARE_PLUGIN_COMMON(Name)

#define DECLARE_PLUGIN_NO_CREATION_DATA_NO_DEPENDENCIES(Name)										\
	public:																							\
		static void* _cpp_plug_Create(ModuleInfo* dependencies, int dependenciesCount, void* data)	\
		{ return new Name(); }																		\
		CppPlug_INTERNAL_DECLARE_PLUGIN_COMMON(Name)

#define REGISTER_PLUGIN(Name, Service)	\
	AddPluginInfo(pluginsInfo, pluginsInfoCount, #Name, #Service, Name::_cpp_plug_Create, Name::_cpp_plug_Destroy, Name::_cpp_plug_SaveDataForReload, Name::_cpp_plug_LoadDataAfterReload, Name::_cpp_plug_OnMessage);

#define REGISTER_PLUGIN_EX(Name, Service, CreateFunc, DestroyFunc, SaveDataForReloadFunc, LoadDataAferReloadFunc, OnMessageFunc)	\
	AddPluginInfo(pluginsInfo, pluginsInfoCount, #Name, #Service, CreateFunc, DestroyFunc, SaveDataForReloadFunc, LoadDataAferReloadFunc, OnMessageFunc);

#define EXPORT_MODULE(ExportApi, PluginsRegistrations)															\
	extern _C_EXPORT_ ExportApi struct LoadModuleResult LoadModule();											\
	extern _C_EXPORT_ ExportApi int UnloadModule();																\
	extern _C_EXPORT_ ExportApi bool __get_registered_plugins(PluginInfo** pluginsInfo, int& pluginsInfoCount)	\
	{																											\
		if(pluginsInfo == nullptr || *pluginsInfo != nullptr)													\
			return false;																						\
		pluginsInfoCount = 0;																					\
		PluginsRegistrations																					\
		return true;																							\
	}

#define EXPORT_MODULE_EX(ExportApi, PluginsRegistrations)														\
	extern _C_EXPORT_ ExportApi struct LoadModuleResult LoadModule();											\
	extern _C_EXPORT_ ExportApi int UnloadModule();																\
	extern _C_EXPORT_ ExportApi int ReloadModule();																\
	extern _C_EXPORT_ ExportApi bool __get_registered_plugins(PluginInfo** pluginsInfo, int& pluginsInfoCount)	\
	{																											\
		if(pluginsInfo == nullptr || *pluginsInfo != nullptr)													\
			return false;																						\
		pluginsInfoCount = 0;																					\
		PluginsRegistrations																					\
		return true;																							\
	}

#ifdef __cplusplus
extern "C" {
#endif

/** Forward declaration of used types */
struct PluginInfo;
struct CreatedPlugin;
struct ModuleDependencyInfo;
struct ModuleBinaryInfo;
struct ModuleInfo;
struct LoadModuleResult;

/** Used enumerations */
enum ModuleDependencyVersionType
{
	MDVT_Any = 0x0,			/** Any version of the module can be used */
	MDVT_CheckMajor = 0x1,	/** Only the versions with the same major release number can be used */
	MDVT_CheckMinor = 0x2,	/** Only the versions with the same minor release number can be used */
	MDVT_CheckPatch = 0x4	/** Only the versions with the same patch release number can be used */
};

enum ModuleType
{
	MT_NotValid = 0,
	MT_Managed = 1,
	MT_Unmanaged = 2
};

enum ArchType
{
	AT_NotValid = 0,
	AT_x86 = 1,
	AT_x64 = 2,
	AT_All = 3
};

enum OperatingSystem
{
	OS_NotValid = 0,
	OS_Windows = 1,
	OS_Mac = 2,
	OS_Linux = 3,
	OS_All = 4
};

/** Plugin functions */
typedef void*(*__createPluginFunc)(ModuleInfo*, int, void*);
typedef void(*__destroyPluginFunc)(void*);
typedef bool(*__onMessagePluginFunc)(void*, const char*, void*);
typedef const char*(*__saveDataForPluginReloadFunc)(void*);
typedef void(*__loadDataAfterPluginReloadFunc)(void*, const char*);

typedef CreatedPlugin*(*__moduleCreatePluginFunc)(void*, const char*, void*);
typedef bool(*__moduleDestroyPluginFunc)(void*, CreatedPlugin*);
typedef bool(*__moduleSendMessageToPluginFunc)(CreatedPlugin*, const char*, void*);

/** Module functions */
typedef int(*__unloadModuleFunc)();
typedef int(*__reloadModuleFunc)();

typedef LoadModuleResult(*__loadModuleFunc)();
typedef bool(*__getRegisteredPluginsFunc)(PluginInfo**, int&);

/** Data structures */
struct PluginInfo
{
	char* _name;													/** Name of the plugin (could be the class name if the plugin is implemented as a class) */
	char* _service;													/** Name of the service this plugin is for */
	__createPluginFunc _createPluginFunc;							/** Pointer to the creation function of the plugin */
	__destroyPluginFunc _destroyPluginFunc;							/** Pointer to the destruction function of the plugin */
	__onMessagePluginFunc _onMessagePluginFunc;						/** Pointer to the plugin's function that will handle the messages coming from the plugin's user */
	__saveDataForPluginReloadFunc _saveDataForPluginReloadFunc;		/** Function called to let the plugin store its data before getting reloaded due to the module being reloaded */
	__loadDataAfterPluginReloadFunc _loadDataAfterPluginReloadFunc;	/** Function called to let the plugin load its data afters is has been reloaded due to the module being reloaded */

	/** Reserved attributes */

	void* _reserved_module;
};

struct CreatedPlugin
{
	/** Reserved attributes */

	void* _reserved_plugin;
	ModuleInfo* _reserved_module_info;
	PluginInfo* _reserved_plugin_info;
};

struct ModuleDependencyInfo
{
	char* _name;				/** Name of the dependency module (must match the name given for ModuleInfo::_name */
	int _versionMajor;			/** Major release number of the dependency module */
	int _versionMinor;			/** Minor release number of the dependency module */
	int _versionPatch;			/** Patch release number of the dependency module */
	int _versionDependencyFlag;	/** Use a combination of ModuleDependencyVersionType values (defaults to MDVT_CheckMajor | MDVT_CheckMinor) */
};

struct ModuleBinaryInfo
{
	char* _path;				/** Path to the binary. If the binary can be auto compiled than this must contain the path to the compiled binary. Can be relative (starting from where the module's package.json is) */
	ArchType _archType;			/** The architecture supported by the binary */
	OperatingSystem _os;		/** The operating system supported by the binary */
	bool _autoCompile;			/** If true the binary can be auto compiled with the additional informations */
	char* _entryPointNamespace;	/** Valid only for managed modules: namespace that contains the class that contains the module's management methods (see _entryPointClass) */
	char* _entryPointClass;		/** Valid only for managed modules: name of the class that contains the module's management methods (LoadModule, ReloadModule, UnloadModule) */
	char* _compileCommand;		/** This is the command executed to compile the binary (overrides all the other parameters like _sourceFiles, _compiler, _compilerArgs, _linker, _linkerArgs) */
	char* _sourceFiles;			/** If _compileCommand isn't specified and _autoCompile is true, this must contain the list of files to be compiled separated by a semicolon */
	char* _compiler;			/** If _compileCommand isn't specified and _autoCompile is true, this must contain the name of the compiler to be used to compile the binary */
	char* _compilerArgs;		/** If _compileCommand isn't specified and _autoCompile is true, this can contain the additional parameters to give to the compiler specified to compile the binary */
	char* _linker;				/** If _compileCommand isn't specified and _autoCompile is true, this can contain the name of the linker to be used to link the binary */
	char* _linkerArgs;			/** If _compileCommand isn't specified and _autoCompile is true, this can contain the additional parameters to give to the linker specified to link the binary */
};

struct ModuleInfo
{
	char* _name;							/** Name of the module */
	ModuleType _moduleType;					/** The type of the module (managed or unmanaged) */
	ModuleBinaryInfo* _binaries;			/** List of all the possible binaries that contains the module (divided by architecture type) */
	int _binariesCount;						/** Number of binaries */
	char* _description;						/** Brief description of the module */
	char* _author;							/** Author of the module (can contain name, email, website, etc. */
	char* _website;							/** Website of the module */
	char* _issues;							/** Link to a page where module's issues can be added */
	char* _license;							/** The license of the module (GPL, MIT, etc.) */
	ModuleDependencyInfo* _dependencies;	/** List of module that the module depends on */
	int _dependenciesCount;					/** Number of dependencies */
	int _versionMajor;						/** Major version of the module */
	int _versionMinor;						/** Minor version of the module */
	int _versionPatch;						/** Patch version of the module */

	/** Reserved attributes*/

	void* _reserved_module;
	__moduleCreatePluginFunc _reserved_createPluginFunc;
	__moduleDestroyPluginFunc _reserved_destroyPluginFunc;
	__moduleSendMessageToPluginFunc _reserved_sendMessageToPluginFunc;

	PluginInfo* _reserved_registeredPlugins;	/** After the module is loaded this is the list of plugins that it has registered */
	int _reserved_registeredPluginsCount;		/** The number of registered plugins */

	ModuleInfo* _reserved_dependencies;
	int _reserved_dependenciesCount;
};

struct LoadModuleResult
{
	__unloadModuleFunc _unloadModuleFunc;	/** Pointer to the function that gets called to unload the module */
	__reloadModuleFunc _reloadModuleFunc;	/** Pointer to the function that gets called to reload the module */
};

/** Function to operate on ModuleDependencyInfo */
extern _C_EXPORT_ CppPlug_API void InitializeModuleDependencyInfo(ModuleDependencyInfo& info);
extern _C_EXPORT_ CppPlug_API void DestroyModuleDependencyInfo(ModuleDependencyInfo& info);
extern _C_EXPORT_ CppPlug_API void CopyModuleDependencyInfo(ModuleDependencyInfo& dst, const ModuleDependencyInfo& src);
extern _C_EXPORT_ CppPlug_API void SetModuleDependencyInfoName(ModuleDependencyInfo& info, const char* value);
extern _C_EXPORT_ CppPlug_API void SetModuleDependencyInfoVersion(ModuleDependencyInfo& info, int major, int minor, int patch, int flag);

/** Function to operate on ModuleBinaryInfo */
extern _C_EXPORT_ CppPlug_API void InitializeModuleBinaryInfo(ModuleBinaryInfo& info);
extern _C_EXPORT_ CppPlug_API void DestroyModuleBinaryInfo(ModuleBinaryInfo& info);
extern _C_EXPORT_ CppPlug_API void CopyModuleBinaryInfo(ModuleBinaryInfo& dst, const ModuleBinaryInfo& src);
extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoPath(ModuleBinaryInfo& info, const char* value);
extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoOperatingSystem(ModuleBinaryInfo& info, OperatingSystem value);
extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoArchType(ModuleBinaryInfo& info, ArchType value);
extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoAutoCompile(ModuleBinaryInfo& info, bool value);
extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoEntryPointNamespace(ModuleBinaryInfo& info, const char* value);
extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoEntryPointClass(ModuleBinaryInfo& info, const char* value);
extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoCompileCommand(ModuleBinaryInfo& info, const char* value);
extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoSourceFiles(ModuleBinaryInfo& info, const char* value);
extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoCompiler(ModuleBinaryInfo& info, const char* value);
extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoCompilerArgs(ModuleBinaryInfo& info, const char* value);
extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoLinker(ModuleBinaryInfo& info, const char* value);
extern _C_EXPORT_ CppPlug_API void SetModuleBinaryInfoLinkerArgs(ModuleBinaryInfo& info, const char* value);

/** Function to operate on ModuleInfo */
extern _C_EXPORT_ CppPlug_API void InitializeModuleInfo(ModuleInfo& info);
extern _C_EXPORT_ CppPlug_API void DestroyModuleInfo(ModuleInfo& info);
extern _C_EXPORT_ CppPlug_API void CopyModuleInfo(ModuleInfo& dst, const ModuleInfo& src);
extern _C_EXPORT_ CppPlug_API void SetModuleInfoName(ModuleInfo& info, const char* value);
extern _C_EXPORT_ CppPlug_API void SetModuleInfoModuleType(ModuleInfo& info, ModuleType value);
extern _C_EXPORT_ CppPlug_API void SetModuleInfoBinaries(ModuleInfo& info, ModuleBinaryInfo* value, int valueCount);
extern _C_EXPORT_ CppPlug_API void SetModuleInfoDescription(ModuleInfo& info, const char* value);
extern _C_EXPORT_ CppPlug_API void SetModuleInfoAuthor(ModuleInfo& info, const char* value);
extern _C_EXPORT_ CppPlug_API void SetModuleInfoWebsite(ModuleInfo& info, const char* value);
extern _C_EXPORT_ CppPlug_API void SetModuleInfoIssues(ModuleInfo& info, const char* value);
extern _C_EXPORT_ CppPlug_API void SetModuleInfoLicense(ModuleInfo& info, const char* value);
extern _C_EXPORT_ CppPlug_API void SetModuleInfoDependencies(ModuleInfo& info, ModuleDependencyInfo* value, int valueCount);
extern _C_EXPORT_ CppPlug_API void SetModuleInfoVersion(ModuleInfo& info, int major, int minor, int patch);

/** Functions to operate on PluginInfo*/
static void InitializePluginInfo(PluginInfo& info) { memset(&info, 0, sizeof(PluginInfo)); }
static void DestroyPluginInfo(PluginInfo& info)
{
	if (info._name != nullptr)
		free((void*)info._name);

	if (info._service != nullptr)
		free((void*)info._service);

	memset(&info, 0, sizeof(PluginInfo));
}
static void SetPluginInfoName(PluginInfo& info, const char* value)
{
	if (value == nullptr)
		return;

	if (info._name != nullptr)
		free((void*)info._name);

	int len = strlen(value) + 1;
	info._name = (char*)malloc(len);

	strcpy(info._name, value);
}
static void SetPluginInfoService(PluginInfo& info, const char* value)
{
	if (value == nullptr)
		return;

	if (info._service != nullptr)
		free((void*)info._service);

	int len = strlen(value) + 1;
	info._service = (char*)malloc(len);

	strcpy(info._service, value);
}
static void SetPluginInfoCreateFunction(PluginInfo& info, __createPluginFunc value) { info._createPluginFunc = value; }
static void SetPluginInfoDestroyFunction(PluginInfo& info, __destroyPluginFunc value) { info._destroyPluginFunc = value; }
static void SetPluginInfoSaveDataForReloadFunction(PluginInfo& info, __saveDataForPluginReloadFunc value) { info._saveDataForPluginReloadFunc = value; }
static void SetPluginInfoLoadDataAfterReloadFunction(PluginInfo& info, __loadDataAfterPluginReloadFunc value) { info._loadDataAfterPluginReloadFunc = value; }
static void SetPluginInfoOnMessageFunction(PluginInfo& info, __onMessagePluginFunc value) { info._onMessagePluginFunc = value; }
static void CopyPluginInfo(PluginInfo& dst, const PluginInfo& src)
{
	DestroyPluginInfo(dst);

	SetPluginInfoName(dst, src._name);
	SetPluginInfoService(dst, src._service);
	SetPluginInfoCreateFunction(dst, src._createPluginFunc);
	SetPluginInfoDestroyFunction(dst, src._destroyPluginFunc);
	SetPluginInfoSaveDataForReloadFunction(dst, src._saveDataForPluginReloadFunc);
	SetPluginInfoLoadDataAfterReloadFunction(dst, src._loadDataAfterPluginReloadFunc);
	SetPluginInfoOnMessageFunction(dst, src._onMessagePluginFunc);

	dst._reserved_module = src._reserved_module;
}

static void AddPluginInfo(PluginInfo** infos, int& infosCount, const char* name, const char* service, __createPluginFunc createFunc, __destroyPluginFunc destroyFunc, __saveDataForPluginReloadFunc saveDataForReloadFunc, __loadDataAfterPluginReloadFunc loadDataAfterReloadFunc, __onMessagePluginFunc onMessageFunc)
{
	// Make sure the data is valid
	if (infos == nullptr || name == nullptr || service == nullptr || createFunc == nullptr || destroyFunc == nullptr || onMessageFunc == nullptr)
		return;

	// Reallocate to add enough space to add a new plugin info
	*infos = (PluginInfo*)realloc(*infos, sizeof(PluginInfo) * (infosCount + 1));

	// Initialize the new plugin info
	InitializePluginInfo((*infos[infosCount]));

	// Set the plugin info data
	SetPluginInfoName((*infos[infosCount]), name);
	SetPluginInfoService((*infos[infosCount]), service);
	SetPluginInfoCreateFunction((*infos[infosCount]), createFunc);
	SetPluginInfoDestroyFunction((*infos[infosCount]), destroyFunc);
	SetPluginInfoSaveDataForReloadFunction((*infos[infosCount]), saveDataForReloadFunc);
	SetPluginInfoLoadDataAfterReloadFunction((*infos[infosCount]), loadDataAfterReloadFunc);
	SetPluginInfoOnMessageFunction((*infos[infosCount]), onMessageFunc);

	// Increment the number of plugin infos
	infosCount++;
}

static void ClearPluginInfos(PluginInfo** infos, int& infosCount)
{
	// Make sure the data is valid
	if (infos == nullptr || infosCount <= 0)
		return;

	for (int i = 0; i < infosCount; i++)
		DestroyPluginInfo(*(infos[i]));

	free(*infos);
	infosCount = 0;

	*infos = nullptr;
}

/** Functions to operate on plugins */
static CreatedPlugin* CreatePlugin(ModuleInfo* module, const char* pluginName, void* creationData = nullptr)
{
	if (module == nullptr || module->_reserved_createPluginFunc == nullptr || pluginName == nullptr)
		return nullptr;
	return module->_reserved_createPluginFunc(module->_reserved_module, pluginName, creationData);
}

static bool DestroyPlugin(CreatedPlugin* plugin)
{
	if (plugin == nullptr || plugin->_reserved_module_info == nullptr || plugin->_reserved_module_info->_reserved_destroyPluginFunc == nullptr)
		return false;
	return plugin->_reserved_module_info->_reserved_destroyPluginFunc(plugin->_reserved_module_info, plugin);
}

static bool SendMessageToPlugin(CreatedPlugin* plugin, const char* messageName, void* messageData)
{
	if (plugin == nullptr || messageName == nullptr || plugin->_reserved_module_info == nullptr || plugin->_reserved_plugin == nullptr || plugin->_reserved_module_info->_reserved_sendMessageToPluginFunc == nullptr)
		return false;
	return plugin->_reserved_module_info->_reserved_sendMessageToPluginFunc(plugin, messageName, messageData);
}

/** Functions to operate on module's dependencies */
static ModuleInfo* GetDependency(ModuleInfo* dependencies, int dependenciesCount, const char* wantedDependencyName)
{
	if (dependencies == nullptr || dependenciesCount <= 0 || wantedDependencyName == nullptr)
		return nullptr;

	for (int i = 0; i < dependenciesCount; i++)
	{
		if (strcmp(dependencies[i]._name, wantedDependencyName) == 0)
			return &dependencies[i];
	}

	return nullptr;
}

#ifdef  __cplusplus
}
#endif

#endif //__ModuleTools_INCLUDE_H__