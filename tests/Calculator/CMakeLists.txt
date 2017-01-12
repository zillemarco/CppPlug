project(Calculator C CXX)

file(GLOB SRC "${PROJECT_SOURCE_DIR}/src/*.*")

include_directories("${PROJECT_SOURCE_DIR}/../../src")

add_executable(${PROJECT_NAME} ${SRC})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Tests")

target_link_libraries(${PROJECT_NAME} CppPlug)

# Set the output path and properties
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG             "${CMAKE_BINARY_DIR}/Build/CppPlug/lib/Debug"           LIBRARY_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/Debug"           RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/Debug")
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE           "${CMAKE_BINARY_DIR}/Build/CppPlug/lib/Release"         LIBRARY_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/Release"         RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/Release")
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL        "${CMAKE_BINARY_DIR}/Build/CppPlug/lib/MinSizeRel"      LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL     "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/MinSizeRel"      RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/MinSizeRel")
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO    "${CMAKE_BINARY_DIR}/Build/CppPlug/lib/RelWithDebInfo"  LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/RelWithDebInfo"  RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/Build/CppPlug/bin/RelWithDebInfo")

set_target_properties(${PROJECT_NAME} PROPERTIES RELEASE_POSTFIX        "")
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX          "")
set_target_properties(${PROJECT_NAME} PROPERTIES MINSIZEREL_POSTFIX     "")
set_target_properties(${PROJECT_NAME} PROPERTIES RELWITHDEBINFO_POSTFIX "")