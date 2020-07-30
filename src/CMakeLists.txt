set(CURRENT_TARGET "application")
set(CURRENT_TARGET_NAME "qelement")
set(CURRENT_TARGET_INTERFACE "${CURRENT_TARGET}_interface")

message(STATUS "Configuring ${CURRENT_TARGET}...")

find_package(Threads REQUIRED)

set(ENABLE_LIBNOTIFY OFF CACHE BOOL "Use libnotify for notifications instead of Qt.")
pkg_check_modules(LIBNOTIFY "libnotify")
if (LIBNOTIFY_FOUND AND ENABLE_LIBNOTIFY)
    message(STATUS "Enabling libnotify support...")
    set(CONFIG_STATUS_NOTIFICATION_SYSTEM "libnotify" CACHE INTERNAL "")
else()
    set(CONFIG_STATUS_NOTIFICATION_SYSTEM "Qt" CACHE INTERNAL "")
endif()

# Qt
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5WebEngine REQUIRED)
find_package(Qt5WebEngineWidgets REQUIRED)

set(CONFIG_STATUS_QT "${Qt5Core_VERSION} (system)" CACHE INTERNAL "")

# Qt automoc
message(STATUS "Enabling Qt automoc...")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# embedded assets
qt5_add_resources(RCC_SOURCES "${PROJECT_SOURCE_DIR}/assets/assets.qrc")
add_custom_target(GenerateEmbeddedAssets DEPENDS ${RCC_SOURCES})

# create target after qt setup
CreateTarget(${CURRENT_TARGET} EXECUTABLE ${CURRENT_TARGET_NAME} 20)

# append assets to target sources
add_dependencies(${CURRENT_TARGET} GenerateEmbeddedAssets)
set_property(TARGET ${CURRENT_TARGET} APPEND PROPERTY SOURCES ${RCC_SOURCES})

# Qt deprecated warnings
target_compile_definitions(${CURRENT_TARGET} PRIVATE -DQT_DEPRECATED_WARNINGS)
target_compile_definitions(${CURRENT_TARGET} PRIVATE -DQT_DISABLE_DEPRECATED_BEFORE=0x060000)

# disable Qt foreach macro
target_compile_definitions(${CURRENT_TARGET} PRIVATE -DQT_NO_FOREACH)

target_include_directories(${CURRENT_TARGET} PRIVATE "${PROJECT_SOURCE_DIR}/libs/cpp-httplib")

target_link_libraries(${CURRENT_TARGET}
    PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Concurrent
        Qt5::WebEngine
        Qt5::WebEngineWidgets
        Threads::Threads
)

# libnotify
if (LIBNOTIFY_FOUND AND ENABLE_LIBNOTIFY)
    target_compile_definitions(${CURRENT_TARGET} PRIVATE -DLIBNOTIFY_ENABLED)
    target_include_directories(${CURRENT_TARGET} PRIVATE SYSTEM "${LIBNOTIFY_INCLUDE_DIRS}")
    target_link_libraries(${CURRENT_TARGET} PRIVATE "${LIBNOTIFY_LDFLAGS}")
endif()

message(STATUS "Configured ${CURRENT_TARGET}.")
