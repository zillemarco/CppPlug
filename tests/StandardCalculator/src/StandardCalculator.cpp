#include "StandardCalculator.h"

#include "Sum.h"

#include <ModuleTools.h>
#include <stdio.h>

EXPORT_MODULE_EX(StandardCalculator_API, 
	REGISTER_PLUGIN(Sum, StandardCalculator)
)

int UnloadModule()
{
	printf("Unloading module: StandardCalculator\n");
	return 0;
}

int ReloadModule()
{
	printf("Reloading module: StandardCalculator\n");
	return 0;
}

struct LoadModuleResult LoadModule()
{
	printf("Loading module: StandardCalculator\n");

	struct LoadModuleResult result;
	result._reloadModuleFunc = ReloadModule;
	result._unloadModuleFunc = UnloadModule;

	return result;
}