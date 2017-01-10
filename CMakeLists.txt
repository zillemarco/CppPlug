cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 11)

project(CppPlug C CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CppPlug_VERSION_MAJOR "0")
set(CppPlug_VERSION_MINOR "1")
set(CppPlug_VERSION_PATCH "0")
set(CppPlug_VERSION "${CppPlug_VERSION_MAJOR}.${CppPlug_VERSION_MINOR}.${CppPlug_VERSION_PATCH}")

option(MANAGED_PLUGINS_SUPPORT  "If enabled managed plugins will be supported using Mono"    ON)
option(CREATE_INSTALL           "Generate installation target"                               OFF)
option(BUILD_TESTS              "Builds the tests"                                           ON)

set(CppPlug_RELEASE_SUFFIX                    ""      CACHE STRING "Suffix for release builds")
set(CppPlug_DEBUG_SUFFIX                      "_d"    CACHE STRING "Suffix for debug builds")
set(CppPlug_RELEASE_WITH_DEBUG_INFO_SUFFIX    "_rd"   CACHE STRING "Suffix for release with debug info builds")
set(CppPlug_MIN_SIZE_RELEASE_SUFFIX           "_msr"  CACHE STRING "Suffix for min size release builds")

add_definitions(-DCppPlug_EXPORT_SHARED)

if(MANAGED_PLUGINS_SUPPORT)
set(MONO_ROOT "" CACHE STRING "Path where to find the Mono installation")

if(MONO_ROOT STREQUAL "")
    message(FATAL_ERROR "Provide a valid root path for Mono")
endif()

add_definitions(-DSUPPORT_MANAGED)
include_directories("${MONO_ROOT}/include/mono-2.0")

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
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY                   "${CMAKE_BINARY_DIR}/Build/CppPlug/lib" LIBRARY_OUTPUT_DIRECTORY                "${CMAKE_BINARY_DIR}/Build/CppPlug/bin" RUNTIME_OUTPUT_DIRECTORY                "${CMAKE_BINARY_DIR}/Build/CppPlug/bin")
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG             "${CMAKE_BINARY_DIR}/Build/CppPlug/lib" LIBRARY_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/Build/CppPlug/bin" RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/Build/CppPlug/bin")
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE           "${CMAKE_BINARY_DIR}/Build/CppPlug/lib" LIBRARY_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/Build/CppPlug/bin" RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/Build/CppPlug/bin")
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL        "${CMAKE_BINARY_DIR}/Build/CppPlug/lib" LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL     "${CMAKE_BINARY_DIR}/Build/CppPlug/bin" RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     "${CMAKE_BINARY_DIR}/Build/CppPlug/bin")
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO    "${CMAKE_BINARY_DIR}/Build/CppPlug/lib" LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/Build/CppPlug/bin" RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/Build/CppPlug/bin")

set_target_properties(${PROJECT_NAME} PROPERTIES RELEASE_POSTFIX        "${CppPlug_RELEASE_SUFFIX}")
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX          "${CppPlug_DEBUG_SUFFIX}")
set_target_properties(${PROJECT_NAME} PROPERTIES MINSIZEREL_POSTFIX     "${CppPlug_MIN_SIZE_RELEASE_SUFFIX}")
set_target_properties(${PROJECT_NAME} PROPERTIES RELWITHDEBINFO_POSTFIX "${CppPlug_RELEASE_WITH_DEBUG_INFO_SUFFIX}")

# Include version information in the output
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${CppPlug_VERSION})

if(MANAGED_PLUGINS_SUPPORT)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "${MONO_RUNTIME_DLL}" "${CMAKE_BINARY_DIR}/Build/CppPlug/bin")
endif()

if (WIN32)
    # The CppPlug DLL needs a special compile-time macro and import library name
    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" IMPORT_PREFIX "")
    set_target_properties(${PROJECT_NAME} PROPERTIES IMPORT_SUFFIX "_dll.lib")
elseif (APPLE)
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

if(BUILD_TESTS)
    add_subdirectory(tests/Calculator)
    add_subdirectory(tests/StandardCalculator)
    add_subdirectory(tests/AdvancedCalculator)
endif()

if(CREATE_INSTALL)
    
    if(MANAGED_PLUGINS_SUPPORT)
        install(FILES "${MONO_RUNTIME_DLL}" DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
    endif()

    install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/" DESTINATION "include" PATTERN "*.cpp" EXCLUDE)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "bin/"
									LIBRARY DESTINATION "bin/"
									ARCHIVE DESTINATION "lib/")
endif()



# -h="D:\Progetti\Git\CppPlug\src\IModule.h" --inc="D:\Progetti\Git\CppPlug\src" -d="CppPlug_COMPILE_MANAGED_BRIDGE" --iln="CppPlug_d" --isln="CppPlug_d.dll" --od="D:\Progetti\Git\CppPlug" --on="CppPlug"