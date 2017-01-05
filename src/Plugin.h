#ifndef __Plugin_INCLUDE_H__
#define __Plugin_INCLUDE_H__

#include "CppPlugDefs.h"
#include "IModule.h"

#include <string>

struct PluginDataToRestoreAfterReload
{
	CreatedPlugin* _referencePlugin;
	std::string _storedData;
	std::string _pluginName;
};

class CppPlug_API Plugin
{
	friend class Module;
	friend class UnmanagedModule;

protected:
	Plugin() { }

public:
	virtual ~Plugin() { }

protected:
	virtual void CreateImpl(ModuleInfo* dependencies, int dependenciesCount, void* creationData) = 0;
	virtual void DestroyImpl() = 0;

public:
	virtual bool SendMessage(const std::string& messageName, void* messageData = nullptr) = 0;

protected:
	virtual const char* SaveDataForReload() = 0;
	virtual void LoadDataAfterReload(const char* savedData) = 0;
};

#endif //__Plugin_INCLUDE_H__