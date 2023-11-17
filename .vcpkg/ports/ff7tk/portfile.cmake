if(MSVC)
    set(VCPKG_C_FLAGS "-bigobj ${VCPKG_C_FLAGS}")
    set(VCPKG_CXX_FLAGS "-bigobj ${VCPKG_CXX_FLAGS}")
endif()

vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO sithlord48/ff7tk
  REF v0.83.3
  SHA512 4c1f01670a5c34619d172f1abb47e1f7d4a597fe7e05eeedc2ecb3829825498f1e51f2dfd1ac03a858c2a828dbd4a80cc6c95c7ef8b64d3a7d686062de5ecf0c
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
