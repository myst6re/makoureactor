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

set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

find_package(QT NAMES Qt5 Qt6 COMPONENTS LinguistTools)
find_package(Qt${QT_VERSION_MAJOR}LinguistTools)

set(TS_FILES
    Makou_Reactor_fr.ts
    Makou_Reactor_ja.ts
)

qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

add_custom_target(translations ALL DEPENDS ${QM_FILES})

if(APPLE)
    set(QT_TRANSLATIONS_DIR "${_qt_dir}/translations")
    set(QT_QM_FILES
        "${QT_TRANSLATIONS_DIR}/qt_fr.qm"
        "${QT_TRANSLATIONS_DIR}/qt_ja.qm"
    )
    install(FILES ${QM_FILES} DESTINATION "$<TARGET_FILE_DIR:${GUI_TARGET}>/../Resources/")
    install(FILES ${QT_QM_FILES} DESTINATION "$<TARGET_FILE_DIR:${GUI_TARGET}>/../Resources/")
elseif(WIN32)
    install(FILES ${QM_FILES} DESTINATION "${DEPLOY_TEMP}")
endif()

