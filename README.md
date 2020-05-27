# Makou Reactor

[![Build Status Linux/OSX](https://travis-ci.org/myst6re/makoureactor.svg?branch=develop)](https://travis-ci.org/myst6re/makoureactor)
[![Build status Win32](https://ci.appveyor.com/api/projects/status/lwiophf2d6cklg95/branch/develop?svg=true)](https://ci.appveyor.com/project/myst6re/makoureactor/branch/develop)
[![Coverity Scan Build Status](https://img.shields.io/coverity/scan/8102.svg)](https://scan.coverity.com/projects/myst6re-makoureactor)

![Makou Reactor](images/logo-shinra.png)

Final Fantasy VII field archive editor.

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
PR on GitHub.

## Building

If you are a developer and you want to build this software, follow the
instructions below.

### Requirements

 - Qt 5.4+
 - GLUT
 - zlib

On Windows, you can either use mingw32 (g++) or msvc to compile.

### With Qt Creator

Open `Makou_Reactor.pro` with Qt Creator IDE.
Please refer to the official documentation of Qt Creator
if you have troubles to compile and run Makou Reactor.

### Command line

#### Windows

Use the command prompt given in your Qt installation
to set your environment (`%QTDIR%/bin/qtvars.bat` for Qt 4
and `%QTDIR%/%compiler%/bin/qtenv2.bat` for Qt 5).
If you want to compile with msvc, run `vcvarsall.bat`
from your Microsoft Visual Studio installation
(eg: `C:\Program Files\Microsoft Visual Studio 11.0\VC\vcvarsall.bat`).

Then run:

~~~sh
mkdir build
cd build
qmake /path/to/Makou_Reactor.pro
# For mingw32
mingw32-make
# For msvc
jom
~~~

#### Linux

Set your environment:

 - [Qt 4 only] `QTDIR` must contains the directory of your Qt installation
 - `PATH` should contains the path to the bin directory of your Qt installation (`$QTDIR/bin`)
 - [Qt 4 only] Your compiler should be in the `PATH`

Then run:

~~~sh
mkdir build
cd build
qmake /path/to/Makou_Reactor.pro
make
~~~

## How to contribute

Pull requests are welcome on [GitHub](https://github.com/myst6re/makoureactor).
Ensure that you have commited in `develop` branch, otherwise I will not accept your
request.
