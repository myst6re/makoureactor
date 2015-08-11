TEMPLATE = app
win32 {
    TARGET = Makou_Reactor
} else {
    TARGET = makoureactor
}

QT += core gui opengl
QMAKE_CXXFLAGS += -std=c++0x

# Input
HEADERS += \
    Window.h \
    Parameters.h \
    Data.h \
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
    widgets/LgpDialog.h \
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
    core/field/DatFile.h \
    core/field/PCFieldFile.h \
    core/LzsIO.h

SOURCES += \
    Window.cpp \
    txt.cpp \
    main.cpp \
    Data.cpp \
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
    widgets/LgpDialog.cpp \
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
    core/field/DatFile.cpp \
    core/field/PCFieldFile.cpp \
    core/LzsIO.cpp

TRANSLATIONS += Makou_Reactor_en.ts \
    Makou_Reactor_ja.ts

RESOURCES += Makou_Reactor.qrc
macx {
    ICON = images/Makou_Reactor.icns
}

win32 {
    RC_FILE = Makou_Reactor.rc

    TASKBAR_BUTTON {
        LIBS += -lole32
        INCLUDEPATH += include
        DEFINES += TASKBAR_BUTTON
    }
    INCLUDEPATH += zlib-1.2.7
    # LIBS += -lz
} else {
    LIBS += -lz
}

OTHER_FILES += Makou_Reactor.rc
DISTFILES += Makou_Reactor.desktop

#all other *nix (except for symbian)
unix:!macx:!symbian {
    LIBS += -lglut -lGLU
    system(lrelease Makou_Reactor.pro) #call lrelease to make the qm files.
}
