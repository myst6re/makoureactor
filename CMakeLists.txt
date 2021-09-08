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

set(RELEASE_NAME "Makou_Reactor")
set(GUI_TARGET "${RELEASE_NAME}")
#set(CLI_TARGET "makoureactor")

project(${RELEASE_NAME} VERSION 1.8.5 LANGUAGES CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x050E00)

find_package(QT NAMES Qt5 Qt6 COMPONENTS Widgets OpenGL Zlib REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets OpenGL Zlib REQUIRED)
find_package(ff7tk COMPONENTS ff7tk ff7tkUtils REQUIRED)

file(GLOB_RECURSE PROJECT_SOURCES "src/*.cpp" "src/*.h")
set(RESOURCES "src/qt/${RELEASE_NAME}.qrc")

if (APPLE)
    set(OS_BUNDLE MACOSX_BUNDLE)
    set(ICON_FILE "src/qt/images/${RELEASE_NAME}.icns")
    set(EXTRA_RESOURCES ${ICON_FILE})

    set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(MACOSX_BUNDLE_BUNDLE_NAME "Makou Reactor")
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER org.myst6re.makoureactor)
    set(MACOSX_BUNDLE_ICON_FILE ${RELEASE_NAME}.icns)
elseif (WIN32)
    set(OS_BUNDLE WIN32)
    set(ICON_FILE "src/qt/images/logo-shinra.ico")
    set(EXTRA_RESOURCES "src/qt/${RELEASE_NAME}.rc")
endif()

# GUI
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${GUI_TARGET} MANUAL_FINALIZATION ${OS_BUNDLE} ${PROJECT_SOURCES} ${QM_FILES} ${RESOURCES} ${EXTRA_RESOURCES})
else()
    add_executable(${GUI_TARGET} ${OS_BUNDLE} ${PROJECT_SOURCES} ${QM_FILES} ${RESOURCES} ${EXTRA_RESOURCES})
endif()
target_include_directories(${GUI_TARGET} PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(${GUI_TARGET} PRIVATE
    Qt::OpenGL
    Qt::Widgets
    Qt::Zlib
    ff7tk::ff7tk
    ff7tk::ff7tkUtils
)

if (APPLE)
    set_target_properties(${GUI_TARGET} PROPERTIES OUTPUT_NAME "Makou Reactor")
endif()

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

message("-- CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")

if(APPLE)
    set(INSTALL_DESTINATION "/Applications")
    install(TARGETS ${GUI_TARGET} BUNDLE DESTINATION "${INSTALL_DESTINATION}")
elseif (WIN32)
    set(INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
    install(TARGETS ${GUI_TARGET} RUNTIME DESTINATION "${INSTALL_DESTINATION}")
    if(MINGW)
        set(FF7TK_DEPENDS
            ${ff7tk_DIR}/../../../bin/libff7tk.dll
            ${ff7tk_DIR}/../../../bin/libff7tkUtils.dll
        )
    else()
        set(FF7TK_DEPENDS
            ${ff7tk_DIR}/../../../bin/ff7tk.dll
            ${ff7tk_DIR}/../../../bin/ff7tkUtils.dll
        )
    endif()
    install(FILES ${FF7TK_DEPENDS} DESTINATION "${INSTALL_DESTINATION}")
else()
    install(TARGETS ${GUI_TARGET} RUNTIME)
endif()

get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
install(CODE "set(TARGET_FILE_DIR \"${INSTALL_DESTINATION}\")")
install(CODE "set(_qt_bin_dir \"${_qt_bin_dir}\")")
install(SCRIPT deploy.cmake)

add_subdirectory(lang)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${GUI_TARGET})
endif()
