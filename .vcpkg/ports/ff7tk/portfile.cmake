set(COMMIT_HASH 2e7b52d695a04ba6bbaba6f0dae1b173126a191d)
set(VCPKG_C_FLAGS "-bigobj ${VCPKG_C_FLAGS}")
set(VCPKG_CXX_FLAGS "-bigobj ${VCPKG_CXX_FLAGS}")

vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO sithlord48/ff7tk
  REF ${COMMIT_HASH}
  SHA512 690b523e6e1d179cf3d429f9c9391b8537a3092814165bc513c6aab90bd404e50f3d34df7719bd656d83bf5cc5468ebef20ca14917134647495db83eb2030f1c
  HEAD_REF master
)

vcpkg_cmake_configure(
  SOURCE_PATH ${SOURCE_PATH}
  OPTIONS
    "-DPACKAGE=OFF"
    "-DTESTS=OFF"
    "-DCMAKE_PROJECT_INCLUDE=${CMAKE_CURRENT_LIST_DIR}/qt.cmake"
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/${PORT})
vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include" "${CURRENT_PACKAGES_DIR}/debug/lib/cmake" "${CURRENT_PACKAGES_DIR}/lib/cmake" "${CURRENT_PACKAGES_DIR}/debug/share" "${CURRENT_PACKAGES_DIR}/share/licenses")
file(INSTALL ${SOURCE_PATH}/COPYING.TXT DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
