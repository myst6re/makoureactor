TEMPLATE = app
TARGET = makoureactor

QT += core gui opengl widgets
CONFIG += c++11

# Input
HEADERS += \
    Window.h \
    Parameters.h \
    Data.h \
    core/PsfFile.h \
    core/field/BackgroundTilesFile.h \
    core/field/MaplistFile.h \
    widgets/AboutDialog.h \
    widgets/ArchivePreview.h \
    widgets/EmptyFieldWidget.h \
    widgets/HelpWidget.h \
    widgets/LgpWidget.h \
    widgets/PsfDialog.h \
    widgets/ScriptEditorWidgets/ScriptEditorSpecialPage.h \
    widgets/WalkmeshWidget.h \
    widgets/WalkmeshManager.h \
    widgets/VertexWidget.h \
    widgets/VarOrValueWidget.h \
    widgets/VarManager.h \
    widgets/TutWidget.h \
    widgets/TextPreview.h \
    widgets/TextManager.h \
    widgets/TextHighlighter.h \
    widgets/Search.h \
    widgets/ScriptList.h \
    widgets/ScriptEditor.h \
    widgets/QTaskBarButton.h \
    widgets/OrientationWidget.h \
    widgets/OpcodeList.h \
    widgets/ModelManager.h \
    widgets/MiscWidget.h \
    widgets/MassImportDialog.h \
    widgets/MassExportDialog.h \
    widgets/Listwidget.h \
    widgets/KeyEditorDialog.h \
    widgets/ImportDialog.h \
    widgets/HexLineEdit.h \
    widgets/GrpScriptList.h \
    widgets/FormatSelectionWidget.h \
    widgets/FontWidget.h \
    widgets/FontPalette.h \
    widgets/FontLetter.h \
    widgets/FontGrid.h \
    widgets/FontDisplay.h \
    widgets/FieldModel.h \
    widgets/EncounterWidget.h \
    widgets/EncounterTableWidget.h \
    widgets/ConfigWindow.h \
    widgets/ColorDisplay.h \
    widgets/BGDialog.h \
    widgets/ApercuBGLabel.h \
    widgets/ApercuBG.h \
    widgets/AnimEditorDialog.h \
    widgets/ScriptEditorWidgets/ScriptEditorWindowPage.h \
    widgets/ScriptEditorWidgets/ScriptEditorView.h \
    widgets/ScriptEditorWidgets/ScriptEditorStructPage.h \
    widgets/ScriptEditorWidgets/ScriptEditorMathPage.h \
    widgets/ScriptEditorWidgets/ScriptEditorGenericList.h \
    widgets/ScriptEditorWidgets/Delegate.h \
    core/WindowBinFile.h \
    core/Var.h \
    core/TimFile.h \
    core/TextureFile.h \
    core/TexFile.h \
    core/QLockedFile.h \
    core/PsColor.h \
    core/LZS.h \
    core/Lgp_p.h \
    core/Lgp.h \
    core/IsoArchive.h \
    core/IsoArchiveFF7.h \
    core/GZIP.h \
    core/GZIPPS.h \
    core/FF7Text.h \
    core/FF7Font.h \
    core/Config.h \
    core/field/TutFile.h \
    core/field/TdbFile.h \
    core/field/Section1File.h \
    core/field/Script.h \
    core/field/Palette.h \
    core/field/Opcode.h \
    core/field/InfFile.h \
    core/field/IdFile.h \
    core/field/GrpScript.h \
    core/field/FieldPS.h \
    core/field/FieldPC.h \
    core/field/FieldModelPart.h \
    core/field/FieldModelLoaderPS.h \
    core/field/FieldModelLoaderPC.h \
    core/field/FieldModelLoader.h \
    core/field/FieldModelFilePS.h \
    core/field/FieldModelFilePC.h \
    core/field/FieldModelFile.h \
    core/field/FieldArchivePS.h \
    core/field/FieldArchivePC.h \
    core/field/FieldArchiveIO.h \
    core/field/FieldArchiveIOPS.h \
    core/field/FieldArchiveIOPC.h \
    core/field/FieldArchive.h \
    core/field/Field.h \
    core/field/EncounterFile.h \
    core/field/CaFile.h \
    core/field/BackgroundFilePS.h \
    core/field/BackgroundFilePC.h \
    core/field/BackgroundFile.h \
    core/field/FieldIO.h \
    widgets/FontManager.h \
    core/field/TutFilePC.h \
    core/field/TutFileStandard.h \
    core/field/FieldPart.h \
    widgets/ModelManagerPS.h \
    widgets/ModelManagerPC.h \
    FieldModelThread.h \
    core/world/TblFile.h \
    core/field/PaletteIO.h \
    core/field/BackgroundTiles.h \
    core/field/BackgroundTilesIO.h \
    core/field/BackgroundTextures.h \
    core/field/BackgroundTexturesIO.h \
    core/field/BackgroundIO.h \
    widgets/OperationsManager.h \
    core/Archive.h \
    widgets/ScriptEditorWidgets/ScriptEditorMoviePage.h \
    widgets/SearchAll.h \
    core/field/CharArchive.h \
    core/field/FieldPSDemo.h \
    core/IO.h \
    core/field/FieldModelAnimation.h \
    core/field/FieldModelSkeleton.h \
    core/field/BcxFile.h \
    core/field/BsxFile.h \
    core/field/HrcFile.h \
    core/field/AFile.h \
    core/field/PFile.h \
    core/field/RsdFile.h \
    core/field/FieldModelTextureRef.h \
    core/field/FieldModelTextureRefPC.h \
    core/field/FieldModelTextureRefPS.h \
    widgets/ScriptManager.h \
    widgets/FieldList.h \
    widgets/Splitter.h \
    core/AkaoIO.h \
    core/Akao.h \
    core/Clipboard.h \
    widgets/ModelColorsLayout.h

