###############################################################################
## Copyright (C) 2009-2012 Arzel Jérôme <myst6re@gmail.com>
## Copyright (C) 2020 Julian Xhokaxhiu <https://julianxhokaxhiu.com>
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.
###############################################################################

cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0010 NEW)
cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0091 NEW)

set(RELEASE_NAME "Makou_Reactor")
set(GUI_TARGET "${RELEASE_NAME}")
#set(CLI_TARGET "makoureactor")

project("Makou Reactor" VERSION 1.8.5 LANGUAGES CXX
    DESCRIPTION "Final Fantasy VII field archive editor"
    HOMEPAGE_URL "https://github.com/myst6re/makoureactor"
)

message("${Qt5_DIR} - $ENV{Qt5_DIR}")

set(QT_AUTODETECT True)

# Auto-Detect Qt installation
if(NOT DEFINED Qt5_DIR AND NOT DEFINED ENV{Qt5_DIR})
    find_program(QT_QMAKE_EXECUTABLE "qmake")

    if(NOT QT_QMAKE_EXECUTABLE MATCHES "QT_QMAKE_EXECUTABLE-NOTFOUND")
        # ../../
        get_filename_component(QT_PATH "${QT_QMAKE_EXECUTABLE}" PATH)
        get_filename_component(QT_PATH "${QT_PATH}" PATH)
    else()
        if(WIN32)
            # look for user-registry pointing to qtcreator
            get_filename_component(QT_INSTALLATION_PATH [HKEY_CURRENT_USER\\Software\\Classes\\Applications\\QtProject.QtCreator.cpp\\shell\\Open\\Command] PATH)

            message("-- QtCreator Auto-Detected at ${QT_INSTALLATION_PATH}")

            # ../../../
            get_filename_component(QT_INSTALLATION_PATH "${QT_INSTALLATION_PATH}" PATH)
            get_filename_component(QT_INSTALLATION_PATH "${QT_INSTALLATION_PATH}" PATH)
            get_filename_component(QT_INSTALLATION_PATH "${QT_INSTALLATION_PATH}" PATH)
        endif()

        file(GLOB QT_VERSIONS "${QT_INSTALLATION_PATH}/5.*")
        list(SORT QT_VERSIONS)

        # assume the latest version will be last alphabetically
        list(REVERSE QT_VERSIONS)

        list(GET QT_VERSIONS 0 QT_VERSION)

        message("-- Last Qt version ${QT_VERSION}")

        if(MSVC)
            if(MSVC_TOOLSET_VERSION MATCHES 142)
                set(QT_MSVC "2019")
            elseif(MSVC_TOOLSET_VERSION MATCHES 141)
                set(QT_MSVC "2017")
            endif()

            set(QT_PATH "${QT_VERSION}/msvc${QT_MSVC}_64")
        elseif(MINGW)
            file(GLOB QT_COMPILERS "${QT_VERSION}/mingw*")
            list(GET QT_COMPILERS 0 QT_PATH)
        else()
            file(GLOB QT_COMPILERS "${QT_VERSION}/*")
            list(GET QT_COMPILERS 0 QT_PATH)
        endif()

        set(QT_QMAKE_EXECUTABLE "${QT_PATH}/bin/qmake")
    endif()

    set(Qt5_DIR "${QT_PATH}/lib/cmake/Qt5")

    message("-- Qt Auto-Detected at ${QT_PATH}")
    message("-- Qt5_DIR: ${Qt5_DIR}")
endif()

if(NOT DEFINED QT_DIR AND NOT DEFINED ENV{QT_DIR})
    set(QT_DIR "${Qt5_DIR}")
endif()

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x050F00)

find_package(QT NAMES Qt5 Qt6 COMPONENTS Widgets OpenGL REQUIRED)
find_package(QT NAMES Qt5 Qt6 COMPONENTS Zlib QUIET)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets OpenGL REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Zlib QUIET)
find_package(ff7tk COMPONENTS ff7tk ff7tkUtils REQUIRED)

if(NOT Qt${QT_VERSION_MAJOR}Zlib_FOUND)
    find_package(ZLIB REQUIRED)
    set(ZLIB_LIBRARY ZLIB::ZLIB)
else()
    set(ZLIB_LIBRARY Qt::Zlib)
endif()

file(GLOB_RECURSE PROJECT_SOURCES "src/*.cpp" "src/*.h")
set(RESOURCES "src/qt/${RELEASE_NAME}.qrc")

if(APPLE)
    set(ICON_FILE "deploy/macosx/${RELEASE_NAME}.icns")
    set(EXTRA_RESOURCES ${ICON_FILE})

    set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(MACOSX_BUNDLE_BUNDLE_NAME "Makou Reactor")
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER org.myst6re.makoureactor)
    set(MACOSX_BUNDLE_ICON_FILE ${RELEASE_NAME}.icns)
