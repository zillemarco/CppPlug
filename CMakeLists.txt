cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 11)

project(CppPlug C CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Check target architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CppPlug_64_BIT 1)
else()
    set(CppPlug_64_BIT 0)
endif()

set(CppPlug_VERSION_MAJOR "0")
set(CppPlug_VERSION_MINOR "1")
set(CppPlug_VERSION_PATCH "0")
set(CppPlug_VERSION "${CppPlug_VERSION_MAJOR}.${CppPlug_VERSION_MINOR}.${CppPlug_VERSION_PATCH}")

option(MANAGED_PLUGINS_SUPPORT  "If enabled managed plugins will be supported using Mono"    ON)
option(CREATE_INSTALL           "Generate installation target"                               OFF)
option(BUILD_TESTS              "Builds the tests"                                           ON)

add_definitions(-DCppPlug_EXPORT_SHARED)

if(MANAGED_PLUGINS_SUPPORT)
set(MONO_ROOT "" CACHE STRING "Path where to find the Mono installation")

if(MONO_ROOT STREQUAL "")
    message(FATAL_ERROR "Provide a valid root path for Mono")
endif()

add_definitions(-DSUPPORT_MANAGED)
include_directories("${MONO_ROOT}/include/mono-2.0")

set(MONO_COMPILER_DIR "${MONO_ROOT}/bin")
endif()

file(GLOB SRC "${PROJECT_SOURCE_DIR}/src/*.*")

add_library(${PROJECT_NAME} SHARED ${SRC})

if(WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_DEFINITIONS_RELEASE _CRT_SECURE_NO_WARNINGS)
    set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_DEFINITIONS_DEBUG _CRT_SECURE_NO_WARNINGS)
endif()

if(MANAGED_PLUGINS_SUPPORT)

set(MONO_RUNTIME_DLL "${MONO_ROOT}/bin/monosgen-2.0.dll")
target_link_libraries(${PROJECT_NAME} "${MONO_ROOT}/lib/monosgen-2.0.lib")

endif()

# Set the output path and properties
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG             "${CMAKE_BINARY_DIR}/Build/CppPlug/lib/Debug"           LIBRARY_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/Debug"           RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/Debug")
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE           "${CMAKE_BINARY_DIR}/Build/CppPlug/lib/Release"         LIBRARY_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/Release"         RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/Release")
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL        "${CMAKE_BINARY_DIR}/Build/CppPlug/lib/MinSizeRel"      LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL     "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/MinSizeRel"      RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/MinSizeRel")
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO    "${CMAKE_BINARY_DIR}/Build/CppPlug/lib/RelWithDebInfo"  LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/RelWithDebInfo"  RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/RelWithDebInfo")

set_target_properties(${PROJECT_NAME} PROPERTIES RELEASE_POSTFIX        "")
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX          "")
set_target_properties(${PROJECT_NAME} PROPERTIES MINSIZEREL_POSTFIX     "")
set_target_properties(${PROJECT_NAME} PROPERTIES RELWITHDEBINFO_POSTFIX "")

# Include version information in the output
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${CppPlug_VERSION})

if(MANAGED_PLUGINS_SUPPORT)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "${MONO_RUNTIME_DLL}" "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:RelWithDebInfo>:RelWithDebInfo>$<$<CONFIG:MinSizeRel>:MinSizeRel>")
endif()

if (APPLE)
    # Append -fno-common to the compile flags to work around a bug in
    # Apple's GCC
    get_target_property(CppPlug_CFLAGS ${PROJECT_NAME} COMPILE_FLAGS)
    
    if (NOT CppPlug_CFLAGS)
        set(CppPlug_CFLAGS "")
    endif()
    
    set_target_properties(${PROJECT_NAME} PROPERTIES
                        COMPILE_FLAGS "${CppPlug_CFLAGS} -fno-common"
                        INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/CppPlug/lib${LIB_SUFFIX}")

    set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${CppPlug_VERSION})
endif()

if(MANAGED_PLUGINS_SUPPORT)
    add_subdirectory(src/dotNet)

    add_dependencies(${PROJECT_NAME} "CppPlug.Net")

    if(WIN32)
        if(NOT EXISTS "${PROJECT_SOURCE_DIR}/CppPlugGen/build/CppPlugGen.sln")
            message(FATAL_ERROR "CppPlugGen project wans't found. Generate the project using the script inside the CppPlugGen folder")
        endif()
        
        if(CppPlug_64_BIT)
        else()
            set(CppPlug_Net_Bindings "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/")
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND "${PROJECT_SOURCE_DIR}/CppPlugGen/build/lib/CppPlugGen.exe" "${MONO_COMPILER_DIR}" "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:RelWithDebInfo>:RelWithDebInfo>$<$<CONFIG:MinSizeRel>:MinSizeRel>/" WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
        endif()
    endif()
endif()

if(BUILD_TESTS)
    add_subdirectory(tests/Calculator)
    add_subdirectory(tests/StandardCalculator)
    add_subdirectory(tests/AdvancedCalculator)
    
    if(MANAGED_PLUGINS_SUPPORT)
        add_subdirectory(tests/ManagedCalculator)
    endif()
endif()

if(CREATE_INSTALL)
    
    if(MANAGED_PLUGINS_SUPPORT)
        install(FILES "${MONO_RUNTIME_DLL}" DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
    endif()

    install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/" DESTINATION "include" PATTERN "*.cpp" EXCLUDE PATTERN "dotNet" EXCLUDE)

    if(MANAGED_PLUGINS_SUPPORT)
        install(FILES "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:RelWithDebInfo>:RelWithDebInfo>$<$<CONFIG:MinSizeRel>:MinSizeRel>/CppPlug.Net.dll" DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
    endif()

    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "bin/"
									LIBRARY DESTINATION "bin/"
									ARCHIVE DESTINATION "lib/")
endif()