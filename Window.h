/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2012 Arzel Jérôme <myst6re@gmail.com>
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
#ifndef DEF_WINDOW
#define DEF_WINDOW

#include <QtGui>
#include "core/field/FieldArchive.h"
#include "widgets/VarManager.h"
#include "widgets/TextManager.h"
#include "widgets/BGDialog.h"
#include "widgets/ApercuBG.h"
#include "widgets/Search.h"
#include "widgets/GrpScriptList.h"
#include "widgets/ScriptList.h"
#include "widgets/OpcodeList.h"
#include "widgets/FieldModel.h"
#include "widgets/ModelManagerPC.h"
#include "widgets/ModelManagerPS.h"
#include "widgets/TutWidget.h"
#include "widgets/WalkmeshManager.h"
#include "widgets/QTaskBarButton.h"
#include "widgets/LgpDialog.h"
#include "FieldModelThread.h"

class Window : public QMainWindow, FieldArchiveIOObserver
{
    Q_OBJECT
public:
	explicit Window();
	virtual ~Window();

	void open(const QString &cheminFic, bool isDir=false);
	FieldArchive::Sorting getFieldSorting();

	bool observerWasCanceled() const;
	void setObserverMaximum(unsigned int max);
	void setObserverValue(int value);

public slots:
	void openFile();
	void openDir();
	void refresh();

	void setModified(bool enabled=true);
	void saveAs(bool currentPath=false);
	void save();
	int closeFile(bool quit=false);

	bool gotoField(int fieldID);
	void gotoOpcode(int fieldID, int grpScriptID, int scriptID, int opcodeID);
	void gotoText(int fieldID, int textID, int from, int size);

	void jpText(bool);

	//	void notifyFileChanged(const QString &path);
	//	void notifyDirectoryChanged(const QString &path);
	void exporter();
	void massExport();
	void massImport();
	void importer();
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
	void fontManager();
	void about();
private slots:
	void emitOpcodeID();

	void openField(bool reload=false);
	void showGrpScripts();
	void showModel(Field *field, FieldModelFile *fieldModelFile);
	void showScripts();
	void compile();
	void filterMap();
	void changeLanguage(QAction *);
	void config();
signals:
	void fieldIDChanged(int);
	void grpScriptIDChanged(int);
	void scriptIDChanged(int);
	void opcodeIDChanged(int);
private:
	void setWindowTitle();
	void restartNow();
	void showProgression(const QString &message, bool canBeCanceled);
	void hideProgression();

	QLineEdit *lineSearch;
	QTreeWidget *fieldList;
	GrpScriptList *groupScriptList;
	ScriptList *scriptList;
	OpcodeList *opcodeList;
	QLabel *compileScriptIcon, *compileScriptLabel;
	QStackedWidget *zonePreview;
	ApercuBG *zoneImage;
	FieldModel *fieldModel;
	QGridLayout *gridLayout;

	QToolBar *toolBar;

	FieldArchive *fieldArchive;
	Field *field;
	bool firstShow;

	Search *searchDialog;
	VarManager *varDialog;

	QAction *actionSave, *actionSaveAs, *actionExport;
	QAction *actionMassExport, *actionImport, *actionMassImport, *actionClose;
	QAction *actionRun, *actionModels, *actionArchive;
	QAction *actionEncounter;
	QAction *actionMisc, *actionJp_txt;
	QMenu *menuLang;

	TextManager *textDialog;
	ModelManager *_modelManager;
	TutWidget *_tutManager;
	WalkmeshManager *_walkmeshManager;
	BGDialog *_backgroundManager;

	QTaskBarButton *taskBarButton;
	QProgressDialog *progressDialog;
	QAction *authorAction;
	QLabel *authorLbl;

	FieldModelThread *modelThread;
protected:
	void showEvent(QShowEvent *);
	void closeEvent(QCloseEvent *);
	QMenu *createPopupMenu();
};

#endif
