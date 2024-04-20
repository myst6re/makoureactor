###############################################################################
## Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
## Copyright (C) 2020 Julian Xhokaxhiu <https://julianxhokaxhiu.com>
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.
###############################################################################

cmake_minimum_required(VERSION 3.24)
cmake_policy(SET CMP0010 NEW)
cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0091 NEW)

set(VCPKG_DISABLE_COMPILER_TRACKING 1)
set(VCPKG_INSTALL_OPTIONS "--clean-after-build" "--x-use-aria2")

set(RELEASE_NAME "Makou_Reactor")
set(GUI_TARGET "${RELEASE_NAME}")
set(CLI_TARGET "makoureactor")
if(NOT PRERELEASE_STRING)
    set(PRERELEASE_STRING "")
endif()

project("Makou Reactor" VERSION 2.1.0 LANGUAGES CXX
    DESCRIPTION "Final Fantasy VII field archive editor"
    HOMEPAGE_URL "https://github.com/myst6re/makoureactor"
)

# Try to autodetect Qt path if not specified
include(qt.cmake)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/W3)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(GUI "Build the gui executable" ON)
option(CLI "Build the cli executable" OFF)
option(BUNDLE_FF7TK_QM "Include FF7tk QM Files in bundle" ON)

add_compile_definitions(
    QT_DISABLE_DEPRECATED_BEFORE=0x050F00
    #QT_RESTRICTED_CAST_FROM_ASCII
    MAKOU_REACTOR_NAME="${PROJECT_NAME}"
    MAKOU_REACTOR_DESCRIPTION="${PROJECT_DESCRIPTION}"
    MAKOU_REACTOR_COMPANY="myst6re"
    MAKOU_REACTOR_VERSION="${PROJECT_VERSION}${PRERELEASE_STRING}"
    MAKOU_REACTOR_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    MAKOU_REACTOR_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    MAKOU_REACTOR_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    MAKOU_REACTOR_VERSION_TWEAK=0
)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets OpenGLWidgets Svg OpenGL LinguistTools Core5Compat Xml Quick REQUIRED)

set(REQUIRED_FF7TK_VERSION 1.0.0)
find_package(ff7tk ${REQUIRED_FF7TK_VERSION} CONFIG REQUIRED COMPONENTS ff7tk fftkData ff7tkQtWidgets ff7tkFormats ff7tkUtils)

find_package(ZLIB REQUIRED)

set(LANGS
    fr
    ja)
list(TRANSFORM LANGS REPLACE ".+" "translations/${RELEASE_NAME}_\\0.ts" OUTPUT_VARIABLE TS_FILES)

