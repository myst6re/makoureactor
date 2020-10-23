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

# Autodetect the current Qt MSVC toolkit installed
# Original source: https://stackoverflow.com/a/47723189 + custom patch to reflect current VS year based on the toolkit
IF(DEFINED $ENV{_CMAKE_QT_DIR})
    SET(QT_MISSING False)
ELSE()
    SET(QT_MISSING True)
ENDIF()

# use Qt_DIR approach so you can find Qt after cmake has been invoked
IF(NOT QT_MISSING)
    SET(QT_PATH $ENV{_CMAKE_QT_DIR})
ELSE()
    IF(MSVC)
        # look for user-registry pointing to qtcreator
        GET_FILENAME_COMPONENT(QT_BIN [HKEY_CURRENT_USER\\Software\\Classes\\Applications\\QtProject.QtCreator.cpp\\shell\\Open\\Command] PATH)

        # get root path so we can search for 5.3, 5.4, 5.5, etc
        STRING(REPLACE "/Tools" ";" QT_BIN "${QT_BIN}")
        LIST(GET QT_BIN 0 QT_BIN)
        FILE(GLOB QT_VERSIONS "${QT_BIN}/5.*")
        LIST(SORT QT_VERSIONS)

        # assume the latest version will be last alphabetically
        LIST(REVERSE QT_VERSIONS)

        LIST(GET QT_VERSIONS 0 QT_VERSION)

        # fix any double slashes which seem to be common
        STRING(REPLACE "//" "/"  QT_VERSION "${QT_VERSION}")

        IF(MSVC_TOOLSET_VERSION MATCHES 142)
            SET(QT_MSVC "2019")
        ELSEIF(MSVC_TOOLSET_VERSION MATCHES 141)
            SET(QT_MSVC "2017")
        ENDIF()

        # check for 64-bit os
        # may need to be removed for older compilers as it wasn't always offered
        IF(CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE MATCHES x64)
            SET(QT_MSVC "${QT_MSVC}_64")
        ENDIF()
        SET(QT_PATH "${QT_VERSION}/msvc${QT_MSVC}")
    ENDIF()
ENDIF()

MESSAGE("-- Qt found: ${QT_PATH}")
SET(Qt5_DIR "${QT_PATH}/lib/cmake/Qt5/")
SET(Qt5Test_DIR "${QT_PATH}/lib/cmake/Qt5Test")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(OpenGL REQUIRED)
find_package(Qt5 COMPONENTS Widgets Zlib LinguistTools REQUIRED)

set(RELEASE_NAME "Makou_Reactor")
set(GUI_TARGET "${RELEASE_NAME}")
set(CLI_TARGET "cli")

file(GLOB_RECURSE source_files "*.cpp")

# Translations
file(GLOB_RECURSE translation_files "${RELEASE_NAME}_*.ts")
set_source_files_properties(${translation_files} PROPERTIES OUTPUT_LOCATION "${CMAKE_INSTALL_FULL_BINDIR}")
qt5_add_translation(QM_FILES ${translation_files})

# GUI
add_executable(${GUI_TARGET} WIN32 ${source_files} ${QM_FILES} qt/${RELEASE_NAME}.qrc qt/${RELEASE_NAME}.rc)
target_include_directories(${GUI_TARGET} PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_compile_options(${GUI_TARGET}
    PRIVATE /D_CRT_SECURE_NO_WARNINGS
    PRIVATE /DNOMINMAX
    PRIVATE /EHsc
    PRIVATE /std:c++latest
    PRIVATE /w
    PRIVATE /Ox
)
target_link_libraries(${GUI_TARGET}
    OpenGL::GL
    OpenGL::GLU
    Qt5::Widgets
    Qt5::Zlib
)
target_link_options(${GUI_TARGET}
    PRIVATE /PDBALTPATH:${RELEASE_NAME}.pdb
)

# CLI
add_executable(${CLI_TARGET} ${source_files} ${QM_FILES} qt/${RELEASE_NAME}.rc)
target_include_directories(${CLI_TARGET} PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_compile_options(${CLI_TARGET}
    PRIVATE /DMR_CONSOLE
    PRIVATE /D_CRT_SECURE_NO_WARNINGS
    PRIVATE /DNOMINMAX
    PRIVATE /EHsc
    PRIVATE /std:c++latest
    PRIVATE /w
    PRIVATE /Ox
)
target_link_libraries(${CLI_TARGET}
    OpenGL::GL
    OpenGL::GLU
    Qt5::Widgets
    Qt5::Zlib
)
target_link_options(${CLI_TARGET}
    PRIVATE /PDBALTPATH:${RELEASE_NAME}.pdb
)

include(GNUInstallDirs)

# Retrieve the absolute path to qmake and then use that path to find the binaries
get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(LCONVERT_EXECUTABLE lconvert HINTS "${_qt_bin_dir}")
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")

# Executable
install(TARGETS ${GUI_TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
# CLI
install(TARGETS ${CLI_TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
# Translations
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_BINDIR})
# License
install(FILES ${CMAKE_SOURCE_DIR}/COPYING.TXT DESTINATION ${CMAKE_INSTALL_BINDIR})
# PDB ( Debug mode only )
install(FILES $<TARGET_PDB_FILE:${GUI_TARGET}> DESTINATION ${CMAKE_INSTALL_BINDIR} OPTIONAL)
# Deploy Qt
if(WIN32)
    add_custom_command(TARGET ${GUI_TARGET} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E env PATH="${_qt_bin_dir}"
            "${WINDEPLOYQT_EXECUTABLE} --force --release --dir \"${CMAKE_INSTALL_FULL_BINDIR}\" --no-quick-import --no-translations --no-webkit2 --no-angle --no-svg --no-webkit --no-opengl-sw $<TARGET_FILE:${GUI_TARGET}>"
        COMMENT "Running windeployqt..."
    )
endif()
