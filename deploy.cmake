if(APPLE)
    set(DEPLOYQT_NAME macdeployqt)
elseif(WIN32)
    set(DEPLOYQT_NAME windeployqt)
endif()

if (APPLE OR WIN32)
    find_program(DEPLOYQT_EXECUTABLE "${DEPLOYQT_NAME}" HINTS "${_qt_bin_dir}")
    execute_process(
        COMMAND "${DEPLOYQT_EXECUTABLE}"
        "${TARGET_FILE_DIR}"
    )
endif()

if(WIN32)
    set (FF7TK_DEPENDS
        ${ff7tk_DIR}/../../../bin/ff7tk.dll
    )
endif()
