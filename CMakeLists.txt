cmake_minimum_required(VERSION 3.14)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")

include(ProjectSetup)

project(QElement CXX)

DetectAdditionalDebugOptions()

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
