#ifndef __DynamicLibrary_INCLUDE_H__
#define __DynamicLibrary_INCLUDE_H__

#include <string>

class DynamicLibrary
{
public:
  static DynamicLibrary* Load(const std::string& path, std::string& errorString);
  ~DynamicLibrary();
  
  void* GetSymbol(const std::string& name);

private:
  DynamicLibrary();
  
  DynamicLibrary(void* handle);
  DynamicLibrary(const DynamicLibrary& src);
  
private:
  void* _handle;  
};

#endif //__DynamicLibrary_INCLUDE_H__