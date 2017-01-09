#ifndef __ModulesManager_INCLUDE_H__
#define __ModulesManager_INCLUDE_H__

#include "CppPlugDefs.h"
#include "Module.h"

#include <vector>
#include <map>
#include <memory>

class Path;

class CppPlug_API ModulesManager
{
	friend Module;
	friend class UnmanagedModule;

	#ifdef SUPPORT_MANAGED
		friend class ManagedModule;
	#endif	

	typedef std::map<std::string, std::shared_ptr<Module> > ModulesMap;
	
private:
	ModulesManager();

public:
	~ModulesManager();

public:
	static ModulesManager& GetInstance();

	std::shared_ptr<Module> GetModule(const std::string& name);

	std::shared_ptr<Module> LoadModule(const std::string& path, std::string* error = nullptr);

	bool ReloadModule(std::shared_ptr<Module> module, std::function<void()> moduleUnloaded = nullptr, std::string* error = nullptr);

private:
	void ReadModuleJSON(const Path& path, ModuleInfo& moduleInfos);

	void OnModuleLoaded(Module* module);
	void OnModuleUnloaded(Module* module);

private:
	ModulesMap* _loadedModules;
};

#endif //__ModulesManager_INCLUDE_H__