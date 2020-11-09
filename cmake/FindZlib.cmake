include(FindPackageHandleStandardArgs)

if (NOT ZLIB_FOUND)
	find_library(
		ZLIB_LIBRARY
		zlibd zlib z
		PATH_SUFFIXES
		lib
	)

	find_path(
		ZLIB_INCLUDE_DIR
		zlib.h
		PATH_SUFFIXES
		include
	)

	add_library(Zlib::Zlib STATIC IMPORTED)

	set_target_properties(
		Zlib::Zlib
		PROPERTIES
		IMPORTED_LOCATION
		"${ZLIB_LIBRARY}"
		INTERFACE_INCLUDE_DIRECTORIES
		"${ZLIB_INCLUDE_DIR}"
	)

	find_package_handle_standard_args(Zlib DEFAULT_MSG ZLIB_LIBRARY ZLIB_INCLUDE_DIR)
endif()
