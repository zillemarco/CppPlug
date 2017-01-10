#ifndef __UnmanagedModule_INCLUDE_H__
#define __UnmanagedModule_INCLUDE_H__

#include <vector>
#include <map>
#include <memory>

#include "Module.h"

class DynamicLibrary;

class CppPlug_API UnmanagedModule : public Module
{
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
};

#endif //__UnmanagedModule_INCLUDE_H__