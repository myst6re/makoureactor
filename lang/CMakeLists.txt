###############################################################################
## Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
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

set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

find_package(QT NAMES Qt5 Qt6 COMPONENTS LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS LinguistTools)

set(LANGS
    fr
    ja
CACHE INTERNAL "")
list(TRANSFORM LANGS REPLACE ".+" "${RELEASE_NAME}_\\0.ts" OUTPUT_VARIABLE TS_FILES)

qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src ${TS_FILES} OPTIONS -locations none)

add_custom_target(translations ALL DEPENDS ${QM_FILES})

if(APPLE)
    set(TRANSLATIONS_PATH ${PROJECT_NAME}.app/Contents/Resources CACHE INTERNAL "")
elseif(WIN32)
    set(TRANSLATIONS_PATH translations CACHE INTERNAL "")
else()
    set(TRANSLATIONS_PATH share/makoureactor/translations CACHE INTERNAL "")
endif()

install(FILES ${QM_FILES} DESTINATION "${TRANSLATIONS_PATH}")
