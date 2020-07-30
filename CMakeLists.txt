cmake_minimum_required(VERSION 3.14)

project(QElement)

message(STATUS "Building on: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")

set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo)

# Use project folders in IDE's
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Meta and Helper Modules
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}"
    "${PROJECT_SOURCE_DIR}/cmake/modules"
    "${PROJECT_SOURCE_DIR}/cmake/libs")

# Ensure we are building out of source
include(MacroEnsureOutOfSourceBuild)
EnsureOutOfSourceBuild()

# Target creation macro
include(CreateTarget)

# Folder where generated files go
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/Generated)
file(MAKE_DIRECTORY "${GENERATED_DIR}")
include_directories(${GENERATED_DIR})

# Handle configuration types
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Set the CMake configuration type to use")
message(STATUS "Using configuration type: ${CMAKE_BUILD_TYPE}")
set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})

# Position Independent Code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Shared Libraries Configuration
set(CMAKE_INSTALL_RPATH "${ORIGIN}")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

# Enable (strict) C++20 features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Release mode flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -DNDEBUG")

# Generic flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wno-unknown-pragmas -Wno-switch -Wno-unused-command-line-argument")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -Wno-unknown-pragmas -Wno-switch -Wno-unused-command-line-argument")

# Prevent implicit function declarations and force developers to include all the required header files
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=implicit-function-declaration")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=implicit-function-declaration")

# Debug mode flags
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    message(STATUS "Building with development configuration.")
    add_definitions(-DDEBUG_BUILD)

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        message(STATUS "LLVM/clang detected. Enabling LLDB debugger tuning...")
        add_definitions("-glldb")
    endif()
endif()

# Set target destination for built targets
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib-static)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(PkgConfig REQUIRED)

add_subdirectory(src)


# print configuration summary
message(STATUS "")
message(STATUS "============ Configuration Summary ============")

message(STATUS "Build Type:                ${CMAKE_BUILD_TYPE}")
message(STATUS "OS:                        ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")
message(STATUS "Compiler:                  ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

message(STATUS "Qt:                        ${CONFIG_STATUS_QT}")
message(STATUS "Notification System:       ${CONFIG_STATUS_NOTIFICATION_SYSTEM}")

message(STATUS "")
