# Makou Reactor

[![CI/CD](https://github.com/myst6re/makoureactor/actions/workflows/build.yml/badge.svg)](https://github.com/myst6re/makoureactor/actions/workflows/build.yml)
[![Coverity Scan Build Status](https://img.shields.io/coverity/scan/8102.svg)](https://scan.coverity.com/projects/myst6re-makoureactor)

![Makou Reactor](src/qt/images/logo-shinra.png)

Final Fantasy VII field archive editor ([Forum](http://forums.qhimm.com/index.php?topic=9658.0)).

## Installing

Makou Reactor can be downloaded as a simple ZIP archive, or via repositories
when available:

- **Windows**: Download the last release on [GitHub](https://github.com/myst6re/makoureactor/releases).
- **MacOSX**: Download the latest release on [GitHub](https://github.com/myst6re/makoureactor/releases)
- **Debian/Ubuntu**: [GitHub](https://github.com/myst6re/makoureactor/releases)
- **ArchLinux**: Use packages from the [AUR](https://aur.archlinux.org/packages/?O=0&SeB=nd&K=makoureactor&outdated=&SB=n&SO=a&PP=50&do_Search=Go).
- **Other Linux**: Download the latest release on [GitHub](https://github.com/myst6re/makoureactor/releases)

## Contributing

You are welcome to contribute on this project, feel free to open issues and
PR on [GitHub](https://github.com/myst6re/makoureactor).
Ensure that you have commited in `develop` branch, otherwise I will not accept your
request.

## Building

If you are a developer and you want to build this software, follow the
instructions below.

### Requirements

- [zlib](http://www.zlib.net/)
- [ff7tk](https://www.github.com/sithlord48/ff7tk)
- CMake
- Qt 6.2+

### Windows

#### Zlib

Download and install Zlib using the [official installer](https://gnuwin32.sourceforge.net/downlinks/zlib.php) at this path: `C:\Program Files (x86)\zlib`

#### ff7tk

Download and install ff7tk using the [official installer](https://github.com/sithlord48/ff7tk/releases/download/continuous/ff7tk-continuous-win64-installer.exe) at the suggested default path.

#### Qt + Qt Creator

0. Download the online installer from https://www.qt.io/download-qt-installer
1. Install Qt 6 with these items checked:
   - **Qt/Qt 6.x.x (last version):** Check `MSVC 2019 64-bit`, `Qt Creator`, `Qt Shader Tools` and `Qt 5 Compatibility Module`
   - **Developer And Designer Tools:** Check `CMake` and `Ninja`
2. Run Qt Creator and open an existing project, select the CMakeLists.txt in the source directory of Makou Reactor

#### Visual Studio

> **Please note:**
>
> By default Visual Studio will pick the **x86-Release** build configuration, but you can choose any other profile available.

0. **REQUIRED!** Follow the steps to install Qt, if you didn't already
1. Download the the latest [Visual Studio Community](https://visualstudio.microsoft.com/vs/community/) installer
2. Run the installer and import this [.vsconfig](.vsconfig) file in the installer to pick the required components to build this project
3. Once installed, open this repository **as a folder** in Visual Studio 2019 and click the build button.

#### Optional: Visual Studio Code

0. **REQUIRED!** Follow the steps to install Qt, if you didn't already
1. **REQUIRED!** Follow the steps to install Visual Studio, if you didn't already
2. Download and install the latest [Visual Studio Code](https://code.visualstudio.com/) release
3. Install the following extensions:
   - https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools
   - https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools
4. Open this repository as a folder in Visual Studio code
5. Choose as build profile in the status bar `CMake: [Release]` (or one of the aforementioned profiles)
6. Click the button on the status bar `Build`

### macOS

In order to continue please make sure you have `brew` installed. See https://brew.sh/ for instructions.

#### Install dependencies

```sh
$ brew install cmake
$ brew install qt6
```

Close and reopen your terminal.

#### Build

> **Please note**: You can choose other build profiles if you want. See https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html for more information.

```sh
$ mkdir -p .dist/build .dist/install
$ cmake -S . -B .dist/build -DCMAKE_INSTALL_PREFIX=.dist/install -DCMAKE_BUILD_TYPE=Release
$ cmake --build .dist/build --config Release
```

### Linux

#### Install dependencies

##### Debian/Ubuntu

```sh
$ apt-get install build-essential cmake qt6-default qttools6-dev
```

##### Arch Linux

```sh
$ pacman -S --needed base-devel cmake qt6
```

#### Build

> **Please note**: You can choose other build profiles if you want. See https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html for more information.

```sh
$ mkdir -p .dist/build .dist/install
$ cmake -S . -B .dist/build -DCMAKE_INSTALL_PREFIX=.dist/install -DCMAKE_BUILD_TYPE=Release
$ cmake --build .dist/build --config Release
```

#### Install

```sh
$ cmake --build .dist/build --target install
```
