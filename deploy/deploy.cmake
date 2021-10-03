if(APPLE)
    set(DEPLOYQT_NAME mac)
    set(DEPLOYQT_ARGS)
elseif(WIN32)
    set(DEPLOYQT_NAME win)
    set(DEPLOYQT_ARGS --no-quick-import --no-webkit2 --no-angle --no-svg --no-webkit)
else()
    set(DEPLOYQT_NAME linux)
    set(DEPLOYQT_ARGS -appimage -extra-plugins=iconengines,platformthemes/libqgtk3.so)
endif()

find_program(DEPLOYQT_EXECUTABLE "${DEPLOYQT_NAME}deployqt" HINTS "${_qt_bin_dir}")
if(UNIX)
    #set(ENV{QTDIR} "")
    #set(ENV{QT_PLUGIN_PATH} "")
    #set(ENV{LD_LIBRARY_PATH} "${ff7tk_DIR}/../..")
    #execute_process(
    #    COMMAND "${DEPLOYQT_EXECUTABLE}"
    #    "${TARGET_FILE_DIR}" ${DEPLOYQT_ARGS}
    #)
else()
    execute_process(
        COMMAND "${DEPLOYQT_EXECUTABLE}"
        ${DEPLOYQT_ARGS} "${TARGET_FILE_DIR}"
    )
endif()
