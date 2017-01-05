#include "UnmanagedPlugin.h"

UnmanagedPlugin::UnmanagedPlugin(ModuleInfo* moduleInfo, PluginInfo* pluginInfo, void* creationData, void* pluginImpl)
	: _createdLocally(pluginImpl != nullptr)
	, _pluginImpl(pluginImpl)
	, _pluginInfos(pluginInfo)
{
	if(pluginImpl == nullptr)
		CreateImpl(moduleInfo->_reserved_dependencies, moduleInfo->_reserved_dependenciesCount, creationData);
}

UnmanagedPlugin::~UnmanagedPlugin()
{
	DestroyImpl();
}

void UnmanagedPlugin::CreateImpl(ModuleInfo* dependencies, int dependenciesCount, void* creationData)
{
	if (_pluginInfos)
		_pluginImpl = _pluginInfos->_createPluginFunc(dependencies, dependenciesCount, creationData);
}

void UnmanagedPlugin::DestroyImpl()
{
	if (_pluginImpl != nullptr && _pluginInfos)
	{
		_pluginInfos->_destroyPluginFunc(_pluginImpl);
		_pluginImpl = nullptr;
	}
}

bool UnmanagedPlugin::SendMessage(const std::string& messageName, void* messageData)
{
	if (_pluginImpl == nullptr || !_pluginInfos)
		return false;

	return _pluginInfos->_onMessagePluginFunc(_pluginImpl, messageName.c_str(), messageData);
}

const char* UnmanagedPlugin::SaveDataForReload()
{
	if (_pluginImpl == nullptr || !_pluginInfos)
		return false;

	return _pluginInfos->_saveDataForPluginReloadFunc(_pluginImpl);
}

void UnmanagedPlugin::LoadDataAfterReload(const char* savedData)
{
	if (_pluginImpl == nullptr || !_pluginInfos)
		return;

	_pluginInfos->_loadDataAfterPluginReloadFunc(_pluginImpl, savedData);
}