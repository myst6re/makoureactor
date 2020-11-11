include(ExternalProject)

set(ZLIB_VERSION "1.2.11")

ExternalProject_Add(
  zlib
  URL           "https://github.com/madler/zlib/archive/v${ZLIB_VERSION}.zip"
  INSTALL_DIR		"${CMAKE_BINARY_DIR}/vendor"
  CMAKE_ARGS
    "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
    "-DCMAKE_PREFIX_PATH=<INSTALL_DIR>"
    "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
    "-DCMAKE_MSVC_RUNTIME_LIBRARY=${CMAKE_MSVC_RUNTIME_LIBRARY}"
)