SOURCES += \
    Window.cpp \
    core/PsfFile.cpp \
    core/field/BackgroundTilesFile.cpp \
    core/field/MaplistFile.cpp \
    txt.cpp \
    main.cpp \
    Data.cpp \
    widgets/AboutDialog.cpp \
    widgets/ArchivePreview.cpp \
    widgets/EmptyFieldWidget.cpp \
    widgets/HelpWidget.cpp \
    widgets/LgpWidget.cpp \
    widgets/PsfDialog.cpp \
    widgets/ScriptEditorWidgets/ScriptEditorSpecialPage.cpp \
    widgets/WalkmeshWidget.cpp \
    widgets/WalkmeshManager.cpp \
    widgets/VertexWidget.cpp \
    widgets/VarOrValueWidget.cpp \
    widgets/VarManager.cpp \
    widgets/TutWidget.cpp \
    widgets/TextPreview.cpp \
    widgets/TextManager.cpp \
    widgets/TextHighlighter.cpp \
    widgets/Search.cpp \
    widgets/ScriptList.cpp \
    widgets/ScriptEditor.cpp \
    widgets/QTaskBarButton.cpp \
    widgets/OrientationWidget.cpp \
    widgets/OpcodeList.cpp \
    widgets/ModelManager.cpp \
    widgets/MiscWidget.cpp \
    widgets/MassImportDialog.cpp \
    widgets/MassExportDialog.cpp \
    widgets/Listwidget.cpp \
    widgets/KeyEditorDialog.cpp \
    widgets/ImportDialog.cpp \
    widgets/HexLineEdit.cpp \
    widgets/GrpScriptList.cpp \
    widgets/FormatSelectionWidget.cpp \
    widgets/FontWidget.cpp \
    widgets/FontPalette.cpp \
    widgets/FontLetter.cpp \
    widgets/FontGrid.cpp \
    widgets/FontDisplay.cpp \
    widgets/FieldModel.cpp \
    widgets/EncounterWidget.cpp \
    widgets/EncounterTableWidget.cpp \
    widgets/ConfigWindow.cpp \
    widgets/ColorDisplay.cpp \
    widgets/BGDialog.cpp \
    widgets/ApercuBGLabel.cpp \
    widgets/ApercuBG.cpp \
    widgets/AnimEditorDialog.cpp \
    widgets/ScriptEditorWidgets/ScriptEditorWindowPage.cpp \
    widgets/ScriptEditorWidgets/ScriptEditorView.cpp \
    widgets/ScriptEditorWidgets/ScriptEditorStructPage.cpp \
    widgets/ScriptEditorWidgets/ScriptEditorMathPage.cpp \
    widgets/ScriptEditorWidgets/ScriptEditorGenericList.cpp \
    widgets/ScriptEditorWidgets/Delegate.cpp \
    core/WindowBinFile.cpp \
    core/Var.cpp \
    core/TimFile.cpp \
    core/TextureFile.cpp \
    core/TexFile.cpp \
    core/QLockedFile.cpp \
    core/PsColor.cpp \
    core/LZS.cpp \
    core/Lgp_p.cpp \
    core/Lgp.cpp \
    core/IsoArchive.cpp \
    core/IsoArchiveFF7.cpp \
    core/GZIP.cpp \
    core/GZIPPS.cpp \
    core/FF7Text.cpp \
    core/FF7Font.cpp \
    core/Config.cpp \
    core/field/TutFile.cpp \
    core/field/TdbFile.cpp \
    core/field/Section1File.cpp \
    core/field/Script.cpp \
    core/field/Palette.cpp \
    core/field/Opcode.cpp \
    core/field/InfFile.cpp \
    core/field/IdFile.cpp \
    core/field/GrpScript.cpp \
    core/field/FieldPS.cpp \
    core/field/FieldPC.cpp \
    core/field/FieldModelPart.cpp \
    core/field/FieldModelLoaderPS.cpp \
    core/field/FieldModelLoaderPC.cpp \
    core/field/FieldModelLoader.cpp \
    core/field/FieldModelFilePS.cpp \
    core/field/FieldModelFilePC.cpp \
    core/field/FieldModelFile.cpp \
    core/field/FieldArchivePS.cpp \
    core/field/FieldArchivePC.cpp \
    core/field/FieldArchiveIO.cpp \
    core/field/FieldArchiveIOPS.cpp \
    core/field/FieldArchiveIOPC.cpp \
    core/field/FieldArchive.cpp \
    core/field/Field.cpp \
    core/field/EncounterFile.cpp \
    core/field/CaFile.cpp \
    core/field/BackgroundFilePS.cpp \
    core/field/BackgroundFilePC.cpp \
    core/field/BackgroundFile.cpp \
    core/field/FieldIO.cpp \
    widgets/FontManager.cpp \
    core/field/TutFilePC.cpp \
    core/field/TutFileStandard.cpp \
    core/field/FieldPart.cpp \
    widgets/ModelManagerPS.cpp \
    widgets/ModelManagerPC.cpp \
    FieldModelThread.cpp \
    core/world/TblFile.cpp \
    core/field/PaletteIO.cpp \
    core/field/BackgroundTiles.cpp \
    core/field/BackgroundTilesIO.cpp \
    core/field/BackgroundTextures.cpp \
    core/field/BackgroundTexturesIO.cpp \
    core/field/BackgroundIO.cpp \
    widgets/OperationsManager.cpp \
    core/Archive.cpp \
    widgets/ScriptEditorWidgets/ScriptEditorMoviePage.cpp \
    widgets/SearchAll.cpp \
    core/field/CharArchive.cpp \
    core/field/FieldPSDemo.cpp \
    core/IO.cpp \
    core/field/FieldModelAnimation.cpp \
    core/field/FieldModelSkeleton.cpp \
    core/field/BcxFile.cpp \
    core/field/BsxFile.cpp \
    core/field/HrcFile.cpp \
    core/field/AFile.cpp \
    core/field/PFile.cpp \
    core/field/RsdFile.cpp \
    core/field/FieldModelTextureRef.cpp \
    core/field/FieldModelTextureRefPC.cpp \
    core/field/FieldModelTextureRefPS.cpp \
    widgets/ScriptManager.cpp \
    widgets/FieldList.cpp \
    widgets/Splitter.cpp \
    core/AkaoIO.cpp \
    core/Akao.cpp \
    core/Clipboard.cpp \
    widgets/ModelColorsLayout.cpp

