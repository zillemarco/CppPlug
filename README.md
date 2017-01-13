# CppPlug

![CppPlug](https://github.com/zillemarco/CppPlug/blob/master/logo/CppPlug.gif?raw=true)

CppPlug is a library that can be used to make a standard C++ project be able to be extended via plugins and modules that can be created in various languages like C, C++ and C#.

Mono is used to make the C++ communicate with C# plugins and this also gives the opportunity to the plugin developer to create its own plugin in other languages like Python (using IronPY), JavaScript (using IronJS) or any other language supported by Mono.

Managed modules, created using C#, are also able to be compiled at runtime (when they are loaded) using the Mono compiler.

Modules can depend on other modules with no restrictions (the module develeper can create a C++ module that depends on a C# module or vice versa) and hot reload of modules is supported.
