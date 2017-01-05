#ifndef __UnmanagedModule_INCLUDE_H__
#define __UnmanagedModule_INCLUDE_H__

#include <vector>
#include <map>
#include <memory>

#include "Module.h"

class DynamicLibrary;
struct PluginDataToRestoreAfterReload;

class CppPlug_API UnmanagedModule : public Module
{
	typedef std::map<std::string, std::vector<PluginInfo*> > GivenServicesMap;
	typedef std::map<std::string, PluginInfo*> RegisteredPluginsMap;
	typedef std::vector<PluginInfo*> RegisteredPluginsVector;
	typedef std::vector<CreatedPlugin*> CreatedPluginsVector;
	typedef std::vector<PluginDataToRestoreAfterReload*> PluginDataToRestoreAfterReloadVector;

public:
	UnmanagedModule(const ModuleInfo& moduleInfo, const std::string& basePath);

	virtual ~UnmanagedModule();

protected:
	bool Load(std::string& error, bool reloadPluginsFromStoredData = false) override;
	void Unload(bool unloadDependantModules = true, bool storePluginsForReload = false) override;
	bool Reload(std::function<void()> moduleUnloaded, std::string& error) override;

public:
	std::vector<std::string> GetGivenServices() const override;
	std::vector<std::string> GetRegisteredPluginsForService(const std::string& serviceName) const override;

	std::vector<std::string> GetRegisteredPlugins() const override;

	Plugin* CreatePlugin(const std::string& pluginName, void* creationData = nullptr) override;
	bool DestroyPlugin(Plugin* plugin) override;

protected:
	bool Compile(std::string& error) override;

private:
	static CreatedPlugin* CreatePlugin(void* module, const char* pluginName, void* creationData);
	static bool DestroyPlugin(void* moduleInfo, CreatedPlugin* plugin);
	static bool SendMessageToPlugin(CreatedPlugin* plugin, const char* messageName, void* messageData);
		
private:
	__loadModuleFunc _loadModuleFunc;			/** Pointer to the function that gets called to load the module */
	__unloadModuleFunc _unloadModuleFunc;		/** Pointer to the function that gets called to unload the module */
	__reloadModuleFunc _reloadModuleFunc;		/** Pointer to the function that gets called to reload the module */

	DynamicLibrary* _moduleBinary;				/** Binary file that contains the module */

	GivenServicesMap* _givenServices;				/** Map that contains all the services given from the module and for each service there is the list of plugins that give that service */
	RegisteredPluginsMap* _registeredPluginsMap;	/** Map that contains all the plugins registered from the module */
	RegisteredPluginsVector* _registeredPluginsVec;	/** List that contains all the plugins registered from the module */
	CreatedPluginsVector* _createdPlugins;			/** List of all the plugins created from this module */
	
	PluginDataToRestoreAfterReloadVector* _pluginsDataToRestoreAfterReload;	/** Data serialized by the plugins to be used when they get recreated after a module reload */
};

#endif //__UnmanagedModule_INCLUDE_H__