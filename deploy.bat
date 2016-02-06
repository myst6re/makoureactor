@echo off

set OUTPUT_DIR=deploy
set EXE_PATH=release\Makou_Reactor.exe
set LIB_DIR=%QTDIR%\bin
set QT_TR_DIR=%QTDIR%\translations
set LANGUAGES=fr ja

rem Create target directory
if not exist %OUTPUT_DIR% mkdir %OUTPUT_DIR%

rem Deploy DLLs
%LIB_DIR%\windeployqt.exe --force --release --dir %OUTPUT_DIR% --no-quick-import --no-translations --no-webkit2 --no-svg --no-webkit %EXE_PATH%

rem Removing unused DLLs
rem del /q %OUTPUT_DIR%\opengl32sw.dll

rem Deploy Translations
for %%l in (%LANGUAGES%) do (
    %LIB_DIR%\lconvert -o %OUTPUT_DIR%\qt_%%l.qm -no-obsolete -no-ui-lines %QT_TR_DIR%\qtbase_%%l.qm
    echo "Create %QT_TR_DIR%\qtbase_%%l.qm"
)

rem Deploy Exe
xcopy /y %EXE_PATH% %OUTPUT_DIR%


rem Compress Exe and DLLs. Note: DLLs in platforms/ directory should not be compressed.
upx %OUTPUT_DIR%\*.dll %OUTPUT_DIR%\*.exe

exit 0