set(PROJECT_SOURCES
    "src/3d/FieldModel.cpp"
    "src/3d/FieldModel.h"
    "src/3d/Renderer.cpp"
    "src/3d/Renderer.h"
    "src/3d/WalkmeshWidget.cpp"
    "src/3d/WalkmeshWidget.h"
    "src/Data.cpp"
    "src/Data.h"
    #"src/FieldModelThread.cpp"
    #"src/FieldModelThread.h"
    "src/Window.cpp"
    "src/Window.h"
    "src/core/Clipboard.cpp"
    "src/core/Clipboard.h"
    "src/core/Config.cpp"
    "src/core/Config.h"
    "src/core/FF7Font.cpp"
    "src/core/FF7Font.h"
    "src/core/SystemColor.cpp"
    "src/core/SystemColor.h"
    "src/core/Var.cpp"
    "src/core/Var.h"
    "src/core/field/AFile.cpp"
    "src/core/field/AFile.h"
    "src/core/field/BackgroundFile.cpp"
    "src/core/field/BackgroundFile.h"
    "src/core/field/BackgroundFilePC.cpp"
    "src/core/field/BackgroundFilePC.h"
    "src/core/field/BackgroundFilePS.cpp"
    "src/core/field/BackgroundFilePS.h"
    "src/core/field/BackgroundIO.cpp"
    "src/core/field/BackgroundIO.h"
    "src/core/field/BackgroundTextures.cpp"
    "src/core/field/BackgroundTextures.h"
    "src/core/field/BackgroundTexturesIO.cpp"
    "src/core/field/BackgroundTexturesIO.h"
    "src/core/field/BackgroundTiles.cpp"
    "src/core/field/BackgroundTiles.h"
    "src/core/field/BackgroundTilesFile.cpp"
    "src/core/field/BackgroundTilesFile.h"
    "src/core/field/BackgroundTilesIO.cpp"
    "src/core/field/BackgroundTilesIO.h"
    "src/core/field/BcxFile.cpp"
    "src/core/field/BcxFile.h"
    "src/core/field/BsxFile.cpp"
    "src/core/field/BsxFile.h"
    "src/core/field/CaFile.cpp"
    "src/core/field/CaFile.h"
    "src/core/field/CharArchive.cpp"
    "src/core/field/CharArchive.h"
    "src/core/field/EncounterFile.cpp"
    "src/core/field/EncounterFile.h"
    "src/core/field/Field.cpp"
    "src/core/field/Field.h"
    "src/core/field/FieldArchive.cpp"
    "src/core/field/FieldArchive.h"
    "src/core/field/FieldArchiveIO.cpp"
    "src/core/field/FieldArchiveIO.h"
    "src/core/field/FieldArchiveIOPC.cpp"
    "src/core/field/FieldArchiveIOPC.h"
    "src/core/field/FieldArchiveIOPS.cpp"
    "src/core/field/FieldArchiveIOPS.h"
    "src/core/field/FieldArchivePC.cpp"
    "src/core/field/FieldArchivePC.h"
    "src/core/field/FieldArchivePS.cpp"
    "src/core/field/FieldArchivePS.h"
    "src/core/field/FieldIO.cpp"
    "src/core/field/FieldIO.h"
    "src/core/field/FieldModelAnimation.cpp"
    "src/core/field/FieldModelAnimation.h"
    "src/core/field/FieldModelFile.cpp"
    "src/core/field/FieldModelFile.h"
    "src/core/field/FieldModelFilePC.cpp"
    "src/core/field/FieldModelFilePC.h"
    "src/core/field/FieldModelFilePS.cpp"
    "src/core/field/FieldModelFilePS.h"
    "src/core/field/FieldModelLoader.cpp"
    "src/core/field/FieldModelLoader.h"
    "src/core/field/FieldModelLoaderPC.cpp"
    "src/core/field/FieldModelLoaderPC.h"
    "src/core/field/FieldModelLoaderPS.cpp"
    "src/core/field/FieldModelLoaderPS.h"
    "src/core/field/FieldModelPart.cpp"
    "src/core/field/FieldModelPart.h"
    "src/core/field/FieldModelSkeleton.cpp"
    "src/core/field/FieldModelSkeleton.h"
    "src/core/field/FieldModelTextureRef.cpp"
    "src/core/field/FieldModelTextureRef.h"
    "src/core/field/FieldModelTextureRefPC.cpp"
    "src/core/field/FieldModelTextureRefPC.h"
    "src/core/field/FieldModelTextureRefPS.cpp"
    "src/core/field/FieldModelTextureRefPS.h"
    "src/core/field/FieldPC.cpp"
    "src/core/field/FieldPC.h"
    "src/core/field/FieldPS.cpp"
    "src/core/field/FieldPS.h"
    "src/core/field/FieldPSDemo.cpp"
    "src/core/field/FieldPSDemo.h"
    "src/core/field/FieldPart.cpp"
    "src/core/field/FieldPart.h"
    "src/core/field/GrpScript.cpp"
    "src/core/field/GrpScript.h"
    "src/core/field/HrcFile.cpp"
    "src/core/field/HrcFile.h"
    "src/core/field/IdFile.cpp"
    "src/core/field/IdFile.h"
    "src/core/field/InfFile.cpp"
    "src/core/field/InfFile.h"
    "src/core/field/MapList.cpp"
    "src/core/field/MapList.h"
    "src/core/field/Opcode.cpp"
    "src/core/field/Opcode.h"
    "src/core/field/PFile.cpp"
    "src/core/field/PFile.h"
    "src/core/field/Palette.cpp"
    "src/core/field/Palette.h"
    "src/core/field/PaletteIO.cpp"
    "src/core/field/PaletteIO.h"
    "src/core/field/RsdFile.cpp"
    "src/core/field/RsdFile.h"
    "src/core/field/Script.cpp"
    "src/core/field/Script.h"
    "src/core/field/Section1File.cpp"
    "src/core/field/Section1File.h"
    "src/core/field/TdbFile.cpp"
    "src/core/field/TdbFile.h"
    "src/core/field/TutFile.cpp"
    "src/core/field/TutFile.h"
    "src/core/field/TutFilePC.cpp"
    "src/core/field/TutFilePC.h"
    "src/core/field/TutFileStandard.cpp"
    "src/core/field/TutFileStandard.h"
    "src/main.cpp"
    "src/widgets/AboutDialog.cpp"
    "src/widgets/AboutDialog.h"
    "src/widgets/AnimEditorDialog.cpp"
    "src/widgets/AnimEditorDialog.h"
    "src/widgets/ApercuBG.cpp"
    "src/widgets/ApercuBG.h"
    "src/widgets/ApercuBGLabel.cpp"
    "src/widgets/ApercuBGLabel.h"
    "src/widgets/ArchivePreview.cpp"
    "src/widgets/ArchivePreview.h"
    "src/widgets/BackgroundEditor.cpp"
    "src/widgets/BackgroundEditor.h"
    "src/widgets/BackgroundPaletteEditor.cpp"
    "src/widgets/BackgroundPaletteEditor.h"
    "src/widgets/BackgroundTileEditor.cpp"
    "src/widgets/BackgroundTileEditor.h"
    "src/widgets/BGDialog.cpp"
    "src/widgets/BGDialog.h"
    "src/widgets/ConfigWindow.cpp"
    "src/widgets/ConfigWindow.h"
    "src/widgets/EmptyFieldWidget.cpp"
    "src/widgets/EmptyFieldWidget.h"
    "src/widgets/EncounterWidget.cpp"
    "src/widgets/EncounterWidget.h"
    "src/widgets/FieldList.cpp"
    "src/widgets/FieldList.h"
    "src/widgets/FontDisplay.cpp"
    "src/widgets/FontDisplay.h"
    "src/widgets/FontGrid.cpp"
    "src/widgets/FontGrid.h"
    "src/widgets/FontLetter.cpp"
    "src/widgets/FontLetter.h"
    "src/widgets/FontManager.cpp"
    "src/widgets/FontManager.h"
    "src/widgets/FontPalette.cpp"
    "src/widgets/FontPalette.h"
    "src/widgets/FontWidget.cpp"
    "src/widgets/FontWidget.h"
    "src/widgets/FormatSelectionWidget.cpp"
    "src/widgets/FormatSelectionWidget.h"
    "src/widgets/GrpScriptList.cpp"
    "src/widgets/GrpScriptList.h"
    #"src/widgets/GrpScriptWizard.cpp"
    #"src/widgets/GrpScriptWizard.h"
    "src/widgets/HelpWidget.cpp"
    "src/widgets/HelpWidget.h"
    "src/widgets/ImportDialog.cpp"
    "src/widgets/ImportDialog.h"
    "src/widgets/KeyEditorDialog.cpp"
    "src/widgets/KeyEditorDialog.h"
    "src/widgets/LgpWidget.cpp"
    "src/widgets/LgpWidget.h"
    "src/widgets/MassExportDialog.cpp"
    "src/widgets/MassExportDialog.h"
    "src/widgets/MassImportDialog.cpp"
    "src/widgets/MassImportDialog.h"
    "src/widgets/MiscWidget.cpp"
    "src/widgets/MiscWidget.h"
    "src/widgets/ModelColorsLayout.cpp"
    "src/widgets/ModelColorsLayout.h"
    "src/widgets/ModelManager.cpp"
    "src/widgets/ModelManager.h"
    "src/widgets/ModelManagerPC.cpp"
    "src/widgets/ModelManagerPC.h"
    "src/widgets/ModelManagerPS.cpp"
    "src/widgets/ModelManagerPS.h"
    "src/widgets/OpcodeList.cpp"
    "src/widgets/OpcodeList.h"
    "src/widgets/OperationsManager.cpp"
    "src/widgets/OperationsManager.h"
    "src/widgets/PsColorDialog.cpp"
    "src/widgets/PsColorDialog.h"
    "src/widgets/PsfDialog.cpp"
    "src/widgets/PsfDialog.h"
    "src/widgets/QColorPicker.cpp"
    "src/widgets/QColorPicker.h"
    "src/widgets/ScriptEditor.cpp"
    "src/widgets/ScriptEditor.h"
    "src/widgets/ScriptEditorWidgets/Delegate.cpp"
    "src/widgets/ScriptEditorWidgets/Delegate.h"
    "src/widgets/ScriptEditorWidgets/ScriptEditorGenericList.cpp"
    "src/widgets/ScriptEditorWidgets/ScriptEditorGenericList.h"
    "src/widgets/ScriptEditorWidgets/ScriptEditorMathPage.cpp"
    "src/widgets/ScriptEditorWidgets/ScriptEditorMathPage.h"
    "src/widgets/ScriptEditorWidgets/ScriptEditorModelPage.cpp"
    "src/widgets/ScriptEditorWidgets/ScriptEditorModelPage.h"
    "src/widgets/ScriptEditorWidgets/ScriptEditorMoviePage.cpp"
    "src/widgets/ScriptEditorWidgets/ScriptEditorMoviePage.h"
    "src/widgets/ScriptEditorWidgets/ScriptEditorSpecialPage.cpp"
    "src/widgets/ScriptEditorWidgets/ScriptEditorSpecialPage.h"
    "src/widgets/ScriptEditorWidgets/ScriptEditorStructPage.cpp"
    "src/widgets/ScriptEditorWidgets/ScriptEditorStructPage.h"
    "src/widgets/ScriptEditorWidgets/ScriptEditorView.cpp"
    "src/widgets/ScriptEditorWidgets/ScriptEditorView.h"
    "src/widgets/ScriptEditorWidgets/ScriptEditorWalkmeshPage.cpp"
    "src/widgets/ScriptEditorWidgets/ScriptEditorWalkmeshPage.h"
    "src/widgets/ScriptEditorWidgets/ScriptEditorWindowPage.cpp"
    "src/widgets/ScriptEditorWidgets/ScriptEditorWindowPage.h"
    "src/widgets/ScriptList.cpp"
    "src/widgets/ScriptList.h"
    "src/widgets/ScriptManager.cpp"
    "src/widgets/ScriptManager.h"
    "src/widgets/Search.cpp"
    "src/widgets/Search.h"
    "src/widgets/SearchAll.cpp"
    "src/widgets/SearchAll.h"
    "src/widgets/TextHighlighter.cpp"
    "src/widgets/TextHighlighter.h"
    "src/widgets/TextManager.cpp"
    "src/widgets/TextManager.h"
    "src/widgets/TextPreview.cpp"
    "src/widgets/TextPreview.h"
    "src/widgets/TutWidget.cpp"
    "src/widgets/TutWidget.h"
    "src/widgets/VarManager.cpp"
    "src/widgets/VarManager.h"
    "src/widgets/VarOrValueWidget.cpp"
    "src/widgets/VarOrValueWidget.h"
    "src/widgets/VertexWidget.cpp"
    "src/widgets/VertexWidget.h"
    "src/widgets/WalkmeshManager.cpp"
    "src/widgets/WalkmeshManager.h"
    "src/widgets/ExportChunksDialog.cpp"
    "src/widgets/ExportChunksDialog.h"
)