TRANSLATIONS += Makou_Reactor_fr.ts  \
    Makou_Reactor_ja.ts

CODECFORTR = UTF-8
CODECFORSRC = UTF-8

RESOURCES += Makou_Reactor.qrc
macx {
    ICON = images/Makou_Reactor.icns
}

# include zlib
!win32 {
    LIBS += -lz
} else {
    exists($$[QT_INSTALL_PREFIX]/include/QtZlib) {
        INCLUDEPATH += $$[QT_INSTALL_PREFIX]/include/QtZlib
    } else {
        INCLUDEPATH += zlib-1.2.7
        greaterThan(QT_MAJOR_VERSION, 4) {
            LIBS += -lz
        }
    }
}

win32 {
    RC_FILE = Makou_Reactor.rc
    !contains(CONFIG, "console") {
        TARGET = Makou_Reactor
    }
    # Regedit features
    LIBS += -ladvapi32 -lshell32
    # OpenGL features
    LIBS += -lopengl32 -lGlU32
    # QTaskbarButton
    greaterThan(QT_MAJOR_VERSION, 4):qtHaveModule(winextras):!contains(CONFIG, "console") {
        QT += winextras
        DEFINES += TASKBAR_BUTTON QTASKBAR_WIN_QT5
    } else {
        TASKBAR_BUTTON {
            DEFINES += TASKBAR_BUTTON
            LIBS += -lole32
            INCLUDEPATH += include
        }
    }
}

