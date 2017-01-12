project(ManagedCalculator C CXX)

set(ManagedCalculator_Source_Path "${PROJECT_SOURCE_DIR}/")
set(ManagedCalculator_Output_Path "${CMAKE_BINARY_DIR}/Build/CppPlug/")

configure_file("${PROJECT_SOURCE_DIR}/ManagedCalculator.csproj.template" "${CMAKE_BINARY_DIR}/ManagedCalculator/ManagedCalculator.csproj")

include_external_msproject(ManagedCalculator "${CMAKE_BINARY_DIR}/ManagedCalculator/ManagedCalculator.csproj" TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Tests")

add_dependencies(${PROJECT_NAME} "CppPlug")