set(CURRENT_TARGET "application")
set(CURRENT_TARGET_NAME "qelement")
set(CURRENT_TARGET_INTERFACE "${CURRENT_TARGET}_interface")

message(STATUS "Configuring ${CURRENT_TARGET}...")

# Qt
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5WebEngine REQUIRED)
find_package(Qt5WebEngineWidgets REQUIRED)

set(CONFIG_STATUS_QT "${Qt5Core_VERSION} (system)" CACHE INTERNAL "")

# Qt automoc
message(STATUS "Enabling Qt automoc...")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

CreateTarget(${CURRENT_TARGET} EXECUTABLE ${CURRENT_TARGET_NAME} 20)

# Qt deprecated warnings
target_compile_definitions(${CURRENT_TARGET} PRIVATE -DQT_DEPRECATED_WARNINGS)
target_compile_definitions(${CURRENT_TARGET} PRIVATE -DQT_DISABLE_DEPRECATED_BEFORE=0x060000)

# disable Qt foreach macro
target_compile_definitions(${CURRENT_TARGET} PRIVATE -DQT_NO_FOREACH)

target_link_libraries(${CURRENT_TARGET}
    PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::WebEngine
        Qt5::WebEngineWidgets
)

message(STATUS "Configured ${CURRENT_TARGET}.")
