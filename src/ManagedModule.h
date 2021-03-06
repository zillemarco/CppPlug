#ifdef SUPPORT_MANAGED

#ifndef __ManagedModule_INCLUDE_H__
#define __ManagedModule_INCLUDE_H__

#include <vector>
#include <map>
#include <memory>

#include "Module.h"

class DynamicLibrary;
struct ManagedModuleInternalData;
struct MonoInitilizationData;

class CppPlug_API ManagedModule : public Module
{
	friend class ModulesManager;

public:
	ManagedModule(const ModuleInfo& moduleInfo, const std::string& basePath);

	virtual ~ManagedModule();

protected:
	bool Load(std::string& error, bool reloadPluginsFromStoredData = false) override;
	void Unload(bool unloadDependantModules = true, bool storePluginsForReload = false) override;
	bool Reload(std::function<void()> moduleUnloaded, std::string& error) override;

protected:
	static bool InitializeMono(const std::string& domainName, const std::string& monoAssemblyDir, const std::string& monoConfigDir, std::string& error);
	static void ShutDownMono();

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
	ManagedModuleInternalData* _internalData; /** Internal data used by the managed module */
	
	static MonoInitilizationData* s_monoInitilizationData;	/** Data got from Mono initialization */
	static bool s_monoInitialized;							/** Used to know if Mono has been initialized correctly */
};

#endif //__ManagedModule_INCLUDE_H__

#endif