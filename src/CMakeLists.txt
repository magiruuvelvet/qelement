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
find_package(Qt5Network REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5WebEngine REQUIRED)
find_package(Qt5WebEngineWidgets REQUIRED)
find_package(Qt5LinguistTools)

set(CONFIG_STATUS_QT "${Qt5Core_VERSION} (system)" CACHE INTERNAL "")

# Qt automoc
message(STATUS "Enabling Qt automoc...")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# embedded assets
qt5_add_resources(RCC_SOURCES "${PROJECT_SOURCE_DIR}/assets/assets.qrc")
add_custom_target(GenerateEmbeddedAssets DEPENDS ${RCC_SOURCES})

# ui translations
set(BUILD_TRANSLATIONS OFF CACHE BOOL "Build translations")
if (BUILD_TRANSLATIONS AND Qt5LinguistTools_FOUND)
    message(STATUS "Building of translations activated.")

    # qt lrelease binary
    get_target_property(qt5_lrelease_bin Qt5::lrelease IMPORTED_LOCATION)

    # scan available translation
    file(GLOB_RECURSE UI_TRANSLATION_FILES
        "${PROJECT_SOURCE_DIR}/i18n/*.ts"
    )

    # print some interesting variables
    message(STATUS "Compiled translations will be placed in: ${CMAKE_CURRENT_BINARY_DIR}/i18n")
    message(STATUS "lrelease location: ${qt5_lrelease_bin}")

    # copy qrc file
    configure_file("${PROJECT_SOURCE_DIR}/i18n/translations.qrc" "${CMAKE_CURRENT_BINARY_DIR}/i18n/translations.qrc" COPYONLY)

    # only compile ts files without updating them
    # translation files are updated manually outside of CMake as required
    # also there is a bug with Qt's translation macro which deletes all ts files on clean
    foreach (TS_FILE ${UI_TRANSLATION_FILES})
        get_filename_component(QM_OUTPUT_FILE "${TS_FILE}" NAME_WE)
        message(STATUS "Compiling translation ${QM_OUTPUT_FILE}...")
        execute_process(
            COMMAND "${qt5_lrelease_bin}" "${TS_FILE}" -silent -qm "${CMAKE_CURRENT_BINARY_DIR}/i18n/${QM_OUTPUT_FILE}.qm"
            WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/i18n"
        )
    endforeach()

    qt5_add_resources(RCC_TRANSLATION_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/i18n/translations.qrc")
    add_custom_target(GenerateEmbeddedTranslations DEPENDS ${RCC_TRANSLATION_SOURCES})

    set(CONFIG_STATUS_TRANSLATIONS "enabled" CACHE INTERNAL "")

elseif (BUILD_TRANSLATIONS)
    message(WARNING "Building of translations requested but no Qt linguist tools found.")
    message(WARNING "Translations will not be built.")
    set(RCC_TRANSLATION_SOURCES "")
    add_custom_target(GenerateEmbeddedTranslations)
    set(CONFIG_STATUS_TRANSLATIONS "disabled automatically (missing tools)" CACHE INTERNAL "")
else()
    set(RCC_TRANSLATION_SOURCES "")
    add_custom_target(GenerateEmbeddedTranslations)
    set(CONFIG_STATUS_TRANSLATIONS "disabled" CACHE INTERNAL "")
endif()

# create target after qt setup
CreateTarget(${CURRENT_TARGET} EXECUTABLE ${CURRENT_TARGET_NAME} 20)

# append assets to target sources
add_dependencies(${CURRENT_TARGET} GenerateEmbeddedAssets GenerateEmbeddedTranslations)
set_property(TARGET ${CURRENT_TARGET} APPEND PROPERTY SOURCES ${RCC_SOURCES} ${RCC_TRANSLATION_SOURCES})

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
        Qt5::Network
        Qt5::Concurrent
        Qt5::WebEngine
        Qt5::WebEngineWidgets
        Threads::Threads
)

# libnotify
if (LIBNOTIFY_FOUND AND ENABLE_LIBNOTIFY)
    target_compile_definitions(${CURRENT_TARGET} PRIVATE -DLIBNOTIFY_ENABLED)
    target_include_directories(${CURRENT_TARGET} SYSTEM PRIVATE "${LIBNOTIFY_INCLUDE_DIRS}")
    target_link_libraries(${CURRENT_TARGET} PRIVATE "${LIBNOTIFY_LDFLAGS}")
endif()

install(TARGETS ${CURRENT_TARGET} RUNTIME DESTINATION bin)
install(FILES "${PROJECT_SOURCE_DIR}/assets/qelement.desktop" DESTINATION share/applications)

message(STATUS "Configured ${CURRENT_TARGET}.")
