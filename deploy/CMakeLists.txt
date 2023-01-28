# Deploy translations
if(APPLE)
    set(TRANSLATIONS_PATH ${PROJECT_NAME}.app/Contents/MacOS/translations)
elseif(WIN32)
    set(TRANSLATIONS_PATH translations)
else()
    set(TRANSLATIONS_PATH share/makoureactor/translations)
endif()

install(FILES ${QM_FILES} DESTINATION "${TRANSLATIONS_PATH}")

# Deploy ff7tk
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.20.0")
    # Remove \ occurrences
    cmake_path(SET _ff7tk_ROOT "${ff7tk_DIR}/../..")
else()
    set(_ff7tk_ROOT "${ff7tk_DIR}/../..")
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

list(TRANSFORM LANGS REPLACE ".+" "${_ff7tk_ROOT}/share/ff7tk/translations/ff7tk_\\0.qm" OUTPUT_VARIABLE FF7TK_QM_FILES)
if(BUNDLE_FF7TK_QM)
    install(FILES ${FF7TK_QM_FILES} DESTINATION "${TRANSLATIONS_PATH}")
endif()
# Deploy Qt using macdeployqt and windeployqt scripts
if((APPLE AND GUI) OR WIN32)
    if(WIN32)
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            find_program(DEPLOYQT_EXECUTABLE NAMES windeployqt.debug.bat)
        else()
            find_program(DEPLOYQT_EXECUTABLE NAMES windeployqt)
        endif()
        get_filename_component(_qt_bin_dir ${DEPLOYQT_EXECUTABLE} DIRECTORY)
        if(GUI)
            if (X_VCPKG_APPLOCAL_DEPS_INSTALL)
                install(TARGETS ${GUI_TARGET} RUNTIME DESTINATION .)
            else()
                install(TARGETS ${GUI_TARGET}
                    RUNTIME_DEPENDENCIES
                        PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"
                        POST_EXCLUDE_REGEXES ".*system32/.*\\.dll"
                        DIRECTORIES ${QT_BIN_DIR}
                    RUNTIME DESTINATION .)
            endif()
        elseif(CLI)
            if (X_VCPKG_APPLOCAL_DEPS_INSTALL)
                install(TARGETS ${CLI_TARGET} RUNTIME DESTINATION .)
            else()
                install(TARGETS ${CLI_TARGET}
                    RUNTIME_DEPENDENCIES
                        PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"
                        POST_EXCLUDE_REGEXES ".*system32/.*\\.dll"
                        DIRECTORIES ${QT_BIN_DIR}
                    RUNTIME DESTINATION .)
            endif()
        endif()
    elseif(APPLE)
        find_program(DEPLOYQT_EXECUTABLE NAMES macdeployqt)
        get_filename_component(_qt_bin_dir ${DEPLOYQT_EXECUTABLE} DIRECTORY)
    endif()
    install(CODE "set(_target_file_dir \"${QT_DEPLOY_TMP_DIR}\")")
    install(CODE "set(_target_bundle_name \"${PROJECT_NAME}.app\")")
    install(CODE "set(_qt_translations_dir \"${_qt_translations_dir}\")")
    install(CODE "set(_qt_bin_dir \"${_qt_bin_dir}\")")
    install(CODE "set(LANGS \"${LANGS}\")")
    install(CODE "set(DEPLOYQT_EXECUTABLE \"${DEPLOYQT_EXECUTABLE}\")")
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
        set(_ZLIB_DLL_FILENAME "zlib1.dll")
    endif()
    install(FILES "${_ZLIB_DLL_PATH}/${_ZLIB_DLL_FILENAME}" DESTINATION ".")
endif()