elseif (WIN32)
    set(ICON_FILE "src/qt/images/logo-shinra.ico")
    set(EXTRA_RESOURCES "src/qt/${RELEASE_NAME}.rc")
endif()

# GUI
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${GUI_TARGET} MANUAL_FINALIZATION MACOSX_BUNDLE WIN32 ${PROJECT_SOURCES} ${QM_FILES} ${RESOURCES} ${EXTRA_RESOURCES})
else()
    add_executable(${GUI_TARGET} MACOSX_BUNDLE WIN32 ${PROJECT_SOURCES} ${QM_FILES} ${RESOURCES} ${EXTRA_RESOURCES})
endif()
target_include_directories(${GUI_TARGET} PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(${GUI_TARGET} PRIVATE
    Qt::OpenGL
    Qt::Widgets
    ${ZLIB_LIBRARY}
    ff7tk::ff7tk
    ff7tk::ff7tkUtils
)

# CLI
#add_executable(${CLI_TARGET} ${PROJECT_SOURCES} ${QM_FILES} src/qt/${RELEASE_NAME}.qrc src/qt/${RELEASE_NAME}.rc)
#target_include_directories(${CLI_TARGET} PRIVATE "${CMAKE_SOURCE_DIR}/src")
#target_link_libraries(${CLI_TARGET}
#    Qt::Zlib
#    ff7tk::ff7tk
#    ff7tk::ff7tkUtils
#)
#target_compile_definitions(${CLI_TARGET}
#    PRIVATE MR_CONSOLE=1
#)

include(GNUInstallDirs)

if(APPLE OR WIN32)
    set_target_properties(${GUI_TARGET} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")
endif()

if(APPLE)
    set(DEPLOY_TEMP "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app")
elseif(WIN32)
    set(DEPLOY_TEMP "${CMAKE_BINARY_DIR}/win32-deploy")
    install(DIRECTORY DESTINATION "${DEPLOY_TEMP}")
    install(TARGETS ${GUI_TARGET} RUNTIME DESTINATION "${DEPLOY_TEMP}")
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.20.0")
        cmake_path(SET FF7TK_BIN_DIR "${ff7tk_DIR}/../../../bin")
    endif()
    if(MINGW)
        set(FF7TK_DEPENDS
            "${FF7TK_BIN_DIR}/libff7tk.dll"
            "${FF7TK_BIN_DIR}/libff7tkUtils.dll"
        )
    else()
        set(FF7TK_DEPENDS
            "${FF7TK_BIN_DIR}/ff7tk.dll"
            "${FF7TK_BIN_DIR}/ff7tkUtils.dll"
        )
    endif()
    install(FILES ${FF7TK_DEPENDS} DESTINATION "${DEPLOY_TEMP}")
else()
    set(DEPLOY_TEMP "${CMAKE_INSTALL_PREFIX}/share/applications/Makou_Reactor.desktop")
    install(TARGETS ${GUI_TARGET} RUNTIME)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/images/logo-shinra.png DESTINATION share/icons/hicolor/256x256/apps RENAME Makou_Reactor.png)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/deploy/linux/Makou_Reactor.desktop DESTINATION share/applications)
endif()

if(NOT DEFINED QT_QMAKE_EXECUTABLE OR QT_QMAKE_EXECUTABLE MATCHES "QT_QMAKE_EXECUTABLE-NOTFOUND")
    get_target_property(QT_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
endif()
message("-- QT_QMAKE_EXECUTABLE: ${QT_QMAKE_EXECUTABLE}")
get_filename_component(_qt_bin_dir "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
get_filename_component(_qt_dir "${_qt_bin_dir}" DIRECTORY)

add_subdirectory(deploy)
add_subdirectory(lang)

if(APPLE)
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX "/Applications" CACHE PATH "..." FORCE)
    endif()
    install(TARGETS ${GUI_TARGET} BUNDLE DESTINATION ".")
elseif(WIN32)
    install(DIRECTORY "${DEPLOY_TEMP}/" DESTINATION ".")
    install(CODE "file(REMOVE_RECURSE \"${DEPLOY_TEMP}\")")
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${GUI_TARGET})
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(CPACK_SYSTEM_NAME "macos")
    set(CPACK_GENERATOR "DragNDrop")
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CPACK_GENERATOR "ZIP")
else()
    set(CPACK_GENERATOR "TXZ")
endif()

set(CPACK_SOURCE_IGNORE_FILES .git/* *.user *.yml *.github/*)
set(CPACK_SOURCE_GENERATOR "ZIP;TGZ")

include(CPack)
