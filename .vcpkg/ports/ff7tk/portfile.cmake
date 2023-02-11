set(COMMIT_HASH v0.83.1)
set(VCPKG_C_FLAGS "-bigobj ${VCPKG_C_FLAGS}")
set(VCPKG_CXX_FLAGS "-bigobj ${VCPKG_CXX_FLAGS}")

vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO sithlord48/ff7tk
  REF ${COMMIT_HASH}
  SHA512 4edbdab324adcfa8b1b682ea472803e6c40d8e9094b2487e14bfc7aa5b6714ed9dbf65ebc5cab525a3b0e6c800f0d575afae1852350ec6094d6826d3701a4d46
  HEAD_REF master
)

vcpkg_cmake_configure(
  SOURCE_PATH ${SOURCE_PATH}
  OPTIONS
    "-DTESTS=OFF"
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()
vcpkg_cmake_config_fixup()
vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include" "${CURRENT_PACKAGES_DIR}/debug/lib/cmake" "${CURRENT_PACKAGES_DIR}/lib/cmake" "${CURRENT_PACKAGES_DIR}/debug/share")

file(RENAME ${CURRENT_PACKAGES_DIR}/share/licenses/${PORT}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
