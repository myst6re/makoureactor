set(COMMIT_HASH v0.83.0)
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

file(GLOB FF7TK_CMAKE_FILES ${CURRENT_PACKAGES_DIR}/debug/lib/cmake/${PORT}/*.cmake)
file(COPY ${FF7TK_CMAKE_FILES} DESTINATION ${CURRENT_PACKAGES_DIR}/debug/share/${PORT})
file(REMOVE ${FF7TK_CMAKE_FILES})

file(GLOB FF7TK_CMAKE_FILES ${CURRENT_PACKAGES_DIR}/lib/cmake/${PORT}/*.cmake)
file(COPY ${FF7TK_CMAKE_FILES} DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT})
file(REMOVE ${FF7TK_CMAKE_FILES})

vcpkg_cmake_config_fixup()


file(MAKE_DIRECTORY "${CURRENT_PACKAGES_DIR}/debug/lib/pkgconfig")
file(RENAME "${CURRENT_PACKAGES_DIR}/debug/share/pkgconfig/${PORT}.pc" "${CURRENT_PACKAGES_DIR}/debug/lib/pkgconfig/${PORT}.pc")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share/pkgconfig")

file(MAKE_DIRECTORY "${CURRENT_PACKAGES_DIR}/lib/pkgconfig")
file(RENAME "${CURRENT_PACKAGES_DIR}/share/pkgconfig/${PORT}.pc" "${CURRENT_PACKAGES_DIR}/lib/pkgconfig/${PORT}.pc")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/share/pkgconfig")
vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include" "${CURRENT_PACKAGES_DIR}/debug/lib/cmake" "${CURRENT_PACKAGES_DIR}/lib/cmake" "${CURRENT_PACKAGES_DIR}/debug/share")

file(INSTALL ${SOURCE_PATH}/COPYING.TXT DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
