if(APPLE)
    set(_target_file_dir "${_target_file_dir}/${_target_bundle_name}")

    set(DEPLOYQT_NAME mac)
    set(DEPLOYQT_ARGS)
elseif(WIN32)
    set(DEPLOYQT_NAME win)
    list(JOIN LANGS "," QT_LANGS)
    set(DEPLOYQT_ARGS --no-quick-import -network -xml -qml --translations ${QT_LANGS})
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
    file(COPY ${QT_QM_FILES} DESTINATION "${_target_file_dir}/Contents/MacOS/translations")
elseif(WIN32)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        file(COPY ${_qt_bin_dir}/Qt6Core5Compatd.dll DESTINATION "${_target_file_dir}")
    else()
        file(COPY ${_qt_bin_dir}/Qt6Core5Compat.dll DESTINATION "${_target_file_dir}")
    endif()
endif()
