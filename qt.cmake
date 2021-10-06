###############################################################################
## Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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

if(NOT DEFINED QT_VERSION_TO_FIND)
    SET(QT_VERSION_TO_FIND 5)
endif()

find_package(QT NAMES Qt5 Qt6 QUIET)

# Auto-Detect Qt installation
if(NOT QT_FOUND)
    find_program(QT_QMAKE_EXECUTABLE "qmake")

    if(NOT QT_QMAKE_EXECUTABLE MATCHES "QT_QMAKE_EXECUTABLE-NOTFOUND")
        # ../../
        get_filename_component(QT_PATH "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
        get_filename_component(QT_PATH "${QT_PATH}" DIRECTORY)
    else()
        if(WIN32)
            # look for user-registry pointing to Qt installation
            get_filename_component(QT_INSTALLATION_PATH [HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{89c4e30e-3a1a-47c7-80d7-013ba3e25e57};InstallLocation] ABSOLUTE CACHE)

            if(QT_INSTALLATION_PATH STREQUAL "/")
                # look for user-registry pointing to qtcreator
                get_filename_component(QT_INSTALLATION_PATH [HKEY_CURRENT_USER\\Software\\Classes\\Applications\\QtProject.QtCreator.cpp\\shell\\Open\\Command] DIRECTORY CACHE)

                if(QT_INSTALLATION_PATH STREQUAL "/")
                    message(FATAL_ERROR "Please set QT_INSTALLATION_PATH or Qt5_DIR")
                endif()

                message("-- QtCreator Auto-Detected at ${QT_INSTALLATION_PATH}")

                # ../../../
                get_filename_component(QT_INSTALLATION_PATH "${QT_INSTALLATION_PATH}" DIRECTORY)
                get_filename_component(QT_INSTALLATION_PATH "${QT_INSTALLATION_PATH}" DIRECTORY)
                get_filename_component(QT_INSTALLATION_PATH "${QT_INSTALLATION_PATH}" DIRECTORY)
            endif()
        elseif(NOT QT_INSTALLATION_PATH)
            message(FATAL_ERROR "Please set QT_INSTALLATION_PATH or Qt5_DIR")
        endif()

        file(GLOB QT_VERSIONS "${QT_INSTALLATION_PATH}/${QT_VERSION_TO_FIND}.*")
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

    message("-- Qt Auto-Detected at ${QT_PATH}")
    message("-- Qt${QT_VERSION_TO_FIND}_DIR: ${Qt${QT_VERSION_TO_FIND}_DIR}")
endif()
