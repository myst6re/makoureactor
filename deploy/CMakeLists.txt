# Deploy ff7tk
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.20.0")
    # Remove \ occurrences
    cmake_path(SET _ff7tk_ROOT "${ff7tk_DIR}/../../..")
else()
    set(_ff7tk_ROOT "${ff7tk_DIR}/../../..")
endif()

if(WIN32)
    set(FF7TK_DEPENDS
        ff7tk
        ff7tkWidgets
        ff7tkFormats
        ff7tkUtils
    )
    if(MINGW)
        # libff7tkXxx.dll
        list(TRANSFORM FF7TK_DEPENDS REPLACE ".+" "${_ff7tk_ROOT}/bin/lib\\0.dll")
    else()
        # ff7tkXxx.dll
        list(TRANSFORM FF7TK_DEPENDS REPLACE ".+" "${_ff7tk_ROOT}/bin/\\0.dll")
    endif()
    install(FILES ${FF7TK_DEPENDS} DESTINATION ".")
endif()

list(TRANSFORM LANGS REPLACE ".+" "${_ff7tk_ROOT}/share/ff7tk/lang/ff7tk_\\0.qm" OUTPUT_VARIABLE FF7TK_QM_FILES)
install(FILES ${FF7TK_QM_FILES} DESTINATION "${TRANSLATIONS_PATH}")

# Deploy Qt using macdeployqt and windeployqt scripts
if((APPLE AND GUI) OR WIN32)
    install(CODE "set(_target_file_dir \"${QT_DEPLOY_TMP_DIR}\")")
    install(CODE "set(_target_bundle_name \"${PROJECT_NAME}.app\")")
    install(CODE "set(_qt_translations_dir \"${_qt_translations_dir}\")")
    install(CODE "set(_qt_bin_dir \"${_qt_bin_dir}\")")
    install(CODE "set(LANGS \"${LANGS}\")")
    install(SCRIPT deployqt.cmake)
endif()

# Deploy zlib
if(WIN32)
    get_filename_component(_ZLIB_DLL_PATH "${ZLIB_LIBRARY_RELEASE}" DIRECTORY)
    get_filename_component(_ZLIB_DLL_PATH "${_ZLIB_DLL_PATH}" DIRECTORY)
    set(_ZLIB_DLL_PATH "${_ZLIB_DLL_PATH}/bin")
    if(MINGW)
        set(_ZLIB_DLL_FILENAME "libzlib.dll")
    else()
        set(_ZLIB_DLL_FILENAME "zlib.dll")
    endif()
    install(FILES "${_ZLIB_DLL_PATH}/${_ZLIB_DLL_FILENAME}" DESTINATION ".")
endif()
