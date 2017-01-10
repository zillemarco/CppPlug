#include <ModulesManager.h>
#include <Plugin.h>
#include <Path.h>

int main(int argc, char* argv[])
{
	std::string cwd = Path(argv[0]).GetParent();

	Path standardCalculatorPath = Path(cwd) + Path("StandardCalculator");
	Path advancedCalculatorPath = Path(cwd) + Path("AdvancedCalculator");
	Path managedCalculatorPath = Path(cwd) + Path("ManagedCalculator");

	ModulesManager::GetInstance().Initialize();

	std::string errors = "";
	const std::shared_ptr<Module>& moduleStandard = ModulesManager::GetInstance().LoadModule(standardCalculatorPath.GetPath(), &errors);
	const std::shared_ptr<Module>& moduleManaged = ModulesManager::GetInstance().LoadModule(managedCalculatorPath.GetPath(), &errors);
	const std::shared_ptr<Module>& moduleAdvanced = ModulesManager::GetInstance().LoadModule(advancedCalculatorPath.GetPath(), &errors);
	
	float a = 5.0f;
	float b = 2.0f;
	float result = 0.0f;

	if (moduleAdvanced)
	{
		Plugin* multiplier = moduleAdvanced->CreatePlugin("Multiply", nullptr);
	
		multiplier->SendMessage("setA", &a);
		multiplier->SendMessage("setB", &b);
		multiplier->SendMessage("getResult", &result);

		printf("\nResult: %.0f\n\n", result);
		
		ModulesManager::GetInstance().ReloadModule(moduleStandard, []() {

			printf("Press a key to continue...");
			getchar();
		});
		
		multiplier->SendMessage("setA", &a);
		multiplier->SendMessage("setB", &b);
		multiplier->SendMessage("getResult", &result);

		printf("\nResult: %.0f", result);
	}
	else
	{
		printf("%s", errors.c_str());
	}
	
	printf("\nDone");
	getchar();

	return 0;
}