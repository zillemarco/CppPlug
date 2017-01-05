#ifndef __StandardCalculator_INCLUDE_H__
#define __StandardCalculator_INCLUDE_H__

#ifdef WIN32
	#if StandardCalculator_EXPORT_SHARED
		#define StandardCalculator_API __declspec(dllexport)
	#else
		#define StandardCalculator_API __declspec(dllimport)
	#endif
#else
	#define StandardCalculator_API
#endif

#endif //__StandardCalculator_INCLUDE_H__