#ifndef __Module_INCLUDE_H__
#define __Module_INCLUDE_H__

#include "CppPlugDefs.h"
#include "ModuleTools.h"

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <functional>

class Plugin;
class ModulesManager;
class UnmanagedModule;
struct PluginDataToRestoreAfterReload;

#ifdef SUPPORT_MANAGED
	class ManagedModule;
#endif

class CppPlug_API Module
{
	friend ModulesManager;
	friend UnmanagedModule;

	#ifdef SUPPORT_MANAGED
		friend ManagedModule;
	#endif

public:
	typedef std::map<std::string, std::vector<PluginInfo*> > GivenServicesMap;
	typedef std::map<std::string, PluginInfo*> RegisteredPluginsMap;
	typedef std::vector<PluginInfo*> RegisteredPluginsVector;
	typedef std::vector<CreatedPlugin*> CreatedPluginsVector;
	typedef std::vector<PluginDataToRestoreAfterReload*> PluginDataToRestoreAfterReloadVector;

protected:
	Module(const ModuleInfo& moduleInfo, const std::string& basePath)
		: _manager(nullptr)
		, _basePath(basePath)
		, _binaryInfo(nullptr)
		, _givenServices(new GivenServicesMap())
		, _registeredPluginsMap(new RegisteredPluginsMap())
		, _registeredPluginsVec(new RegisteredPluginsVector())
		, _createdPlugins(new CreatedPluginsVector())
		, _pluginsDataToRestoreAfterReload(new PluginDataToRestoreAfterReloadVector())
	{
		InitializeModuleInfo(_infos);
		CopyModuleInfo(_infos, moduleInfo);

		ChooseBinaryInfos();
	}

public:
	virtual ~Module()
	{
		DestroyModuleInfo(_infos);

		delete _pluginsDataToRestoreAfterReload;
		delete _givenServices;
		delete _registeredPluginsMap;
		delete _registeredPluginsVec;
		delete _createdPlugins;
	}

protected:
	virtual bool Load(std::string& error, bool reloadPluginsFromStoredData = false) = 0;
	virtual void Unload(bool unloadDependantModules = true, bool storePluginsForReload = false) = 0;
	virtual bool Reload(std::function<void()> moduleUnloaded, std::string& error) = 0;
	
	virtual void NotifyUnloadToManager();
	virtual void NotifyUnloadToDependencies();

	virtual void OnDependantModuleUnloaded(Module* module);
	virtual void OnDependencyDestructed(Module* module);

public:
	virtual std::vector<std::string> GetGivenServices() const = 0;
	virtual std::vector<std::string> GetRegisteredPluginsForService(const std::string& serviceName) const = 0;

	virtual std::vector<std::string> GetRegisteredPlugins() const = 0;

	virtual Plugin* CreatePlugin(const std::string& pluginName, void* creationData = nullptr) = 0;
	virtual bool DestroyPlugin(Plugin* plugin) = 0;

	static Module* FromModuleInfo(ModuleInfo* moduleInfo);

public:
	std::string GetName() const { return _infos._name; }

	int GetVersionMajor() const { return _infos._versionMajor; }
	int GetVersionMinor() const { return _infos._versionMinor; }
	int GetVersionPatch() const { return _infos._versionPatch; }

protected:
	virtual bool Compile(std::string& error) = 0;
	virtual bool CheckDependencies(std::string& error);

private:
	/**
	* Chooses the correct binary info based on the system architecture, giving priority to the actual architecture.
	* If a binary for the current artchitecture isn't found then it looks for a binary marked as compatible with all architectures.
	*/
	void ChooseBinaryInfos();

protected:
	ModulesManager* _manager;		/** Modules manager that loaded this module */

	ModuleInfo _infos;				/** Informations about the module gotten from package.json */
	std::string _basePath;			/** Base path of the module (the same where its package.json is located) */

	ModuleBinaryInfo* _binaryInfo;	/** Binary informations used to load the module */

	std::vector<ModuleInfo*> _dependencies;	/** List of all the module's dependencies */
	std::vector<Module*> _dependantModules;	/** List of the modules that depend on this one */
	
	GivenServicesMap* _givenServices;				/** Map that contains all the services given from the module and for each service there is the list of plugins that give that service */
	RegisteredPluginsMap* _registeredPluginsMap;	/** Map that contains all the plugins registered from the module */
	RegisteredPluginsVector* _registeredPluginsVec;	/** List that contains all the plugins registered from the module */
	CreatedPluginsVector* _createdPlugins;			/** List of all the plugins created from this module */

	PluginDataToRestoreAfterReloadVector* _pluginsDataToRestoreAfterReload;	/** Data serialized by the plugins to be used when they get recreated after a module reload */
};

#endif //__Module_INCLUDE_H__