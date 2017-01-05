#include "AdvancedCalculator.h"

#include "Multiply.h"

#include <IModule.h>
#include <stdio.h>

EXPORT_MODULE_EX(AdvancedCalculator_API,
	REGISTER_PLUGIN(Multiply, AdvancedCalculator)
)

int UnloadModule()
{
	printf("Unloading module: AdvancedCalculator\n");
	return 0;
}

int ReloadModule()
{
	printf("Reloading module: AdvancedCalculator\n");
	return 0;
}

struct LoadModuleResult LoadModule()
{
	printf("Loading module: AdvancedCalculator\n");

	struct LoadModuleResult result;
	result._reloadModuleFunc = ReloadModule;
	result._unloadModuleFunc = UnloadModule;

	return result;
}