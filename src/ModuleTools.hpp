#ifndef __ModuleTools_INCLUDE_HPP__
#define __ModuleTools_INCLUDE_HPP__

#include "ModuleTools.h"

#include <vector>

class CModuleInfo;

typedef void*(*__cpp_createPluginFunc)(std::vector<CModuleInfo>, void*);

static void* CreatePluginFuncBridge()

class CppPlug_API CPluginInfo
{
	friend class ManagedModule;

public:
	static CPluginInfo FromPluginInfo(PluginInfo* pluginInfo) { return CPluginInfo(pluginInfo); }

	CPluginInfo(PluginInfo* pluginInfo)
		: _pluginInfo(pluginInfo)
		, _ownsInfos(false)
	{ }

	CPluginInfo()
		: _pluginInfo(new PluginInfo())
		, _ownsInfos(true)
	{
		InitializePluginInfo(*_pluginInfo);
	}

	CPluginInfo(const char* name, const char* service, __cpp_createPluginFunc createPluginFunc, __destroyPluginFunc destroyPluginFunc, __onMessagePluginFunc onMessagePluginFunc, __saveDataForPluginReloadFunc saveDataForPluginReloadFunc, __loadDataAfterPluginReloadFunc loadDataAfterPluginReloadFunc)
		: _pluginInfo(new PluginInfo())
		, _ownsInfos(true)
	{
		InitializePluginInfo(*_pluginInfo);

		Name(name);
		Service(service);
		CreateFunc(createPluginFunc);
		DestroyFunc(destroyPluginFunc);
		OnMessageFunc(onMessagePluginFunc);
		SaveDataForReloadFunc(saveDataForPluginReloadFunc);
		LoadDataAfterReloadFunc(loadDataAfterPluginReloadFunc);
	}

	CPluginInfo(const CPluginInfo& src)
	{
		if (_ownsInfos)
			CopyPluginInfo(*_pluginInfo, *src._pluginInfo);
		else
			_pluginInfo = src._pluginInfo;
	}

	~CPluginInfo()
	{
		if (_ownsInfos)
		{
			DestroyPluginInfo(*_pluginInfo);
			delete _pluginInfo;
		}
	}

public:
	void Name(const char* name) { SetPluginInfoName(*_pluginInfo, name); }
	const char* Name() const { return _pluginInfo->_name != nullptr ? _pluginInfo->_name : ""; }

	void Service(const char* service) { SetPluginInfoService(*_pluginInfo, service); }
	const char* Service() const { return _pluginInfo->_service != nullptr ? _pluginInfo->_service : ""; }
	
	void CreateFunc(__cpp_createPluginFunc createPluginFunc) { SetPluginInfoCreateFunction(*_pluginInfo, createPluginFunc); }
	__cpp_createPluginFunc CreateFunc() const { return _pluginInfo->_createPluginFunc; }

	void DestroyFunc(__destroyPluginFunc destroyPluginFunc) { SetPluginInfoDestroyFunction(*_pluginInfo, destroyPluginFunc); }
	__destroyPluginFunc DestroyFunc() const { return _pluginInfo->_destroyPluginFunc; }

	void OnMessageFunc(__onMessagePluginFunc onMessagePluginFunc) { SetPluginInfoOnMessageFunction(*_pluginInfo, onMessagePluginFunc); }
	__onMessagePluginFunc OnMessageFunc() const { return _pluginInfo->_onMessagePluginFunc; }

	void SaveDataForReloadFunc(__saveDataForPluginReloadFunc saveDataForPluginReloadFunc) { SetPluginInfoSaveDataForReloadFunction(*_pluginInfo, saveDataForPluginReloadFunc); }
	__saveDataForPluginReloadFunc SaveDataForReloadFunc() const { return _pluginInfo->_saveDataForPluginReloadFunc; }

	void LoadDataAfterReloadFunc(__loadDataAfterPluginReloadFunc loadDataAfterPluginReloadFunc) { SetPluginInfoLoadDataAfterReloadFunction(*_pluginInfo, loadDataAfterPluginReloadFunc); }
	__loadDataAfterPluginReloadFunc LoadDataAfterReloadFunc() const { return _pluginInfo->_loadDataAfterPluginReloadFunc; }

private:
	bool _ownsInfos;
	PluginInfo* _pluginInfo;
};

class CppPlug_API CCreatedPlugin
{
public:
	static CCreatedPlugin FromCreatedPlugin(CreatedPlugin* createdPlugin) { return CCreatedPlugin(createdPlugin); }

