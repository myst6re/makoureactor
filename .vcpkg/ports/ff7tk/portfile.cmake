if(VCPKG_TARGET_IS_WINDOWS)
  set(VCPKG_C_FLAGS "-bigobj ${VCPKG_C_FLAGS}")
  set(VCPKG_CXX_FLAGS "-bigobj ${VCPKG_CXX_FLAGS}")
endif()

vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO sithlord48/ff7tk
  REF v1.2.0
  SHA512 7968f18625e587b00d0e2f6106b7d697e74f4c5c3f2791953663dbbce4c55a7a7bf831c6968a4a4847149a7a01abf225cff4f24f3ff4b0313554c1a20901f974
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
