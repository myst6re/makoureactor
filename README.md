# Makou Reactor

[![Build Status Linux/OSX](https://travis-ci.org/myst6re/makoureactor.svg?branch=develop)](https://travis-ci.org/myst6re/makoureactor)
[![Build status Win32](https://ci.appveyor.com/api/projects/status/lwiophf2d6cklg95/branch/develop?svg=true)](https://ci.appveyor.com/project/myst6re/makoureactor/branch/develop)
[![Coverity Scan Build Status](https://img.shields.io/coverity/scan/8102.svg)](https://scan.coverity.com/projects/myst6re-makoureactor)

![Makou Reactor](src/qt/images/logo-shinra.png)

Final Fantasy VII field archive editor ([Forum](http://forums.qhimm.com/index.php?topic=9658.0)).

## Installing

Makou Reactor can be downloaded as a simple ZIP archive, or via repositories
when available:

 - **Windows**: Download the last release on [GitHub](https://github.com/myst6re/makoureactor/releases).
 - **MacOSX**: Download the latest release on [GitHub](https://github.com/myst6re/makoureactor/releases)
 - **Debian/Ubuntu**: Add sithlord48's [Stable Release](https://launchpad.net/~sithlord48/+archive/ubuntu/ff7) or [Daily Builds](https://launchpad.net/~sithlord48/+archive/ubuntu/ff7-daily) ppa to install the makoureactor package
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

 - CMake
 - Qt 5.5+

### Windows

#### Qt

0) Download the online installer from https://www.qt.io/download-qt-installer ( remember to click the Download button )
1) Install Qt with these items checked:
   - **Packages categories:** `Latest release`
   - **Components:** Uncheck everything and pick only `MSVC 2019 32-bit`

#### Visual Studio

> **Please note:**
>
> By default Visual Studio will pick the **x86-Release** build configuration, but you can choose any other profile available.

0) **REQUIRED!** Follow the steps to install Qt, if you didn't already
0) Download the the latest [Visual Studio Community](https://visualstudio.microsoft.com/vs/community/) installer
1) Run the installer and import this [.vsconfig](.vsconfig) file in the installer to pick the required components to build this project
2) Once installed, open this repository **as a folder** in Visual Studio 2019 and click the build button.

#### Optional: Visual Studio Code

0) **REQUIRED!** Follow the steps to install Qt, if you didn't already
0) **REQUIRED!** Follow the steps to install Visual Studio, if you didn't already
1) Download and install the latest [Visual Studio Code](https://code.visualstudio.com/) release
2) Install the following extensions:
   - https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools
   - https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools
3) Open this repository as a folder in Visual Studio code
4) Choose as build profile in the status bar `CMake: [Release]` ( or one of the aforementioned profiles )
5) Click the button on the status bar `Build`
