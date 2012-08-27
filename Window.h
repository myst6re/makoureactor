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

#define DIR_UP		0
#define DIR_DOWN	1

#include <QtGui>
#include "FieldArchive.h"
#include "GrpScript.h"
#include "VarManager.h"
#include "TextManager.h"
#include "ApercuBG.h"
#include "Search.h"
#include "GrpScriptList.h"
#include "ScriptList.h"
#include "OpcodeList.h"
#include "ModelManager.h"
#include "FieldModel.h"
#include "WalkmeshManager.h"
#include "ConfigWindow.h"
#include "EncounterWidget.h"
#include "TutWidget.h"
#include "MiscWidget.h"
#include "ImportDialog.h"

class Window : public QMainWindow
{
    Q_OBJECT
public:
	explicit Window();
	virtual ~Window();

	void ouvrir(const QString &cheminFic, bool isDir=false);
	FieldArchive::Sorting getFieldSorting();
public slots:
	void ouvrirFichier();
	void ouvrirDossier();
	void refresh();

	void activerSave(bool enabled=true);
	void enregistrerSous(bool currentPath=false);
	void enregistrer();
	int fermer(bool quit=false);

	bool gotoField(int fieldID);
	void gotoOpcode(int fieldID, int grpScriptID, int scriptID, int opcodeID);
	void gotoText(int fieldID, int textID, int from, int size);

	void jp_txt(bool);

	//	void notifyFileChanged(const QString &path);
	//	void notifyDirectoryChanged(const QString &path);
	void exporter();
	void massExport();
	void importer();
	void varManager();
	void runFF7();
	void textManager(bool activate=true);
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
	void changeHistoric(const Historic &);
	void undo();
	void redo();

	void ouvrirField();
	void afficherGrpScripts();
	void afficherScripts();
	void filterMap();
	void change_language(QAction *);
	void config();
signals:
	void fieldIDChanged(int);
	void grpScriptIDChanged(int);
	void scriptIDChanged(int);
	void opcodeIDChanged(int);
private:
	QLineEdit *lineSearch;
	QTreeWidget *liste;
	GrpScriptList *liste2;
	ScriptList *liste3;
	OpcodeList *zoneScript;
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

	QAction *actionSave;
	QAction *actionSaveAs;
	QAction *actionExport;
	QAction *actionMassExport;
	QAction *actionImport;
	QAction *actionClose;
	QAction *actionFind;
	QAction *actionText;
	QAction *actionModels;
	QAction *actionEncounter;
	QAction *actionTut;
	QAction *actionWalkmesh;
	QAction *actionBackground;
	QAction *actionMisc;
	QAction *actionJp_txt;
	QAction *actionUndo;
	QAction *actionRedo;
	QMenu *menuLang;

	TextManager *textDialog;
	WalkmeshManager *_walkmeshManager;
	BGDialog *_backgroundManager;

	QProgressBar *progression;
	QLabel *auteurLbl;

	void restart_now();
	QStack<Historic> hists;
	QStack<Historic> restoreHists;
	int currentHistPos;
protected:
	void showEvent(QShowEvent *);
	void closeEvent(QCloseEvent *);
	QMenu *createPopupMenu();
};

#endif