if(APPLE)
    list(APPEND PROJECT_SOURCES
        "src/core/SystemColor.mm"
    )
endif()

set(PROJECT_CLI_SOURCES
    "src/Arguments.cpp"
    "src/Arguments.h"
    "src/ArgumentsExport.cpp"
    "src/ArgumentsExport.h"
    "src/ArgumentsPatch.cpp"
    "src/ArgumentsPatch.h"
    "src/ArgumentsTools.cpp"
    "src/ArgumentsTools.h"
    "src/CLI.cpp"
    "src/CLI.h"
    "src/Data.cpp"
    "src/Data.h"
    "src/core/Config.cpp"
    "src/core/Config.h"
    "src/core/FF7Font.cpp"
    "src/core/FF7Font.h"
    "src/core/SystemColor.cpp"
    "src/core/SystemColor.h"
    "src/core/Var.cpp"
    "src/core/Var.h"
    "src/core/field/AFile.cpp"
    "src/core/field/AFile.h"
    "src/core/field/BackgroundFile.cpp"
    "src/core/field/BackgroundFile.h"
    "src/core/field/BackgroundFilePC.cpp"
    "src/core/field/BackgroundFilePC.h"
    "src/core/field/BackgroundFilePS.cpp"
    "src/core/field/BackgroundFilePS.h"
    "src/core/field/BackgroundIO.cpp"
    "src/core/field/BackgroundIO.h"
    "src/core/field/BackgroundTextures.cpp"
    "src/core/field/BackgroundTextures.h"
    "src/core/field/BackgroundTexturesIO.cpp"
    "src/core/field/BackgroundTexturesIO.h"
    "src/core/field/BackgroundTiles.cpp"
    "src/core/field/BackgroundTiles.h"
    "src/core/field/BackgroundTilesFile.cpp"
    "src/core/field/BackgroundTilesFile.h"
    "src/core/field/BackgroundTilesIO.cpp"
    "src/core/field/BackgroundTilesIO.h"
    "src/core/field/BcxFile.cpp"
    "src/core/field/BcxFile.h"
    "src/core/field/BsxFile.cpp"
    "src/core/field/BsxFile.h"
    "src/core/field/CaFile.cpp"
    "src/core/field/CaFile.h"
    "src/core/field/CharArchive.cpp"
    "src/core/field/CharArchive.h"
    "src/core/field/EncounterFile.cpp"
    "src/core/field/EncounterFile.h"
    "src/core/field/Field.cpp"
    "src/core/field/Field.h"
    "src/core/field/FieldArchive.cpp"
    "src/core/field/FieldArchive.h"
    "src/core/field/FieldArchiveIO.cpp"
    "src/core/field/FieldArchiveIO.h"
    "src/core/field/FieldArchiveIOPC.cpp"
    "src/core/field/FieldArchiveIOPC.h"
    "src/core/field/FieldArchiveIOPS.cpp"
    "src/core/field/FieldArchiveIOPS.h"
    "src/core/field/FieldArchivePC.cpp"
    "src/core/field/FieldArchivePC.h"
    "src/core/field/FieldArchivePS.cpp"
    "src/core/field/FieldArchivePS.h"
    "src/core/field/FieldIO.cpp"
    "src/core/field/FieldIO.h"
    "src/core/field/FieldModelAnimation.cpp"
    "src/core/field/FieldModelAnimation.h"
    "src/core/field/FieldModelFile.cpp"
    "src/core/field/FieldModelFile.h"
    "src/core/field/FieldModelFilePC.cpp"
    "src/core/field/FieldModelFilePC.h"
    "src/core/field/FieldModelFilePS.cpp"
    "src/core/field/FieldModelFilePS.h"
    "src/core/field/FieldModelLoader.cpp"
    "src/core/field/FieldModelLoader.h"
    "src/core/field/FieldModelLoaderPC.cpp"
    "src/core/field/FieldModelLoaderPC.h"
    "src/core/field/FieldModelLoaderPS.cpp"
    "src/core/field/FieldModelLoaderPS.h"
    "src/core/field/FieldModelPart.cpp"
    "src/core/field/FieldModelPart.h"
    "src/core/field/FieldModelSkeleton.cpp"
    "src/core/field/FieldModelSkeleton.h"
    "src/core/field/FieldModelTextureRef.cpp"
    "src/core/field/FieldModelTextureRef.h"
    "src/core/field/FieldModelTextureRefPC.cpp"
    "src/core/field/FieldModelTextureRefPC.h"
    "src/core/field/FieldModelTextureRefPS.cpp"
    "src/core/field/FieldModelTextureRefPS.h"
    "src/core/field/FieldPC.cpp"
    "src/core/field/FieldPC.h"
    "src/core/field/FieldPS.cpp"
    "src/core/field/FieldPS.h"
    "src/core/field/FieldPSDemo.cpp"
    "src/core/field/FieldPSDemo.h"
    "src/core/field/FieldPart.cpp"
    "src/core/field/FieldPart.h"
    "src/core/field/GrpScript.cpp"
    "src/core/field/GrpScript.h"
    "src/core/field/HrcFile.cpp"
    "src/core/field/HrcFile.h"
    "src/core/field/IdFile.cpp"
    "src/core/field/IdFile.h"
    "src/core/field/InfFile.cpp"
    "src/core/field/InfFile.h"
    "src/core/field/MapList.cpp"
    "src/core/field/MapList.h"
    "src/core/field/Opcode.cpp"
    "src/core/field/Opcode.h"
    "src/core/field/PFile.cpp"
    "src/core/field/PFile.h"
    "src/core/field/Palette.cpp"
    "src/core/field/Palette.h"
    "src/core/field/PaletteIO.cpp"
    "src/core/field/PaletteIO.h"
    "src/core/field/RsdFile.cpp"
    "src/core/field/RsdFile.h"
    "src/core/field/Script.cpp"
    "src/core/field/Script.h"
    "src/core/field/Section1File.cpp"
    "src/core/field/Section1File.h"
    "src/core/field/TdbFile.cpp"
    "src/core/field/TdbFile.h"
    "src/core/field/TutFile.cpp"
    "src/core/field/TutFile.h"
    "src/core/field/TutFilePC.cpp"
    "src/core/field/TutFilePC.h"
    "src/core/field/TutFileStandard.cpp"
    "src/core/field/TutFileStandard.h"
    "src/main.cpp"
)