	CCreatedPlugin(CreatedPlugin* createdPlugin)
		: _createdPlugin(createdPlugin)
		, _ownsInfos(false)
	{ }

	CCreatedPlugin(const CCreatedPlugin& src)
	{
		_createdPlugin = src._createdPlugin;
	}

	~CCreatedPlugin() { }

public:
	bool Destroy()
	{
		bool result = false;
		if (_createdPlugin != nullptr)
		{
			result = ::DestroyPlugin(_createdPlugin);
			_createdPlugin = nullptr;
		}
		
		return result;
	}

	bool SendMessage(const char* messageName, void* messageData)
	{
		if (_createdPlugin != nullptr)
			return ::SendMessageToPlugin(_createdPlugin, messageName, messageData);
		return false;
	}
	
private:
	bool _ownsInfos;
	CreatedPlugin* _createdPlugin;
};

class CppPlug_API CModuleDependencyInfo
{
public:
	static CModuleDependencyInfo FromModuleDependencyInfo(ModuleDependencyInfo* moduleDependencyInfo) { return CModuleDependencyInfo(moduleDependencyInfo); }

	CModuleDependencyInfo(ModuleDependencyInfo* moduleDependencyInfo)
		: _moduleDependencyInfo(moduleDependencyInfo)
		, _ownsInfos(false)
	{ }

	CModuleDependencyInfo(const CModuleDependencyInfo& src)
	{
		if (_ownsInfos)
			CopyModuleDependencyInfo(*_moduleDependencyInfo, *src._moduleDependencyInfo);
		else
			_moduleDependencyInfo = src._moduleDependencyInfo;
	}

	~CModuleDependencyInfo()
	{
		if (_ownsInfos)
		{
			DestroyModuleDependencyInfo(*_moduleDependencyInfo);
			delete _moduleDependencyInfo;
		}
	}

public:
	const char* Name() const
	{ 
		return _moduleDependencyInfo == nullptr ? "" : (_moduleDependencyInfo->_name != nullptr ? _moduleDependencyInfo->_name : "");
	}

	void Version(int& major, int& minor, int& patch, int& flags) const
	{ 
		if (_moduleDependencyInfo == nullptr)
		{
			major = 0;
			minor = 0;
			patch = 0;
			flags = 0;
		}
		else
		{
			major = _moduleDependencyInfo->_versionMajor;
			minor = _moduleDependencyInfo->_versionMinor;
			patch = _moduleDependencyInfo->_versionPatch;
			flags = _moduleDependencyInfo->_versionDependencyFlag;
		}
	}

private:
	bool _ownsInfos;
	ModuleDependencyInfo* _moduleDependencyInfo;
};

class CppPlug_API CModuleInfo
{
public:
	static CModuleInfo FromModuleInfo(ModuleInfo* moduleInfo) { return CModuleInfo(moduleInfo); }

	CModuleInfo(ModuleInfo* moduleInfo)
		: _moduleInfo(moduleInfo)
		, _ownsInfos(false)
	{ }

	CModuleInfo()
		: _moduleInfo(new ModuleInfo())
		, _ownsInfos(true)
	{
		InitializeModuleInfo(*_moduleInfo);
	}

	CModuleInfo(const CModuleInfo& src)
	{
		if (_ownsInfos)
			CopyModuleInfo(*_moduleInfo, *src._moduleInfo);
		else
			_moduleInfo = src._moduleInfo;
	}

	~CModuleInfo()
	{
		if (_ownsInfos)
		{
			DestroyModuleInfo(*_moduleInfo);
			delete _moduleInfo;
		}
	}

public:
	void Name(const char* name)
	{
		if(_moduleInfo != nullptr)
			SetModuleInfoName(*_moduleInfo, name);
	}
	const char* Name() const
	{ 
		return _moduleInfo == nullptr ? "" : (_moduleInfo->_name != nullptr ? _moduleInfo->_name : "");
	}

	void Type(ModuleType type)
	{ 
		if (_moduleInfo != nullptr)
			SetModuleInfoModuleType(*_moduleInfo, type);
	}
	ModuleType Type() const
	{ 
		return _moduleInfo == nullptr ? MT_NotValid : _moduleInfo->_moduleType;
	}
	
	void Description(const char* description)
	{ 
		if (_moduleInfo != nullptr)
			SetModuleInfoDescription(*_moduleInfo, description); 
	}
	const char* Description() const 
	{ 
		return _moduleInfo == nullptr ? "" : (_moduleInfo->_description != nullptr ? _moduleInfo->_description : "");
	}

