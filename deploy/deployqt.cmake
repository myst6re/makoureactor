if(APPLE)
    set(_target_file_dir "${_target_file_dir}/${_target_bundle_name}")

    set(DEPLOYQT_ARGS)
elseif(WIN32)
    list(JOIN LANGS "," QT_LANGS)
    set(DEPLOYQT_ARGS --no-quick-import -network -xml -qml --translations ${QT_LANGS})
endif()

if(APPLE OR WIN32)
    execute_process(
        COMMAND "${DEPLOYQT_EXECUTABLE}" ${DEPLOYQT_ARGS} "${_target_file_dir}"
        RESULT_VARIABLE ret
    )

    if(NOT ret EQUAL "0")
        message(FATAL_ERROR "${DEPLOYQT_EXECUTABLE} returns ${ret}")
    endif()
endif()

if(APPLE)
    # Translations are not installed by macdeployqt script
    list(TRANSFORM LANGS REPLACE ".+" "${_qt_translations_dir}/qt_\\0.qm" OUTPUT_VARIABLE QT_QM_FILES)
    file(COPY ${QT_QM_FILES} DESTINATION "${_target_file_dir}/Contents/MacOS/translations")
endif()