if(APPLE)
    list(APPEND PROJECT_CLI_SOURCES
        "src/core/SystemColor.mm"
    )
endif()

set(RESOURCES "src/qt/${RELEASE_NAME}.qrc" "icons/icons.qrc")

if(APPLE)
    set(ICON_FILE "deploy/macosx/${RELEASE_NAME}.icns")
    set(EXTRA_RESOURCES_GUI ${ICON_FILE})
    set(EXTRA_RESOURCES_CLI "")

    set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
elseif (WIN32)
    set(ICON_FILE "src/qt/images/logo-shinra.ico")
    set(EXTRA_RESOURCES_GUI "src/qt/${RELEASE_NAME}.rc")
    set(EXTRA_RESOURCES_CLI "${EXTRA_RESOURCES_GUI}")
endif()

function(add_translations target)
    set(oneValueArgs QM_FILES_OUTPUT_VARIABLE)
    set(multiValueArgs TS_FILES)
    cmake_parse_arguments(arg "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    set(qm_files "")
    qt_add_translations(${target}
        TS_FILES ${TS_FILES}
        LUPDATE_OPTIONS -locations none
        QM_FILES_OUTPUT_VARIABLE qm_files)
    set("${arg_QM_FILES_OUTPUT_VARIABLE}" "${qm_files}" PARENT_SCOPE)
    # Force lupdate
    add_dependencies(${target}_lrelease ${target}_lupdate)
endfunction()

# GUI
if(GUI)
    qt_add_executable(${GUI_TARGET} MANUAL_FINALIZATION MACOSX_BUNDLE WIN32 ${PROJECT_SOURCES} ${RESOURCES} ${EXTRA_RESOURCES_GUI})
    target_include_directories(${GUI_TARGET} PRIVATE "${CMAKE_SOURCE_DIR}/src")
    target_link_libraries(${GUI_TARGET} PRIVATE
        Qt::OpenGL
        Qt::Widgets
        Qt::OpenGLWidgets
        Qt::Svg
        ZLIB::ZLIB
        ff7tk::ff7tk
        ff7tk::ff7tkData
        ff7tk::ff7tkQtWidgets
        ff7tk::ff7tkFormats
        ff7tk::ff7tkUtils
    )

    if(${QT_VERSION_MAJOR} EQUAL 6)
        target_link_libraries(${GUI_TARGET} PRIVATE Qt6::Core5Compat)

        target_compile_definitions(${GUI_TARGET}
            PRIVATE TASKBAR_BUTTON=1
            GUI=1
        )
    endif()

    set_target_properties(${GUI_TARGET} PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "Makou Reactor"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        MACOSX_BUNDLE_GUI_IDENTIFIER org.myst6re.makoureactor
        MACOSX_BUNDLE_ICON_FILE ${RELEASE_NAME}.icns
    )

    if(APPLE OR WIN32)
        set_target_properties(${GUI_TARGET} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")
    endif()

    add_translations(${GUI_TARGET}
        TS_FILES ${TS_FILES}
        QM_FILES_OUTPUT_VARIABLE QM_FILES)
endif()

# CLI
if(CLI)
    qt_add_executable(${CLI_TARGET} MANUAL_FINALIZATION ${PROJECT_CLI_SOURCES} ${RESOURCES} ${EXTRA_RESOURCES_CLI})
    target_include_directories(${CLI_TARGET} PRIVATE "${CMAKE_SOURCE_DIR}/src")
    target_link_libraries(${CLI_TARGET} PRIVATE
        ZLIB::ZLIB
        ff7tk::ff7tk
        ff7tk::ff7tkData
        ff7tk::ff7tkFormats
        ff7tk::ff7tkUtils
    )
    target_compile_definitions(${CLI_TARGET}
        PRIVATE MR_CONSOLE=1 QT_NO_DEBUG_OUTPUT=1
    )
endif()

include(GNUInstallDirs)

if(APPLE)
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX "/Applications" CACHE PATH "..." FORCE)
    endif()
    if(NOT QT_DEPLOY_TMP_DIR)
        set(QT_DEPLOY_TMP_DIR "${CMAKE_BINARY_DIR}" CACHE PATH "Directory to run deployqt scripts")
    endif()
elseif(WIN32)
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT AND ENV{ProgramFiles})
        # Force Program Files for x64 programs (CMake uses x86 by default)
        set(CMAKE_INSTALL_PREFIX "$ENV{ProgramFiles}/${PROJECT_NAME}" CACHE PATH "..." FORCE)
    endif()
    if(NOT QT_DEPLOY_TMP_DIR)
        set(QT_DEPLOY_TMP_DIR "${CMAKE_BINARY_DIR}/win32-deploy" CACHE PATH "Directory to run deployqt scripts")
    endif()
    if(GUI)
        install(TARGETS ${GUI_TARGET} RUNTIME DESTINATION "${QT_DEPLOY_TMP_DIR}")
    endif()
    if(CLI)
        install(TARGETS ${CLI_TARGET} RUNTIME DESTINATION "${QT_DEPLOY_TMP_DIR}")
    endif()
