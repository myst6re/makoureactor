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

find_package(QT NAMES Qt5 COMPONENTS LinguistTools)
find_package(Qt${QT_VERSION_MAJOR}LinguistTools)

file(GLOB_RECURSE TS_FILES "${RELEASE_NAME}_*.ts")

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
