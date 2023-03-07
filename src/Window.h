/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#pragma once

#include <QtWidgets>
#include <QTaskBarButton>
#include "core/field/FieldArchive.h"
#include "widgets/FieldList.h"
#include "widgets/ScriptManager.h"
#include "widgets/VarManager.h"
#include "widgets/TextManager.h"
#include "widgets/BGDialog.h"
#include "widgets/ApercuBG.h"
#include "widgets/Search.h"
#include "3d/FieldModel.h"
#include "widgets/ModelManagerPC.h"
#include "widgets/ModelManagerPS.h"
#include "widgets/TutWidget.h"
#include "widgets/WalkmeshManager.h"
#include "widgets/LgpWidget.h"
//#include "FieldModelThread.h"

#include <Splitter.h>

class Window : public QMainWindow, ArchiveObserver
{
    Q_OBJECT
public:
	explicit Window();
	virtual ~Window() override;

	void open(const QString &cheminFic, FieldArchiveIO::Type type, bool isPS);
	FieldArchive::Sorting getFieldSorting();

	bool observerWasCanceled() const override;
	void setObserverMaximum(unsigned int max) override;
	void setObserverValue(int value) override;
	bool observerRetry(const QString &message) override;

	inline FieldList *fieldList() const {
		return _fieldList;
	}
	inline TextManager *textWidget() const {
		return _textDialog;
	}
	inline ScriptManager *scriptWidget() const {
		return _scriptManager;
	}

public slots:
	void openFile(const QString &path=QString());
	void openDir();

	void setModified(bool enabled=true);
	void setFieldDeleted();
	void saveAs(bool currentPath=false);
	void save();
	int closeFile(bool quit=false);

	bool gotoField(int mapID);
	void gotoOpcode(int mapID, int grpScriptID, int scriptID, int opcodeID);
	void gotoText(int mapID, int textID, qsizetype from, qsizetype size);

	void jpText(bool);

	//	void notifyFileChanged(const QString &path);
	//	void notifyDirectoryChanged(const QString &path);
	void exportCurrentMap();
	void exportCurrentMapIntoChunks();
	void massExport();
	void massImport();
	void importToCurrentMap();
	void varManager();
	void runFF7();
	void textManager(int textID=-1, int from=0, int size=0, bool activate=true);
	void modelManager();
	void encounterManager();
	void tutManager();
	void walkmeshManager();
	void backgroundManager();
	void miscManager();
	void searchManager();
	void archiveManager();
	void miscOperations();
	void about();
	void setEditorPageIndex(int index);
private slots:
	void openRecentFile(QAction *action);
	void disableEditors();
	void openField(bool reload=false);
	void showModel(int grpScriptID);
	void showModel(Field *field, FieldModelFile *fieldModelFile);
	void changeLanguage(QAction *);
	void toggleFieldList();
	void toggleBackgroundPreview();
	void config();
	void searchOpcode(int opcodeID);
	void createCurrentMap();
	void processEvents() const;
private:
	void setWindowTitle();
	void restartNow();
	void showProgression(const QString &message, bool canBeCanceled);
	void hideProgression();
	QProgressDialog *progressDialog();
	void fillRecentMenu();

	QTabBar *_tabBar;
	QLineEdit *lineSearch;
	FieldList *_fieldList;
	QStackedWidget *zonePreview;
	ApercuBG *zoneImage;
	FieldModel *fieldModel;
	Splitter *horizontalSplitter, *verticalSplitter;

	QToolBar *toolBar;

	FieldArchive *fieldArchive;
	Field *field;
	bool firstShow;

	Search *searchDialog;
	VarManager *varDialog;

	QMenu *_recentMenu;
	QAction *actionSave, *actionSaveAs, *actionExport, *actionChunks;
	QAction *actionMassExport, *actionImport, *actionMassImport, *actionClose;
	QAction *actionRun, *actionModels;
	QAction *actionEncounter;
	QAction *actionMisc, *actionMiscOperations, *actionJp_txt;
	QMenu *menuLang;

	ScriptManager *_scriptManager;
	TextManager *_textDialog;
	ModelManager *_modelManager;
	TutWidget *_tutManager;
	WalkmeshManager *_walkmeshManager;
	BGDialog *_backgroundManager;

	QStackedWidget *_mainStackedWidget, *_fieldStackedWidget;
	LgpWidget *_lgpWidget;

	QTaskBarButton *taskBarButton;
	QProgressDialog *_progressDialog;
	QLabel *authorLbl;
	QTimer timer;

//	FieldModelThread *modelThread;
protected:
	void closeEvent(QCloseEvent *event) override;
	QMenu *createPopupMenu() override;
};
