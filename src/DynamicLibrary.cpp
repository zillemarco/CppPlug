#include "DynamicLibrary.h"

#if defined WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

#include <sstream>
#include <iostream>

DynamicLibrary::DynamicLibrary(void* handle)
    : _handle(handle)
{ }

DynamicLibrary::~DynamicLibrary()
{
    if (_handle)
    {
        #ifndef WIN32
            ::dlclose(_handle);
        #else
            ::FreeLibrary((HMODULE)_handle);
        #endif
    }
}

DynamicLibrary* DynamicLibrary::Load(const std::string& name, std::string& errorString)
{
    if (name.empty()) 
    {
        errorString = "Cannot load the binary";
        return nullptr;
    }
  
    void* handle = nullptr;

    #ifdef WIN32
        handle = ::LoadLibraryA(name.c_str());
        if (handle == nullptr)
        {
            DWORD errorCode = ::GetLastError();
            std::stringstream ss;
            ss  << std::string("Cannot load the binary: LoadLibrary(") << name << std::string(") failed. errorCode: ") << errorCode; 
            errorString = ss.str();
        }
    #else
        handle = ::dlopen(name.c_str(), RTLD_NOW);
        if (!handle) 
        {
            std::string dlErrorString;
            const char *zErrorString = ::dlerror();
            
            if (zErrorString)
                dlErrorString = zErrorString;
            
            errorString += "Cannot load the binary: failed to load \"" + name + '"';

            if(dlErrorString.size())
                errorString += ": " + dlErrorString;

            return nullptr;
        }
    #endif
  
    return new DynamicLibrary(handle);
}

void* DynamicLibrary::GetSymbol(const std::string& symbol)
{
    if (!_handle)
        return nullptr;
  
    #ifdef WIN32
        return ::GetProcAddress((HMODULE)_handle, symbol.c_str());
    #else
        return ::dlsym(_handle, symbol.c_str());
    #endif
}