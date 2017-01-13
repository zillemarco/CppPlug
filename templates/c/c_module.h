#ifndef __C_MODULE_INCLUDE_H__
#define __C_MODULE_INCLUDE_H__

#if CppPlug_COMPILE_BRIDGE
    #define CMODULE_API
#else
    #ifdef WIN32
        #if CMODULE_EXPORT_SHARED
            #define CMODULE_API __declspec(dllexport)
        #else
            #define CMODULE_API __declspec(dllimport)
        #endif
    #else
        #define CMODULE_API
    #endif
#endif

#endif //__C_MODULE_INCLUDE_H__