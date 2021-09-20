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
cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0091 NEW)

set(RELEASE_NAME "Makou_Reactor")
set(GUI_TARGET "${RELEASE_NAME}")
#set(CLI_TARGET "makoureactor")

project(${RELEASE_NAME} VERSION 1.8.5 LANGUAGES CXX
    DESCRIPTION "Final Fantasy VII field archive editor"
    HOMEPAGE_URL "https://github.com/myst6re/makoureactor"
)

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

if(APPLE)
    set_target_properties(${GUI_TARGET} PROPERTIES OUTPUT_NAME "Makou Reactor")
    set(DEPLOY_TEMP "${CMAKE_BINARY_DIR}/Makou Reactor.app")
elseif (WIN32)
    set(DEPLOY_TEMP "${CMAKE_BINARY_DIR}/deploy")
    install(DIRECTORY DESTINATION "${DEPLOY_TEMP}")
    install(TARGETS ${GUI_TARGET} RUNTIME DESTINATION "${DEPLOY_TEMP}")
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
    install(FILES ${FF7TK_DEPENDS} DESTINATION "${DEPLOY_TEMP}")
else()
    install(TARGETS ${GUI_TARGET} RUNTIME)
endif()

if (NOT DEFINED QT_QMAKE_EXECUTABLE)
    get_target_property(QT_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
endif()
message("-- QT_QMAKE_EXECUTABLE: ${QT_QMAKE_EXECUTABLE}")
get_filename_component(_qt_bin_dir "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
get_filename_component(_qt_dir "${_qt_bin_dir}" DIRECTORY)
install(CODE "set(TARGET_FILE_DIR \"${DEPLOY_TEMP}\")")
install(CODE "set(_qt_bin_dir \"${_qt_bin_dir}\")")
install(SCRIPT deploy.cmake)

add_subdirectory(lang)

if(APPLE)
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX "/Applications" CACHE PATH "..." FORCE)
    endif()
    install(TARGETS ${GUI_TARGET} BUNDLE DESTINATION ".")
elseif (WIN32)
    install(DIRECTORY "${DEPLOY_TEMP}/" DESTINATION "./bin")
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${GUI_TARGET})
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(CPACK_SYSTEM_NAME "macos")
    set(CPACK_GENERATOR "DragNDrop")
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CPACK_GENERATOR "ZIP")
else()
    set(CPACK_GENERATOR "TXZ")
endif()

set(CPACK_SOURCE_IGNORE_FILES .git/* *.user *.yml *.github/*)
set(CPACK_SOURCE_GENERATOR "ZIP;TGZ")

include(CPack)
