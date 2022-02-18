if(APPLE)
    set(_target_file_dir "${_target_file_dir}/${_target_bundle_name}")

    set(DEPLOYQT_NAME mac)
    set(DEPLOYQT_ARGS)
elseif(WIN32)
    set(DEPLOYQT_NAME win)
    if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
        list(JOIN LANGS "," QT_LANGS)
        set(DEPLOYQT_ARGS --no-quick-import -xml -qml --translations ${QT_LANGS})
    else()
        set(DEPLOYQT_ARGS --no-quick-import --no-webkit2 --no-angle --no-svg --no-webkit -xml -qml -network)
    endif()
endif()

if(APPLE OR WIN32)
    find_program(DEPLOYQT_EXECUTABLE "${DEPLOYQT_NAME}deployqt" HINTS "${_qt_bin_dir}")
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
    file(COPY ${QT_QM_FILES} DESTINATION "${_target_file_dir}/Contents/Resources")
endif()
