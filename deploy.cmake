if(APPLE)
    set(DEPLOYQT_NAME macdeployqt)
    set(DEPLOYQT_ARGS)
elseif(WIN32)
    set(DEPLOYQT_NAME windeployqt)
    set(DEPLOYQT_ARGS --no-quick-import --no-webkit2 --no-angle --no-svg --no-webkit)
endif()

message("-- _qt_bin_dir: ${_qt_bin_dir}")
message("-- TARGET_FILE_DIR: ${TARGET_FILE_DIR} ${CMAKE_BINARY_DIR}")

if (APPLE OR WIN32)
    find_program(DEPLOYQT_EXECUTABLE "${DEPLOYQT_NAME}" HINTS "${_qt_bin_dir}")
    message("-- DEPLOYQT_EXECUTABLE: ${DEPLOYQT_EXECUTABLE}")
    execute_process(
        COMMAND "${DEPLOYQT_EXECUTABLE}"
        ${DEPLOYQT_ARGS} "${TARGET_FILE_DIR}"
    )
endif()