endif()

add_subdirectory(deploy)

if(APPLE)
    if(GUI)
        install(TARGETS ${GUI_TARGET} BUNDLE DESTINATION ".")
    endif()
    if(CLI)
        install(TARGETS ${CLI_TARGET} RUNTIME EXCLUDE_FROM_ALL)
    endif()
elseif(WIN32)
    install(DIRECTORY "${QT_DEPLOY_TMP_DIR}/" DESTINATION ".")
    install(CODE "file(REMOVE_RECURSE \"${QT_DEPLOY_TMP_DIR}\")")
else()
    install(FILES ${CMAKE_SOURCE_DIR}/src/qt/images/logo-shinra.png DESTINATION share/icons/hicolor/256x256/apps RENAME io.github.myst6re.makoureactor.png)
    install(FILES ${CMAKE_SOURCE_DIR}/deploy/linux/io.github.myst6re.makoureactor.desktop DESTINATION share/applications)
    install(FILES ${CMAKE_SOURCE_DIR}/deploy/linux/io.github.myst6re.makoureactor.metainfo.xml DESTINATION share/metainfo)
    install(FILES ${QM_FILES} DESTINATION share/makoureactor/translations)
    if(GUI)
        install(TARGETS ${GUI_TARGET} RUNTIME)
    endif()
    if(CLI)
        install(TARGETS ${CLI_TARGET} RUNTIME)
    endif()
endif()

if(GUI)
    qt_finalize_executable(${GUI_TARGET})
endif()
if(CLI)
    qt_finalize_executable(${CLI_TARGET})
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(CPACK_SYSTEM_NAME "macos")
    set(CPACK_GENERATOR "DragNDrop")
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CPACK_GENERATOR "ZIP")
else()
    set(CPACK_GENERATOR "TXZ")
endif()

set(CPACK_DEBIAN_PACKAGE_NAME "makoureactor")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "myst6re@gmail.com")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libff7tk (>= ${REQUIRED_FF7TK_VERSION}) | libff7tk-all (>=${REQUIRED_FF7TK_VERSION}), qt6-qpa-plugins")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

set(CPACK_PACKAGE_NAME makoureactor)
set(CPACK_SOURCE_IGNORE_FILES ".git/*;*.user;*.yml;*.github/*")
set(CPACK_SOURCE_GENERATOR "ZIP;TGZ")

include(CPack)
