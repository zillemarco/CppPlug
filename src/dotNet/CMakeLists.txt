project(CppPlug.Net C CXX)

if(WIN32)
    add_custom_target(CppPlug.Net ALL ${CMAKE_MAKE_PROGRAM} /p:Configuration=Release "${PROJECT_SOURCE_DIR}/../../CppPlugGen/build/CppPlugGen.sln")
endif()