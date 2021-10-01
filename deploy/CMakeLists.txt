install(CODE "set(TARGET_FILE_DIR \"${DEPLOY_TEMP}\")")
install(CODE "set(_qt_bin_dir \"${_qt_bin_dir}\")")
install(CODE "set(ff7tk_DIR \"${ff7tk_DIR}\")")
install(SCRIPT deploy.cmake)
