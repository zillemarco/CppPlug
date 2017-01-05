#ifndef __AdvancedCalculator_INCLUDE_H__
#define __AdvancedCalculator_INCLUDE_H__

#ifdef WIN32
#if AdvancedCalculator_EXPORT_SHARED
#define AdvancedCalculator_API __declspec(dllexport)
#else
#define AdvancedCalculator_API __declspec(dllimport)
#endif
#else
#define AdvancedCalculator_API
#endif

#endif //__AdvancedCalculator_INCLUDE_H__