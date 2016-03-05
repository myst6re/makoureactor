# Makou Reactor

[![Build Status](https://travis-ci.org/myst6re/makoureactor.svg?branch=develop)](https://travis-ci.org/myst6re/makoureactor)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/myst6re-makoureactor)](https://img.shields.io/coverity/scan/8102.svg)

## Installing

 - **Windows**: Download the last release on [GitHub](https://github.com/myst6re/makoureactor/releases).
 - **Debian/Ubuntu**: Use ppa:sithlord48/ff7-daily ppa and install makoureactor package.
 - **ArchLinux**: Use makoureactor-git package.
 - **MacOSX**: We not provide binaries for now, but you can compile Makou Reactor on OSX.

## Building

### Requirements

 - Qt 4.8 or Qt 5
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
