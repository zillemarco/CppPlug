#ifdef SUPPORT_MANAGED

#include "ManagedPlugin.h"
#include "ModuleTools.hpp"

ManagedPlugin::ManagedPlugin(ModuleInfo* moduleInfo, PluginInfo* pluginInfo, void* creationData, void* pluginImpl)
	: _createdLocally(pluginImpl != nullptr)
	, _pluginImpl(pluginImpl)
	, _pluginInfos(pluginInfo)
{
	if (pluginImpl == nullptr)
		CreateImpl(moduleInfo->_reserved_dependencies, moduleInfo->_reserved_dependenciesCount, creationData);
}

ManagedPlugin::~ManagedPlugin()
{
	DestroyImpl();
}

void ManagedPlugin::CreateImpl(ModuleInfo* dependencies, int dependenciesCount, void* creationData)
{
	if (_pluginInfos && _pluginInfos->_reserved_cppCreatePluginFunc != nullptr)
	{
		std::vector<CModuleInfo> dependenciesVector;

		if (dependencies != nullptr && dependenciesCount > 0)
		{
			dependenciesVector.reserve(dependenciesCount);
			for (int i = 0; i < dependenciesCount; i++)
				dependenciesVector.push_back(&dependencies[i]);
		}
		
		CModuleDependenciesCollection dependenciesCollection(dependenciesVector);

		__cpp_createPluginFunc func = (__cpp_createPluginFunc)_pluginInfos->_reserved_cppCreatePluginFunc;
		_pluginImpl = func(&dependenciesCollection, creationData);
	}
}

void ManagedPlugin::DestroyImpl()
{
	if (_pluginImpl != nullptr && _pluginInfos)
	{
		_pluginInfos->_destroyPluginFunc(_pluginImpl);
		_pluginImpl = nullptr;
	}
}

bool ManagedPlugin::SendMessage(const std::string& messageName, void* messageData)
{
	if (_pluginImpl == nullptr || !_pluginInfos)
		return false;

	return _pluginInfos->_onMessagePluginFunc(_pluginImpl, messageName.c_str(), messageData);
}

const char* ManagedPlugin::SaveDataForReload()
{
	if (_pluginImpl == nullptr || !_pluginInfos)
		return false;

	return _pluginInfos->_saveDataForPluginReloadFunc(_pluginImpl);
}

void ManagedPlugin::LoadDataAfterReload(const char* savedData)
{
	if (_pluginImpl == nullptr || !_pluginInfos)
		return;

	_pluginInfos->_loadDataAfterPluginReloadFunc(_pluginImpl, savedData);
}

#endif