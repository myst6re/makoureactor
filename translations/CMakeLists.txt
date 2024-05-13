set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

if(APPLE)
    set(TRANSLATIONS_PATH ${PROJECT_NAME}.app/Contents/MacOS/translations)
elseif(WIN32)
    set(TRANSLATIONS_PATH translations)
else()
    set(TRANSLATIONS_PATH share/makoureactor/translations)
endif()

find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS
    Core
    LinguistTools
)

set ( makoureactor_TRS
    Makou_Reactor_fr.ts
    Makou_Reactor_ja.ts
)

qt_create_translation(APP_QM_FILES ${CMAKE_SOURCE_DIR} ${makoureactor_TRS} OPTIONS -no-ui-lines -locations none)
add_custom_target(app_translations ALL DEPENDS ${APP_QM_FILES})
install(FILES ${APP_QM_FILES} DESTINATION "${TRANSLATIONS_PATH}")

if(WIN32 OR APPLE)
    file(REAL_PATH "${ff7tk_DIR}/../../../" ff7tk_ROOT )
    find_path(_ff7tk_I18N_PATH NAMES translations PATHS "${ff7tk_ROOT}" PATH_SUFFIXES "/share/ff7tk" NO_DEFAULT_PATH REQUIRED)
    file(REAL_PATH "${_ff7tk_I18N_PATH}/translations" ff7tk_I18N_PATH)
    install(FILES ${ff7tk_I18N_PATH}/ff7tk_fr.qm DESTINATION "${TRANSLATIONS_PATH}")
    install(FILES ${ff7tk_I18N_PATH}/ff7tk_ja.qm DESTINATION "${TRANSLATIONS_PATH}")
    if(APPLE)
        file(REAL_PATH "${_qt_bin_dir}/../" QT_ROOT_DIR)
        find_file(_QT_QM_FILE NAMES qtbase_fr.qm PATHS ${QT_ROOT_DIR} PATH_SUFFIXES "translations" "share/qt/translations" REQUIRED)
        get_filename_component(MAC_QT_LANG_PATH "${_QT_QM_FILE}" DIRECTORY)
        install(FILES ${MAC_QT_LANG_PATH}/qtbase_fr.qm DESTINATION ${TRANSLATIONS_PATH} RENAME qt_fr.qm)
        install(FILES ${MAC_QT_LANG_PATH}/qtbase_ja.qm DESTINATION ${TRANSLATIONS_PATH} RENAME qt_ja.qm)
    endif()
endif()
