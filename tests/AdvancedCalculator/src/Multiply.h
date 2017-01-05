#ifndef __Multiply_INCLUDE_H__
#define __Multiply_INCLUDE_H__

#include "AdvancedCalculator.h"

#include <IModule.h>
#include <stdio.h>

class AdvancedCalculator_API Multiply
{
	DECLARE_PLUGIN_NO_CREATION_DATA(Multiply);

public:
	Multiply(ModuleInfo* deps, int depsCount)
		: _a(0.0f)
		, _b(0.0f)
	{
		ModuleInfo* standardCalculator = GetDependency(deps, depsCount, "StandardCalculator");
		_sumPlugin = CreatePlugin(standardCalculator, "Sum", nullptr);

		if (_sumPlugin != nullptr)
			printf("Sum plugin created!\n");
	}

	~Multiply()
	{
		if (_sumPlugin != nullptr)
			DestroyPlugin(_sumPlugin);
	}

public:
	bool OnMessage(const char* messageName, void* messageData)
	{
		if (strcmp(messageName, "setA") == 0)
		{
			_a = *((float*)messageData);
			return true;
		}
		else if (strcmp(messageName, "setB") == 0)
		{
			_b = *((float*)messageData);
			return true;
		}
		else if (strcmp(messageName, "getResult") == 0)
		{
			float* result = (float*)messageData;
			*result = 0;

			if (_b != 0.0f)
			{
				SendMessageToPlugin(_sumPlugin, "setB", &_a);

				float tmpResult = 0.0f;

				for (int i = 0; i < (int)_b; i++)
				{
					SendMessageToPlugin(_sumPlugin, "setA", &tmpResult);
					SendMessageToPlugin(_sumPlugin, "getResult", &tmpResult);
				}

				*result = tmpResult;
			}

			return true;
		}

		return false;
	}

	const char* SaveDataForReload() { return nullptr; }
	void LoadDataAfterReload(const char*) { }

private:
	CreatedPlugin* _sumPlugin;

	float _a;
	float _b;
};

#endif //__Multiply_INCLUDE_H__