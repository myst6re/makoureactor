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
cmake_policy(SET CMP0091 NEW)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT /NODEFAULTLIB:MSVCRTD /NODEFAULTLIB:LIBCMTD /DEBUG:FULL /FORCE:MULTIPLE /IGNORE:4006,4075,4088,4099")

set(CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/vendor;${CMAKE_SOURCE_DIR}/cmake")

option(SUPERBUILD "Build the project using a superbuild" ON)

if (SUPERBUILD)
	project(SUPERBUILD)
	set_directory_properties(PROPERTIES EP_BASE "${CMAKE_BINARY_DIR}/ep")
	add_subdirectory(third_party)
	include(ExternalProject)
	ExternalProject_Add(
		Makou_Reactor
		SOURCE_DIR	"${CMAKE_SOURCE_DIR}"
		CMAKE_ARGS
			"-DSUPERBUILD=OFF"
			"-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
			"-DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/vendor"
			"-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}"
			"-DCMAKE_MSVC_RUNTIME_LIBRARY=${CMAKE_MSVC_RUNTIME_LIBRARY}"
		DEPENDS
	)
	ExternalProject_Add_Step(
		Makou_Reactor
		reconfigure
		COMMAND ${CMAKE_COMMAND} -E echo "Forcing a superbuild reconfigure"
		DEPENDEES download
		DEPENDERS configure
		ALWAYS ON
	)
	return()
endif()

project(Makou_Reactor)
add_subdirectory(src)
