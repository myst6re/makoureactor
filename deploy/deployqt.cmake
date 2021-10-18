if(APPLE)
    set(DEPLOYQT_NAME mac)
    set(DEPLOYQT_ARGS)
elseif(WIN32)
    set(DEPLOYQT_NAME win)
    if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
        list(JOIN LANGS "," QT_LANGS)
        set(DEPLOYQT_ARGS --no-quick-import --translations ${QT_LANGS})
    else()
        set(DEPLOYQT_ARGS --no-quick-import --no-webkit2 --no-angle --no-svg --no-webkit)
    endif()
endif()

if(APPLE OR WIN32)
    find_program(DEPLOYQT_EXECUTABLE "${DEPLOYQT_NAME}deployqt" HINTS "${_qt_bin_dir}")
    execute_process(
        COMMAND "${DEPLOYQT_EXECUTABLE}" ${DEPLOYQT_ARGS} "${TARGET_FILE_DIR}"
        RESULT_VARIABLE ret
    )

    if(NOT ret EQUAL "0")
        message(FATAL_ERROR "${DEPLOYQT_EXECUTABLE} returns ${ret}")
    endif()
endif()
