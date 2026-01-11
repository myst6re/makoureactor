###############################################################################
## Copyright (C) 2009-2024 Arzel Jérôme <myst6re@gmail.com>
## Copyright (C) 2023 Julian Xhokaxhiu <https://julianxhokaxhiu.com>
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

if(NOT DEFINED QT_VERSION_TO_FIND)
    SET(QT_VERSION_TO_FIND 6)
endif()

find_package(QT NAMES Qt6 QUIET)

# Auto-Detect Qt installation
if(NOT QT_FOUND)
    find_program(QT_QMAKE_EXECUTABLE "qmake")

    if(NOT QT_QMAKE_EXECUTABLE MATCHES "QT_QMAKE_EXECUTABLE-NOTFOUND")
        # ../../
        get_filename_component(QT_PATH "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
        get_filename_component(QT_PATH "${QT_PATH}" DIRECTORY)

        message("-- Found qmake in PATH: ${QT_QMAKE_EXECUTABLE}")
    else()
        if(WIN32)
            # look for user-registry pointing to Qt installation
            cmake_host_system_information(RESULT QT_INSTALLATION_PATH QUERY WINDOWS_REGISTRY "HKEY_CURRENT_USER/Software/Microsoft/Windows/CurrentVersion/Uninstall/{1f0b01f1-fb8c-49bb-8410-ef0628043911}" VALUE "InstallLocation")

            if(QT_INSTALLATION_PATH STREQUAL "")
                # look for user-registry pointing to qtcreator
                cmake_host_system_information(RESULT QT_INSTALLATION_PATH QUERY WINDOWS_REGISTRY "HKEY_CURRENT_USER/Software/Classes/Applications/QtProject.QtCreator.qml/shell/Open/Command")
                string(REPLACE " -client \"%1\"" "" QT_INSTALLATION_PATH ${QT_INSTALLATION_PATH})

                if(QT_INSTALLATION_PATH STREQUAL "")
                    message(FATAL_ERROR "Please set QT_INSTALLATION_PATH or Qt${QT_VERSION_TO_FIND}_DIR")
                endif()

                message("-- QtCreator Auto-Detected at ${QT_INSTALLATION_PATH}")

                # ../../../../
                get_filename_component(QT_INSTALLATION_PATH "${QT_INSTALLATION_PATH}" DIRECTORY)
                get_filename_component(QT_INSTALLATION_PATH "${QT_INSTALLATION_PATH}" DIRECTORY)
                get_filename_component(QT_INSTALLATION_PATH "${QT_INSTALLATION_PATH}" DIRECTORY)
                get_filename_component(QT_INSTALLATION_PATH "${QT_INSTALLATION_PATH}" DIRECTORY)
            endif()
        elseif(NOT QT_INSTALLATION_PATH)
            message(FATAL_ERROR "Please set QT_INSTALLATION_PATH or Qt${QT_VERSION_TO_FIND}_DIR")
        endif()

        file(GLOB QT_VERSIONS "${QT_INSTALLATION_PATH}/${QT_VERSION_TO_FIND}.*")
        list(SORT QT_VERSIONS COMPARE NATURAL)

        # assume the latest version will be last alphabetically
        list(REVERSE QT_VERSIONS)

        list(GET QT_VERSIONS 0 QT_VERSION)

        message("-- Last Qt version ${QT_VERSION}")

        if(MSVC)
            if(MSVC_TOOLSET_VERSION MATCHES 143)
                set(QT_MSVC "2022")
            elseif(MSVC_TOOLSET_VERSION MATCHES 142)
                set(QT_MSVC "2019")
            elseif(MSVC_TOOLSET_VERSION MATCHES 141)
                set(QT_MSVC "2017")
            endif()

            set(QT_PATH "${QT_VERSION}/msvc${QT_MSVC}_64")
        elseif(MINGW)
            file(GLOB QT_COMPILERS "${QT_VERSION}/mingw*")
            list(SORT QT_COMPILERS)
            list(REVERSE QT_COMPILERS)
            list(GET QT_COMPILERS 0 QT_PATH)
        else()
            # For some reason GLOB also list files on Mac OSX
            file(GLOB QT_COMPILERS "${QT_VERSION}/*/lib")
            list(SORT QT_COMPILERS)
            list(REVERSE QT_COMPILERS)
            list(GET QT_COMPILERS 0 QT_PATH)
            # ../
            get_filename_component(QT_PATH "${QT_PATH}" DIRECTORY)
        endif()

        set(QT_QMAKE_EXECUTABLE "${QT_PATH}/bin/qmake")
    endif()

    set(Qt${QT_VERSION_TO_FIND}_DIR "${QT_PATH}/lib/cmake/Qt${QT_VERSION_TO_FIND}")
    set(QT_DIR "${Qt${QT_VERSION_TO_FIND}_DIR}")

    message("-- Qt${QT_VERSION_TO_FIND}_DIR: ${Qt${QT_VERSION_TO_FIND}_DIR}")

    find_package(QT NAMES Qt${QT_VERSION_TO_FIND} REQUIRED)

    message("-- Qt Auto-Detected at ${QT_PATH}")
endif()

if(NOT DEFINED QT_QMAKE_EXECUTABLE OR QT_QMAKE_EXECUTABLE MATCHES "QT_QMAKE_EXECUTABLE-NOTFOUND")
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
    get_target_property(QT_QMAKE_EXECUTABLE Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
endif()
message("-- QT_QMAKE_EXECUTABLE: ${QT_QMAKE_EXECUTABLE}")
get_filename_component(_qt_bin_dir "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
get_filename_component(_qt_dir "${_qt_bin_dir}" DIRECTORY)
set(_qt_translations_dir "${_qt_dir}/translations")

# Add the QT_PATH to the resolution paths
list(APPEND CMAKE_PREFIX_PATH "${QT_PATH}")

set(QT_PATH "${QT_PATH}" CACHE PATH "Path to QT directory")