	void Author(const char* author) 
	{
		if (_moduleInfo != nullptr)
			SetModuleInfoAuthor(*_moduleInfo, author); 
	}
	const char* Author() const
	{ 
		return _moduleInfo == nullptr ? "" : (_moduleInfo->_author != nullptr ? _moduleInfo->_author : "");
	}

	void Website(const char* website) 
	{
		if (_moduleInfo != nullptr)
			SetModuleInfoWebsite(*_moduleInfo, website);
	}
	const char* Website() const
	{ 
		return _moduleInfo == nullptr ? "" : (_moduleInfo->_website != nullptr ? _moduleInfo->_website : "");
	}

	void Issues(const char* issues)
	{
		if (_moduleInfo != nullptr)
			SetModuleInfoIssues(*_moduleInfo, issues);
	}
	const char* Issues() const 
	{ 
		return _moduleInfo == nullptr ? "" : (_moduleInfo->_issues != nullptr ? _moduleInfo->_issues : "");
	}

	void License(const char* license) 
	{
		if (_moduleInfo != nullptr)
			SetModuleInfoLicense(*_moduleInfo, license);
	}
	const char* License() const 
	{ 
		return _moduleInfo == nullptr ? "" : (_moduleInfo->_license != nullptr ? _moduleInfo->_license : ""); 
	}

	void SetVersion(int major, int minor, int patch) 
	{
		if (_moduleInfo != nullptr)
			SetModuleInfoVersion(*_moduleInfo, major, minor, patch); 
	}
	void GetVersion(int& major, int& minor, int& patch) const
	{
		if (_moduleInfo == nullptr)
		{
			major = 0;
			minor = 0;
			patch = 0;
		}
		else
		{
			major = _moduleInfo->_versionMajor;
			minor = _moduleInfo->_versionMinor;
			patch = _moduleInfo->_versionPatch;
		}
	}

	const std::vector<CModuleDependencyInfo> DependenciesInfo() const
	{
		std::vector<CModuleDependencyInfo> result;

		if (_moduleInfo != nullptr)
		{
			result.reserve(_moduleInfo->_dependenciesCount);

			for (int i = 0; i < _moduleInfo->_dependenciesCount; i++)
				result.push_back(CModuleDependencyInfo::FromModuleDependencyInfo(&(_moduleInfo->_dependencies[i])));
		}

		return result;
	}

	const std::vector<CModuleInfo> Dependencies() const
	{
		std::vector<CModuleInfo> result;

		if (_moduleInfo != nullptr)
		{
			result.reserve(_moduleInfo->_reserved_dependenciesCount);

			for (int i = 0; i < _moduleInfo->_reserved_dependenciesCount; i++)
				result.push_back(CModuleInfo::FromModuleInfo(&(_moduleInfo->_reserved_dependencies[i])));
		}

		return result;
	}

	CCreatedPlugin CreatePlugin(const char* pluginName, void* creationData = nullptr)
	{
		return CCreatedPlugin(::CreatePlugin(_moduleInfo, pluginName, creationData));
	}
	
private:
	bool _ownsInfos;
	ModuleInfo* _moduleInfo;
};

class CppPlug_API CLoadModuleResult
{
public:
	CLoadModuleResult(__reloadModuleFunc reloadModuleFunc, __unloadModuleFunc unloadModuleFunc)
	{
		_loadModuleResult._reloadModuleFunc = reloadModuleFunc;
		_loadModuleResult._unloadModuleFunc = unloadModuleFunc;
	}

	CLoadModuleResult(const CLoadModuleResult& src)
	{
		_loadModuleResult._reloadModuleFunc = src._loadModuleResult._reloadModuleFunc;
		_loadModuleResult._unloadModuleFunc = src._loadModuleResult._unloadModuleFunc;
	}

	~CLoadModuleResult() { }

public:
	void ReloadModuleFunc(__reloadModuleFunc reloadModuleFunc) { _loadModuleResult._reloadModuleFunc = reloadModuleFunc; }
	__reloadModuleFunc ReloadModuleFunc() const { return _loadModuleResult._reloadModuleFunc; }

	void UnloadModuleFunc(__unloadModuleFunc unloadModuleFunc) { _loadModuleResult._unloadModuleFunc = unloadModuleFunc; }
	__unloadModuleFunc UnloadModuleFunc() const { return _loadModuleResult._unloadModuleFunc; }

private:
	LoadModuleResult _loadModuleResult;
};

#endif //__ModuleTools_INCLUDE_HPP__