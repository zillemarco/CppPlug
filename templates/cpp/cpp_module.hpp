#ifndef __CPP_MODULE_INCLUDE_H__
#define __CPP_MODULE_INCLUDE_H__

#if CppPlug_COMPILE_BRIDGE
    #define CMODULE_API
#else
    #ifdef WIN32
        #if CPPMODULE_EXPORT_SHARED
            #define CPPMODULE_API __declspec(dllexport)
        #else
            #define CPPMODULE_API __declspec(dllimport)
        #endif
    #else
        #define CPPMODULE_API
    #endif
#endif

#endif //__CPP_MODULE_INCLUDE_H__