OTHER_FILES += Makou_Reactor.rc \
    deploy.bat \
    vars.cfg \
    .travis.yml \
    README.md \
    .clang-format \
    appveyor.yml
DISTFILES += Makou_Reactor.desktop

# call lrelease to make the qm files.
system(lrelease Makou_Reactor.pro)

# all other *nix (except for symbian)
unix:!macx:!symbian {
    LIBS += -lglut -lGLU

    target.path = /usr/bin

    langfiles.files = *.qm
    langfiles.path = /usr/share/makoureactor

    icon.files = images/logo-shinra.png
    icon.path = /usr/share/pixmaps

    desktop.files = Makou_Reactor.desktop
    desktop.path = /usr/share/applications

    INSTALLS += target langfiles icon desktop
}

contains(CONFIG, "console") {
    DEFINES += MR_CONSOLE
    QT -= opengl widgets

    HEADERS = \
        Arguments.h \
        ArgumentsExport.h \
        CLI.h \
        Parameters.h \
        Data.h \
        core/PsfFile.h \
        core/field/BackgroundTilesFile.h \
        core/field/MaplistFile.h \
        core/WindowBinFile.h \
        core/Var.h \
        core/TimFile.h \
        core/TextureFile.h \
        core/TexFile.h \
        core/QLockedFile.h \
        core/PsColor.h \
        core/LZS.h \
        core/Lgp_p.h \
        core/Lgp.h \
        core/IsoArchive.h \
        core/IsoArchiveFF7.h \
        core/GZIP.h \
        core/GZIPPS.h \
        core/FF7Text.h \
        core/FF7Font.h \
        core/Config.h \
        core/field/TutFile.h \
        core/field/TdbFile.h \
        core/field/Section1File.h \
        core/field/Script.h \
        core/field/Palette.h \
        core/field/Opcode.h \
        core/field/InfFile.h \
        core/field/IdFile.h \
        core/field/GrpScript.h \
        core/field/FieldPS.h \
        core/field/FieldPC.h \
        core/field/FieldModelPart.h \
        core/field/FieldModelLoaderPS.h \
        core/field/FieldModelLoaderPC.h \
        core/field/FieldModelLoader.h \
        core/field/FieldModelFilePS.h \
        core/field/FieldModelFilePC.h \
        core/field/FieldModelFile.h \
        core/field/FieldArchivePS.h \
        core/field/FieldArchivePC.h \
        core/field/FieldArchiveIO.h \
        core/field/FieldArchiveIOPS.h \
        core/field/FieldArchiveIOPC.h \
        core/field/FieldArchive.h \
        core/field/Field.h \
        core/field/EncounterFile.h \
        core/field/CaFile.h \
        core/field/BackgroundFilePS.h \
        core/field/BackgroundFilePC.h \
        core/field/BackgroundFile.h \
        core/field/FieldIO.h \
        core/field/TutFilePC.h \
        core/field/TutFileStandard.h \
        core/field/FieldPart.h \
        core/world/TblFile.h \
        core/field/PaletteIO.h \
        core/field/BackgroundTiles.h \
        core/field/BackgroundTilesIO.h \
        core/field/BackgroundTextures.h \
        core/field/BackgroundTexturesIO.h \
        core/field/BackgroundIO.h \
        core/Archive.h \
        core/field/CharArchive.h \
        core/field/FieldPSDemo.h \
        core/IO.h \
        core/field/FieldModelAnimation.h \
        core/field/FieldModelSkeleton.h \
        core/field/BcxFile.h \
        core/field/BsxFile.h \
        core/field/HrcFile.h \
        core/field/AFile.h \
        core/field/PFile.h \
        core/field/RsdFile.h \
        core/field/FieldModelTextureRef.h \
        core/field/FieldModelTextureRefPC.h \
        core/field/FieldModelTextureRefPS.h \
        core/AkaoIO.h \
        core/Akao.h

    SOURCES = \
        Arguments.cpp \
        ArgumentsExport.cpp \
        CLI.cpp \
        main.cpp \
        txt.cpp \
        Data.cpp \
        core/PsfFile.cpp \
        core/field/BackgroundTilesFile.cpp \
        core/field/MaplistFile.cpp \
        core/WindowBinFile.cpp \
        core/Var.cpp \
        core/TimFile.cpp \
        core/TextureFile.cpp \
        core/TexFile.cpp \
        core/QLockedFile.cpp \
        core/PsColor.cpp \
        core/LZS.cpp \
        core/Lgp_p.cpp \
        core/Lgp.cpp \
        core/IsoArchive.cpp \
        core/IsoArchiveFF7.cpp \
        core/GZIP.cpp \
        core/GZIPPS.cpp \
        core/FF7Text.cpp \
        core/FF7Font.cpp \
        core/Config.cpp \
        core/field/TutFile.cpp \
        core/field/TdbFile.cpp \
        core/field/Section1File.cpp \
        core/field/Script.cpp \
        core/field/Palette.cpp \
        core/field/Opcode.cpp \
        core/field/InfFile.cpp \
        core/field/IdFile.cpp \
        core/field/GrpScript.cpp \
        core/field/FieldPS.cpp \
        core/field/FieldPC.cpp \
        core/field/FieldModelPart.cpp \
        core/field/FieldModelLoaderPS.cpp \
        core/field/FieldModelLoaderPC.cpp \
        core/field/FieldModelLoader.cpp \
        core/field/FieldModelFilePS.cpp \
        core/field/FieldModelFilePC.cpp \
        core/field/FieldModelFile.cpp \
        core/field/FieldArchivePS.cpp \
        core/field/FieldArchivePC.cpp \
        core/field/FieldArchiveIO.cpp \
        core/field/FieldArchiveIOPS.cpp \
        core/field/FieldArchiveIOPC.cpp \
        core/field/FieldArchive.cpp \
        core/field/Field.cpp \
        core/field/EncounterFile.cpp \
        core/field/CaFile.cpp \
        core/field/BackgroundFilePS.cpp \
        core/field/BackgroundFilePC.cpp \
        core/field/BackgroundFile.cpp \
        core/field/FieldIO.cpp \
        core/field/TutFilePC.cpp \
        core/field/TutFileStandard.cpp \
        core/field/FieldPart.cpp \
        core/world/TblFile.cpp \
        core/field/PaletteIO.cpp \
        core/field/BackgroundTiles.cpp \
        core/field/BackgroundTilesIO.cpp \
        core/field/BackgroundTextures.cpp \
        core/field/BackgroundTexturesIO.cpp \
        core/field/BackgroundIO.cpp \
        core/Archive.cpp \
        core/field/CharArchive.cpp \
        core/field/FieldPSDemo.cpp \
        core/IO.cpp \
        core/field/FieldModelAnimation.cpp \
        core/field/FieldModelSkeleton.cpp \
        core/field/BcxFile.cpp \
        core/field/BsxFile.cpp \
        core/field/HrcFile.cpp \
        core/field/AFile.cpp \
        core/field/PFile.cpp \
        core/field/RsdFile.cpp \
        core/field/FieldModelTextureRef.cpp \
        core/field/FieldModelTextureRefPC.cpp \
        core/field/FieldModelTextureRefPS.cpp \
        core/AkaoIO.cpp \
        core/Akao.cpp
}
