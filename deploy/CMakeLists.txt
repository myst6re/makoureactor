# Deploy translations
if(APPLE)
    set(TRANSLATIONS_PATH ${PROJECT_NAME}.app/Contents/MacOS/translations)
elseif(WIN32)
    set(TRANSLATIONS_PATH translations)
else()
    set(TRANSLATIONS_PATH share/makoureactor/translations)
endif()

install(FILES ${QM_FILES} DESTINATION "${TRANSLATIONS_PATH}")
install(FILES ${ff7tk_QM_FILES} DESTINATION "${TRANSLATIONS_PATH}")

# Deploy Qt using macdeployqt and windeployqt scripts
if((APPLE AND GUI) OR WIN32)
    install(CODE "set(_target_file_dir \"${QT_DEPLOY_TMP_DIR}\")")
    install(CODE "set(_target_bundle_name \"${PROJECT_NAME}.app\")")
    install(CODE "set(_qt_translations_dir \"${_qt_translations_dir}\")")
    install(CODE "set(_qt_bin_dir \"${_qt_bin_dir}\")")
    install(CODE "set(LANGS \"${LANGS}\")")
    install(SCRIPT deployqt.cmake)
endif()
