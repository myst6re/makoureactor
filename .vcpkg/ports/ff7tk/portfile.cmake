if(VCPKG_TARGET_IS_WINDOWS)
  set(VCPKG_C_FLAGS "-bigobj ${VCPKG_C_FLAGS}")
  set(VCPKG_CXX_FLAGS "-bigobj ${VCPKG_CXX_FLAGS}")
endif()

vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO sithlord48/ff7tk
  REF v1.3.0
  SHA512 ce58f6f18e9fd2c98200eae2c0e5e9e3bec58111c075755724d3cb37715b761a70fc5bf5136bc6f5659c230bc05c1515d2876e805c1e3ea483fdb09096abdb8f
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
