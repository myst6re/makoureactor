# Deploy ff7tk
if(VCPKG_TOOLCHAIN)
    cmake_path(SET _ff7tk_ROOT "${ff7tk_DIR}/../..")
else()
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.20.0")
        # Remove \ occurrences
        cmake_path(SET _ff7tk_ROOT "${ff7tk_DIR}/../../..")
    else()
        set(_ff7tk_ROOT "${ff7tk_DIR}/../../..")
    endif()
endif()

if(WIN32)
  if(GUI)
    set(FF7TK_DEPENDS
        ff7tk
        ff7tkData
        ff7tkQtWidgets
        ff7tkFormats
        ff7tkUtils
    )
  else()
    set(FF7TK_DEPENDS
      ff7tk
      ff7tkData
      ff7tkFormats
      ff7tkUtils
    )
  endif()
    if(MINGW)
        # libff7tkXxx.dll
        list(TRANSFORM FF7TK_DEPENDS REPLACE ".+" "${_ff7tk_ROOT}/bin/lib\\0.dll")
    else()
        # ff7tkXxx.dll
        list(TRANSFORM FF7TK_DEPENDS REPLACE ".+" "${_ff7tk_ROOT}/bin/\\0.dll")
    endif()
    install(FILES ${FF7TK_DEPENDS} DESTINATION ".")
endif()

# Deploy Qt using macdeployqt and windeployqt scripts
if((APPLE AND GUI) OR WIN32)
    install(CODE "set(_target_file_dir \"${QT_DEPLOY_TMP_DIR}\")")
    install(CODE "set(_target_bundle_name \"${PROJECT_NAME}.app\")")
    install(CODE "set(_qt_translations_dir \"${_qt_translations_dir}\")")
    install(CODE "set(_qt_bin_dir \"${_qt_bin_dir}\")")
    install(CODE "set(LANGS \"${LANGS}\")")
    install(CODE "set(CMAKE_BUILD_TYPE \"${CMAKE_BUILD_TYPE}\")")
    install(SCRIPT deployqt.cmake)
endif()

# Deploy zlib
if(WIN32)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND ZLIB_LIBRARY_DEBUG)
        get_filename_component(_ZLIB_DLL_PATH "${ZLIB_LIBRARY_DEBUG}" DIRECTORY)
    else()
        get_filename_component(_ZLIB_DLL_PATH "${ZLIB_LIBRARY_RELEASE}" DIRECTORY)
    endif()
    get_filename_component(_ZLIB_DLL_PATH "${_ZLIB_DLL_PATH}" DIRECTORY)
    set(_ZLIB_DLL_PATH "${_ZLIB_DLL_PATH}/bin")
    if(MINGW)
        set(_ZLIB_DLL_FILENAME "libzlib")
    else()
        set(_ZLIB_DLL_FILENAME "zlib")
    endif()
    if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND ZLIB_LIBRARY_DEBUG)
        set(_ZLIB_DLL_FILENAME "${_ZLIB_DLL_FILENAME}d")
    endif()
    if(VCPKG_TOOLCHAIN)
        set(_ZLIB_DLL_FILENAME "${_ZLIB_DLL_FILENAME}1")
    endif()
    set(_ZLIB_DLL_FILENAME "${_ZLIB_DLL_FILENAME}.dll")
    install(FILES "${_ZLIB_DLL_PATH}/${_ZLIB_DLL_FILENAME}" DESTINATION ".")
endif()
