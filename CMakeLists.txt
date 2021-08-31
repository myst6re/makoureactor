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
cmake_policy(SET CMP0074 NEW)

project(Makou_Reactor LANGUAGES CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(RELEASE_NAME "Makou_Reactor")
set(RELEASE_VERSION "1.8.2")
set(GUI_TARGET "${RELEASE_NAME}")

find_package(Qt5 COMPONENTS Widgets OpenGL LinguistTools REQUIRED)
find_package(ff7tk REQUIRED NO_MODULE COMPONENTS ff7tk ff7tkUtils)
find_package(ZLIB REQUIRED)

file(GLOB_RECURSE PROJECT_SOURCES "*.cpp" "*.h")

# Translations
file(GLOB_RECURSE translation_files "${RELEASE_NAME}_*.ts")
set_source_files_properties(${translation_files} PROPERTIES OUTPUT_LOCATION "${CMAKE_INSTALL_FULL_BINDIR}")
qt5_add_translation(QM_FILES ${translation_files})

if(WIN32)
    set (FF7TK_DEPENDS
        ${ff7tk_DIR}/../../../bin/ff7tk.dll
        ${ff7tk_DIR}/../../../bin/ff7tkWidgets.dll
    )
endif()

# GUI
add_executable(${GUI_TARGET} ${PROJECT_SOURCES} ${QM_FILES} src/qt/${RELEASE_NAME}.qrc src/qt/${RELEASE_NAME}.rc)
target_include_directories(${GUI_TARGET} PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(${GUI_TARGET}
    Qt5::OpenGL
    Qt5::Widgets
    ff7tk::ff7tk
    ff7tk::ff7tkUtils
)
