#!/bin/bash

# NOTE: Keep this script in the same directory as resources for AppImage creation
APPIMAGE_RESOURCES_DIR=$(dirname $(readlink -f $0))
echo "APPIMAGE_RESOURCES_DIR set to ${APPIMAGE_RESOURCES_DIR}"

if [[ "$#" -ne 3 ]]; then
  echo "Wrong number of arguments (\$# = $# args) provided."
  echo "Usage: generate.sh <build_directory_path> <release_name> <release_version>"
  echo "AppImage will be generated in the path this script is called from."
  exit 1
else
  BUILD_DIR=$(readlink -f $1)
  echo "BUILD_DIR set to ${BUILD_DIR}"
  RELEASE_NAME="$2"
  echo "RELEASE_NAME set to ${RELEASE_NAME}"
  RELEASE_VERSION="$3"
  echo "RELEASE_VERSION set to ${RELEASE_VERSION}"
fi

wget --timestamping --directory-prefix=${BUILD_DIR} \
  https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
chmod a+x ${BUILD_DIR}/linuxdeploy-x86_64.AppImage

wget --timestamping --directory-prefix=${BUILD_DIR} \
  https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod a+x ${BUILD_DIR}/linuxdeploy-plugin-qt-x86_64.AppImage

wget --timestamping --directory-prefix=${BUILD_DIR} \
  https://github.com/linuxdeploy/linuxdeploy-plugin-appimage/releases/download/continuous/linuxdeploy-plugin-appimage-x86_64.AppImage
chmod a+x ${BUILD_DIR}/linuxdeploy-plugin-appimage-x86_64.AppImage

ICONS_QT=()

for filename in ${APPIMAGE_RESOURCES_DIR}/icon-*px.png; do
  [[ ${filename} =~ ${APPIMAGE_RESOURCES_DIR}/icon-(.*)px.png ]];
  res=${BASH_REMATCH[1]}
  mkdir -p ${BUILD_DIR}/AppImage-icons/${res}x${res}
  # Copy icon to proper directory
  cp -v ${APPIMAGE_RESOURCES_DIR}/icon-${res}px.png ${BUILD_DIR}/AppImage-icons/${res}x${res}/${RELEASE_NAME}.png
  # Append icon filepath to array that will later be passed to linuxdeploy
  eval "ICONS_QT+=(${BUILD_DIR}/AppImage-icons/${res}x${res}/${RELEASE_NAME}.png)"
done

# Add data files into the AppDir
CMAKE_INSTALL_DIR=${APPIMAGE_RESOURCES_DIR}/../../.dist/build/bin
echo "CMake install directory is: ${CMAKE_INSTALL_DIR}"

CURRENT_APPDIR=${BUILD_DIR}/${RELEASE_NAME}.AppDir
mkdir -p ${CURRENT_APPDIR}/usr/bin
mkdir -p ${CURRENT_APPDIR}/usr/share/license
cp -av ${CMAKE_INSTALL_DIR}/${RELEASE_NAME} ${CURRENT_APPDIR}/usr/bin
cp -av ${CMAKE_INSTALL_DIR}/*.qm ${CURRENT_APPDIR}/usr/bin
cp -av ${CMAKE_INSTALL_DIR}/COPYING.TXT ${CURRENT_APPDIR}/usr/share/license/

# Pass UPDATE_INFORMATION and OUTPUT variables (used by linuxdeploy-plugin-appimage)
# to the environment of the linuxdeploy commands

${BUILD_DIR}/linuxdeploy-x86_64.AppImage \
  --appdir=${BUILD_DIR}/${RELEASE_NAME}.AppDir \
  --executable=${BUILD_DIR}//${RELEASE_NAME}.AppDir/usr/bin/${RELEASE_NAME} \
  --desktop-file=${APPIMAGE_RESOURCES_DIR}/${RELEASE_NAME}.desktop \
  ${ICONS_QT[@]/#/--icon-file=} \
  --plugin=qt

UPDATE_INFORMATION="zsync|https://github.com/myst6re/makoureactor/releases/download/continuous/${RELEASE_NAME}-${RELEASE_VERSION}-x64.AppImage.zsync" \
OUTPUT="${RELEASE_NAME}-${RELEASE_VERSION}-x86_64.AppImage" \
${BUILD_DIR}/linuxdeploy-plugin-appimage-x86_64.AppImage \
  --appdir=${BUILD_DIR}/${RELEASE_NAME}.AppDir
