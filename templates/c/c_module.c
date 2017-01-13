#include "c_module.h"
#include "c_plugin.h"

#include <ModuleTools.h>
#include <stdio.h>

EXPORT_MODULE_EX(CMODULE_API, 
	REGISTER_PLUGIN_EX(CPlugin, "<plugin_service_name>", c_plugin_create, c_plugin_destroy, c_plugin_saveDataForReload, c_plugin_loadDataAfterReload, c_plugin_onMessage)
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