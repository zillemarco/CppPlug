#ifndef __CPLUGIN_INCLUDE_H__
#define __CPLUGIN_INCLUDE_H__

#include <ModuleTools.h>
#include <stdio.h>

struct c_plugin_data {};

static void* c_plugin_create(ModuleInfo* dependencies, int dependenciesCount, void* data)
{
    struct c_plugin_data* plugin = malloc(sizeof(struct c_plugin_data));
    memset(plugin, 0, sizeof(struct c_plugin_data));
    return plugin;
}

static void c_plugin_destroy(void* plugin)
{
    free(plugin);
}

static const char* c_plugin_saveDataForReload(void* plugin)
{
    struct c_plugin_data* p = (struct c_plugin_data*)plugin;
    
    // Serialize data for the plugin to a file and return its path

    return nullptr;
}

static void c_plugin_loadDataAfterReload(void* plugin, const char* data)
{
    struct c_plugin_data* p = (struct c_plugin_data*)plugin;
    
    // Load the data previously serialized from the path given before
}

static bool c_plugin_onMessage(void* plugin, const char* messageName, void* messageData)
{
    struct c_plugin_data* p = (struct c_plugin_data*)plugin;

    // Do something to handle the message

    // Return true to tell the caller that the message has been successfully handled
    return true;
}

#endif //__CPLUGIN_INCLUDE_H__