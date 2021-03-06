#ifndef __CppPlugDefs_INCLUDE_H__
#define __CppPlugDefs_INCLUDE_H__

#if CppPlug_COMPILE_MANAGED_BRIDGE
	#define CppPlug_API
#else
	#if WIN32
		#if CppPlug_EXPORT_SHARED
			#define CppPlug_API __declspec(dllexport)
		#else
			#define CppPlug_API __declspec(dllimport)
		#endif
	#else
		#define CppPlug_API
	#endif
#endif

#ifndef _C_EXPORT_
	#ifdef __cplusplus
		#define _C_EXPORT_ "C"
	#else
		#define _C_EXPORT_
	#endif
#endif

#include <cstdint>

#if SIZE_MAX == UINT32_MAX
#define CppPlug_ENVIRONMENT_32_BIT	1
#define CppPlug_ENVIRONMENT_64_BIT	0
#elif SIZE_MAX == UINT64_MAX
#define CppPlug_ENVIRONMENT_32_BIT	0
#define CppPlug_ENVIRONMENT_64_BIT	1
#else
	#error "The environment is not 32 bit and not 64 bit either"
#endif

#if SUPPORT_MANAGED
	#if WIN32
		#define MONO_DEFAULT_ASSEMBLY_DIR	"C:/Program Files (x86)/Mono/lib"
		#define MONO_DEFAULT_CONFIG_DIR		"C:/Program Files (x86)/Mono/etc"
	#endif

	#define MONO_DEFAULT_DOMAIN_NAME "CppPlug"
#endif

#endif //__CppPlugDefs_INCLUDE_H__