cmake_minimum_required(VERSION 3.16)

project(qt-test VERSION 0.1 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(TargetArch)

target_architecture(ARCH)

if(ARCH STREQUAL "x86_64")
    set(ARCH "x64")
elseif(ARCH STREQUAL "i386")
    set(ARCH "x86")
elseif(ARCH STREQUAL "armv5" OR ARCH STREQUAL "armv6" OR ARCH STREQUAL "armv7")
    set(ARCH "arm")
endif()

if(MINGW)
    set(VCPKG_TARGET_TRIPLET "${ARCH}-mingw-dynamic")
elseif(WIN32)
    set(VCPKG_TARGET_TRIPLET "${ARCH}-windows")
elseif(APPLE)
    set(VCPKG_TARGET_TRIPLET "${ARCH}-osx")
elseif(UNIX)
    set(VCPKG_TARGET_TRIPLET "${ARCH}-linux")
endif()

if(DEFINED ENV{VCPKG_ROOT})
    file(TO_CMAKE_PATH "$ENV{VCPKG_ROOT}" CMAKE_VCPKG_ROOT)

    # include vcpkg is nice, we can have some determine logic
    include("${CMAKE_VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
else()
    message(FATAL_ERROR "Environment variable VCPKG_ROOT is not set")
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/install" CACHE PATH "install path" FORCE)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES qt-test_zh_CN.ts)

qt_add_big_resources(BIG_QRC_FILES qt-test.qrc)

message(STATUS "BIG_QRC_FILES: ${BIG_QRC_FILES}")

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    ${TS_FILES}
    ${BIG_QRC_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(qt-test
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )

    # Define target properties for Android with Qt 6 as:
    # set_property(TARGET qt-test APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    # ${CMAKE_CURRENT_SOURCE_DIR}/android)
    # For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(qt-test SHARED
            ${PROJECT_SOURCES}
        )

    # Define properties for Android with Qt 5 after find_package() calls as:
    # set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(qt-test
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_sources(qt-test PRIVATE ${QM_FILES})

target_link_libraries(qt-test PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

qt_add_resources(qt-test qrc_name
    PREFIX "i18n"
    FILES ${QM_FILES}
    BASE ${CMAKE_CURRENT_BINARY_DIR}
    QT_RESOURCE_ALIAS
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.qt-test)
endif()

set_target_properties(qt-test PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS qt-test
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(qt-test)
endif()

set(deploy_tool_options_arg "")

if(APPLE)
    set(deploy_tool_options_arg -hardened-runtime)
elseif(WIN32)
    set(deploy_tool_options_arg --no-compiler-runtime)
endif()

if(APPLE)
    set(executable_path "$<TARGET_FILE_NAME:qt-test>.app")
else()
    set(executable_path "\\\${QT_DEPLOY_BIN_DIR}/$<TARGET_FILE_NAME:qt-test>")
endif()

qt_generate_deploy_script(
    TARGET qt-test
    OUTPUT_SCRIPT deploy_script
    CONTENT "
qt_deploy_runtime_dependencies(
EXECUTABLE \"${executable_path}\"
DEPLOY_TOOL_OPTIONS ${deploy_tool_options_arg}
GENERATE_QT_CONF
VERBOSE
)")

install(SCRIPT ${deploy_script})

set(CPACK_GENERATOR "ZIP")

set(CPACK_PACKAGE_FILE_NAME "${CMAKE_SYSTEM_NAME}-${PROJECT_NAME}-${qt-test_VERSION}")

include(CPack)
