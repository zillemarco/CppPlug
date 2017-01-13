#ifndef __CPPPLUGIN_INCLUDE_H__
#define __CPPPLUGIN_INCLUDE_H__

#include <ModuleTools.h>
#include <stdio.h>

class CPPMODULE_API CPPPlugin
{
	DECLARE_PLUGIN_NO_CREATION_DATA_NO_DEPENDENCIES(CPPPlugin);

public:
	CPPPlugin() { }
	~CPPPlugin() { }

public:
	bool OnMessage(const char* messageName, void* messageData)
	{
        // Do something to handle the message

        // Return true to tell the caller that the message has been successfully handled
		return true;
	}

	const char* SaveDataForReload()
    {
        // Serialize data for the plugin to a file and return its path
        return nullptr;
    }

	void LoadDataAfterReload(const char* data)
    {
        // Load the data previously serialized from the path given before
    }
};

#endif //__CPPPLUGIN_INCLUDE_H__