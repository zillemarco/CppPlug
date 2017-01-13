#include "cpp_module.hpp"
#include "cpp_plugin.hpp"

#include <ModuleTools.h>
#include <stdio.h>

EXPORT_MODULE_EX(CPPMODULE_API, 
	REGISTER_PLUGIN(CPPPlugin, "<plugin_service_name>")
)

int UnloadModule()
{
	return 0;
}

int ReloadModule()
{
	return 0;
}

struct LoadModuleResult LoadModule()
{
	struct LoadModuleResult result;
	result._reloadModuleFunc = ReloadModule;
	result._unloadModuleFunc = UnloadModule;

	return result;
}