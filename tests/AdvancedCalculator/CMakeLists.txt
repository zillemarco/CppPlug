project(AdvancedCalculator C CXX)

add_definitions(-DAdvancedCalculator_EXPORT_SHARED)

file(GLOB SRC "${PROJECT_SOURCE_DIR}/src/*.*")

include_directories("${PROJECT_SOURCE_DIR}/../../src")

add_library(${PROJECT_NAME} SHARED ${SRC})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Tests")

# Set the output path and properties
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY                   "${CMAKE_BINARY_DIR}/Build/CppPlug/lib/AdvancedCalculator" LIBRARY_OUTPUT_DIRECTORY                "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/AdvancedCalculator" RUNTIME_OUTPUT_DIRECTORY                "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/AdvancedCalculator")
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG             "${CMAKE_BINARY_DIR}/Build/CppPlug/lib/AdvancedCalculator" LIBRARY_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/AdvancedCalculator" RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/AdvancedCalculator")
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE           "${CMAKE_BINARY_DIR}/Build/CppPlug/lib/AdvancedCalculator" LIBRARY_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/AdvancedCalculator" RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/AdvancedCalculator")
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL        "${CMAKE_BINARY_DIR}/Build/CppPlug/lib/AdvancedCalculator" LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL     "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/AdvancedCalculator" RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/AdvancedCalculator")
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO    "${CMAKE_BINARY_DIR}/Build/CppPlug/lib/AdvancedCalculator" LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/AdvancedCalculator" RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/AdvancedCalculator")

set_target_properties(${PROJECT_NAME} PROPERTIES RELEASE_POSTFIX        "${CppPlug_RELEASE_SUFFIX}")
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX          "${CppPlug_DEBUG_SUFFIX}")
set_target_properties(${PROJECT_NAME} PROPERTIES MINSIZEREL_POSTFIX     "${CppPlug_MIN_SIZE_RELEASE_SUFFIX}")
set_target_properties(${PROJECT_NAME} PROPERTIES RELWITHDEBINFO_POSTFIX "${CppPlug_RELEASE_WITH_DEBUG_INFO_SUFFIX}")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/package.json" "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/AdvancedCalculator/package.json")

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