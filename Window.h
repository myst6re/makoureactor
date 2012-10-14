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
#include "FieldArchive.h"
#include "VarManager.h"
#include "TextManager.h"
#include "BGDialog.h"
#include "ApercuBG.h"
#include "Search.h"
#include "GrpScriptList.h"
#include "ScriptList.h"
#include "OpcodeList.h"
#include "FieldModel.h"
#include "ModelManager.h"
#include "WalkmeshManager.h"

class Window : public QMainWindow
{
    Q_OBJECT
public:
	explicit Window();
	virtual ~Window();

	void open(const QString &cheminFic, bool isDir=false);
	FieldArchive::Sorting getFieldSorting();
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
	void about();
private slots:
	void emitOpcodeID();

	void openField(bool reload=false);
	void showGrpScripts();
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
	QAction *actionMassExport, *actionImport, *actionClose;
	QAction *actionRun, *actionModels;
	QAction *actionEncounter, *actionTut;
	QAction *actionMisc, *actionJp_txt;
	QMenu *menuLang;

	TextManager *textDialog;
	ModelManager *_modelManager;
	WalkmeshManager *_walkmeshManager;
	BGDialog *_backgroundManager;

	QProgressBar *progression;
	QLabel *authorLbl;
protected:
	void showEvent(QShowEvent *);
	void closeEvent(QCloseEvent *);
	QMenu *createPopupMenu();
};

#endif
