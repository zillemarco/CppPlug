#ifndef __UnmanagedPlugin_INCLUDE_H__
#define __UnmanagedPlugin_INCLUDE_H__

#include <vector>
#include <map>
#include <memory>

#include "Plugin.h"

class CppPlug_API UnmanagedPlugin : public Plugin
{
	friend class UnmanagedModule;

protected:
	UnmanagedPlugin(ModuleInfo* moduleInfo, PluginInfo* pluginInfo, void* creationData, void* pluginImpl = nullptr);

public:
	virtual ~UnmanagedPlugin();

protected:
	void CreateImpl(ModuleInfo* dependencies, int dependenciesCount, void* creationData) override;
	void DestroyImpl() override;

public:
	bool SendMessage(const std::string& messageName, void* messageData = nullptr) override;

protected:
	const char* SaveDataForReload() override;
	void LoadDataAfterReload(const char* savedData) override;

private:
	bool _createdLocally;	/** Used to know if _pluginImpl was created from here or if it was given */
	void* _pluginImpl;		/** Pointer to the actual plugin implementation object */

	PluginInfo* _pluginInfos;	/** Pointer to the unmanaged plugin info data */
};

#endif //__UnmanagedPlugin_INCLUDE_H__