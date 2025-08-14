if(VCPKG_TARGET_IS_WINDOWS)
  set(VCPKG_C_FLAGS "-bigobj ${VCPKG_C_FLAGS}")
  set(VCPKG_CXX_FLAGS "-bigobj ${VCPKG_CXX_FLAGS}")
endif()

vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO sithlord48/ff7tk
  REF bb08646a6dc1da05070bb0b23c6cd72d75aa2c4f
  SHA512 7796fe6b8a0199867e9acb65392c59ae8cb8e1a95e5f2e0541451d7cac05e7e40174f96f33859fdad68d2a018cc7528f8665e12eaf1a80078b81c9eb63751ff8
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
