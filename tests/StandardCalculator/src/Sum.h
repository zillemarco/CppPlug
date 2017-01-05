#ifndef __Sum_INCLUDE_H__
#define __Sum_INCLUDE_H__

#include "StandardCalculator.h"
#include <IModule.h>

class StandardCalculator_API Sum
{
	DECLARE_PLUGIN_NO_CREATION_DATA_NO_DEPENDENCIES(Sum);

public:
	Sum() : _a(0.0f), _b(0.0f) { }
	~Sum() { }

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
			*((float*)messageData) = _a + _b;
			return true;
		}

		return false;
	}

	const char* SaveDataForReload() { return nullptr; }
	void LoadDataAfterReload(const char*) { }

private:
	float _a;
	float _b;
};

#endif //__Sum_INCLUDE_H__