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
#include "Window.h"
#include "parametres.h"

Window::Window() :
	field(0), firstShow(true), varDialog(0),
	textDialog(0), _walkmeshManager(0), _backgroundManager(0)
{
	setWindowTitle(PROG_FULLNAME);
	setMinimumSize(700,600);
	resize(900, 700);

	statusBar()->show();
	progression = new QProgressBar(statusBar());
	progression->setFixedSize(160,16);
	progression->setMinimum(0);
	progression->setAlignment(Qt::AlignCenter);
	progression->hide();
	auteurLbl = new QLabel(statusBar());
	auteurLbl->setMargin(2);
	auteurLbl->hide();
	statusBar()->addPermanentWidget(auteurLbl);
	statusBar()->addPermanentWidget(progression);
	
	QMenu *menu;
	QAction *actionOpen, *action;
	
	/* Menu 'Fichier' */
	menu = menuBar()->addMenu(tr("&Fichier"));
	
	actionOpen = menu->addAction(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton), tr("&Ouvrir..."), this, SLOT(ouvrirFichier()), QKeySequence("Ctrl+O"));
	menu->addAction(tr("Ouvrir un &dossier Field (PS)..."), this, SLOT(ouvrirDossier()), QKeySequence("Shift+Ctrl+O"));
	actionSave = menu->addAction(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton), tr("Enregi&strer"), this, SLOT(enregistrer()), QKeySequence("Ctrl+S"));
	actionSaveAs = menu->addAction(tr("Enre&gistrer Sous..."), this, SLOT(enregistrerSous()), QKeySequence("Shift+Ctrl+S"));
	actionExport = menu->addAction(tr("&Exporter l'écran courant..."), this, SLOT(exporter()), QKeySequence("Ctrl+E"));
	actionMassExport = menu->addAction(tr("Exporter en &masse..."), this, SLOT(massExport()), QKeySequence("Shift+Ctrl+E"));
	actionImport = menu->addAction(tr("&Importer dans l'écran courant..."), this, SLOT(importer()), QKeySequence("Ctrl+I"));
	menu->addSeparator();
	actionClose = menu->addAction(QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton), tr("Fe&rmer"), this, SLOT(fermer()));
	menu->addAction(tr("&Quitter"), this, SLOT(close()), QKeySequence::Quit);
	
	/* Menu 'Outils' */
	menu = menuBar()->addMenu(tr("&Outils"));
	
	menu->addAction(tr("&Gestionnaire de variables..."), this, SLOT(varManager()), QKeySequence("Ctrl+G"));
	actionFind = menu->addAction(QIcon(":/images/find.png"), tr("Rec&hercher..."), this, SLOT(searchManager()), QKeySequence("Ctrl+F"));
	actionText = menu->addAction(tr("&Textes..."), this, SLOT(textManager()), QKeySequence("Ctrl+T"));
	actionModels = menu->addAction(tr("&Modèles 3D..."), this, SLOT(modelManager()));
	actionEncounter = menu->addAction(tr("&Rencontres aléatoires..."), this, SLOT(encounterManager()));
	actionTut = menu->addAction(tr("&Tutoriels/Musiques..."), this, SLOT(tutManager()));
	actionWalkmesh = menu->addAction(tr("&Zones..."), this, SLOT(walkmeshManager()));
	actionBackground = menu->addAction(tr("&Background..."), this, SLOT(backgroundManager()));
	actionMisc = menu->addAction(tr("&Divers..."), this, SLOT(miscManager()));

	menu = menuBar()->addMenu(tr("&Paramètres"));

	actionJp_txt = menu->addAction(tr("Caractères japonais"), this, SLOT(jp_txt(bool)));
	actionJp_txt->setCheckable(true);
	actionJp_txt->setChecked(Config::value("jp_txt", false).toBool());

	menuLang = menu->addMenu(tr("&Langues"));
	QDir dir(qApp->applicationDirPath());
	QStringList stringList = dir.entryList(QStringList("Makou_Reactor_*.qm"), QDir::Files, QDir::Name);
	action = menuLang->addAction(tr("Français (défaut)"));
	action->setData("fr");
	action->setCheckable(true);
	action->setChecked(Config::value("lang").toString()=="fr");

	menuLang->addSeparator();
	QTranslator translator;
	foreach(QString str, stringList) {
		translator.load(qApp->applicationDirPath()+"/"+str);
		action = menuLang->addAction(translator.translate("Window", "Français"));
		str = str.mid(14,2);
		action->setData(str);
		action->setCheckable(true);
		action->setChecked(Config::value("lang").toString()==str);
	}
	connect(menuLang, SIGNAL(triggered(QAction*)), this, SLOT(change_language(QAction*)));

	menu->addAction(tr("Configuration..."), this, SLOT(config()));

	toolBar = new QToolBar(tr("Barre d'outils &principale"));
	toolBar->setObjectName("toolBar");
	toolBar->setIconSize(QSize(16,16));
	addToolBar(toolBar);
	toolBar->addAction(actionOpen);
	actionOpen->setStatusTip(tr("Ouvrir un fichier"));
	toolBar->addAction(actionSave);
	actionSave->setStatusTip(tr("Enregistrer"));
	actionUndo = toolBar->addAction(QIcon(":/images/undo.png"), tr("Annuler"), this, SLOT(undo()));
	actionUndo->setShortcut(QKeySequence::Undo);
	actionUndo->setVisible(false);
	actionRedo = toolBar->addAction(QIcon(":/images/redo.png"), tr("Rétablir"), this, SLOT(redo()));
	actionRedo->setShortcut(QKeySequence::Redo);
	actionRedo->setVisible(false);
	toolBar->addSeparator();
	toolBar->addAction(actionFind);
	actionFind->setStatusTip(tr("Rechercher"));
	action = toolBar->addAction(QIcon(":/images/ff7.png"), tr("Lancer FF7"), this, SLOT(runFF7()));
	action->setShortcut(Qt::Key_F8);
	action->setShortcutContext(Qt::ApplicationShortcut);
	action->setEnabled(!Data::ff7AppPath().isEmpty());

	QFont font;
	font.setPointSize(8);

	lineSearch = new QLineEdit(this);
	lineSearch->setFixedWidth(120);
	lineSearch->setStatusTip(tr("Recherche rapide"));
	
	liste = new QTreeWidget(this);
	liste->setColumnCount(2);
	liste->setHeaderLabels(QStringList() << tr("Fichier") << tr("Id"));
	liste->setFixedWidth(120);
	liste->setIndentation(0);
	liste->setItemsExpandable(false);
	liste->setSortingEnabled(true);
	liste->setColumnWidth(1,28);
	liste->setAutoScroll(false);
	liste->resizeColumnToContents(0);
	liste->setFont(font);
	liste->sortByColumn(1, Qt::AscendingOrder);
	connect(liste, SIGNAL(itemSelectionChanged()), SLOT(ouvrirField()));

	liste2 = new GrpScriptList(this);
	liste2->setFixedWidth(180);
	liste2->setFont(font);
	connect(liste2, SIGNAL(changed()), SLOT(activerSave()));
	
	liste3 = new ScriptList(this);
	liste3->setFont(font);
	
	zoneScript = new OpcodeList(this);
	connect(zoneScript, SIGNAL(changed()), SLOT(activerSave()));
	connect(zoneScript, SIGNAL(changed()), SLOT(refresh()));
//	connect(zoneScript, SIGNAL(historicChanged(Historic)), SLOT(changeHistoric(Historic)));//TODO

	zoneImage = new ApercuBG();
	if(Config::value("OpenGL", true).toBool()) {
		fieldModel = new FieldModel();
		fieldModel->setFixedSize(304, 214);
	} else {
		fieldModel = 0;
	}

	zonePreview = new QStackedWidget(this);
	zonePreview->setFixedSize(304, 214);
	zonePreview->addWidget(zoneImage);
	if(fieldModel)
		zonePreview->addWidget(fieldModel);

	gridLayout = new QGridLayout;
	
	gridLayout->addWidget(liste, 0, 0);
	gridLayout->addWidget(lineSearch, 1, 0);
	gridLayout->addWidget(zonePreview, 2, 0, 1, 2);
	
	QVBoxLayout *layout2 = new QVBoxLayout;
	layout2->addWidget(liste2->toolBar());
	layout2->addWidget(liste2);
	layout2->setSpacing(2);
	layout2->setContentsMargins(QMargins());
	gridLayout->addLayout(layout2, 0, 1, 2, 1);
	
	gridLayout->addWidget(liste3, 0, 2, 3, 1);
	
	layout2 = new QVBoxLayout;
	layout2->addWidget(zoneScript->toolBar());
	layout2->addWidget(zoneScript);
	layout2->setSpacing(2);
	layout2->setContentsMargins(QMargins());
	gridLayout->addLayout(layout2, 0, 3, 3, 1);

	gridLayout->setHorizontalSpacing(4);
	gridLayout->setVerticalSpacing(2);
	gridLayout->setContentsMargins(QMargins(4,4,4,4));
	
	QWidget *widget = new QWidget(this);
	widget->setLayout(gridLayout);
	this->setCentralWidget(widget);
	
	searchDialog = new Search(this);
	menuBar()->addMenu(createPopupMenu());
	menuBar()->addAction("&?", this, SLOT(about()));
	
	connect(zoneImage, SIGNAL(clicked()), SLOT(backgroundManager()));
	connect(searchDialog, SIGNAL(found(int,int,int,int)), SLOT(gotoOpcode(int,int,int,int)));
	connect(searchDialog, SIGNAL(foundText(int,int,int,int)), SLOT(gotoText(int,int,int,int)));
	connect(lineSearch, SIGNAL(textEdited(QString)), SLOT(filterMap()));
	connect(lineSearch, SIGNAL(returnPressed()), SLOT(filterMap()));
	connect(this, SIGNAL(fieldIDChanged(int)), searchDialog, SLOT(changeFieldID(int)));
	connect(this, SIGNAL(grpScriptIDChanged(int)), searchDialog, SLOT(changeGrpScriptID(int)));
	connect(this, SIGNAL(scriptIDChanged(int)), searchDialog, SLOT(changeScriptID(int)));
	connect(this, SIGNAL(opcodeIDChanged(int)), searchDialog, SLOT(changeOpcodeID(int)));

	liste2->toolBar()->setVisible(Config::value("grpToolbarVisible", true).toBool());
	zoneScript->toolBar()->setVisible(Config::value("scriptToolbarVisible", true).toBool());

	restoreState(Config::value("windowState").toByteArray());
	restoreGeometry(Config::value("windowGeometry").toByteArray());
	liste->setFocus();
	fieldArchive = NULL;
	fermer();
}

Window::~Window()
{
	Config::flush();
}

void Window::showEvent(QShowEvent *)
{
	if(firstShow) {
		if(!windowState().testFlag(Qt::WindowMaximized)) {
			QPoint screenCenter = QApplication::desktop()->screenGeometry(this).center();
			move(screenCenter.x() - width()/2, screenCenter.y() - height()/2);
		}
	}
	firstShow = false;
}

void Window::closeEvent(QCloseEvent *event)
{
	if(!isEnabled() || fermer(true)==QMessageBox::Cancel)	event->ignore();
	else {
		Config::setValue("windowState", saveState());
		Config::setValue("windowGeometry", saveGeometry());
		Config::setValue("grpToolbarVisible", !liste2->toolBar()->isHidden());
		Config::setValue("scriptToolbarVisible", !zoneScript->toolBar()->isHidden());
		event->accept();
	}
}

QMenu *Window::createPopupMenu()
{
	QMenu *menu = new QMenu(tr("&Affichage"), this);
	menu->addAction(toolBar->toggleViewAction());
	menu->addSeparator();
	menu->addAction(liste2->toolBar()->toggleViewAction());
	menu->addAction(zoneScript->toolBar()->toggleViewAction());
	return menu;
}

FieldArchive::Sorting Window::getFieldSorting()
{
	switch(liste->sortColumn()) {
	case 0:
		return FieldArchive::SortByName;
	default:
		return FieldArchive::SortByMapId;
	}
}

void Window::jp_txt(bool enabled)
{
	Config::setValue("jp_txt", enabled);
	if(textDialog) {
		textDialog->updateText();
	}
	afficherScripts();
}

void Window::change_language(QAction *action)
{
	Config::setValue("lang", action->data());
	foreach(QAction *act, menuLang->actions())
		act->setChecked(false);

	action->setChecked(true);
	restart_now();
}

void Window::restart_now()
{
	QString str_title, str_text;
	QTranslator translator;
	if(translator.load(qApp->applicationDirPath()+"/"+QString("Makou_Reactor_")+Config::value("lang").toString())) {
		str_title = translator.translate("Window", "Paramètres modifiés");
		str_text = translator.translate("Window", "Relancez le programme pour que les paramètres prennent effet.");
	}
	else {
		str_title = "Paramètres modifiés";
		str_text = "Relancez le programme pour que les paramètres prennent effet.";
	}
	QMessageBox::information(this, str_title, str_text);
}

int Window::fermer(bool quit)
{
	if(liste->currentItem() != NULL)
		Config::setValue("currentField", liste->currentItem()->text(0));

	if(actionSave->isEnabled() && fieldArchive!=NULL)
	{
		QString fileChangedList = "";
		int i=0;
		for(int j=0 ; j<fieldArchive->size() ; ++j) {
			Field *curField = fieldArchive->field(j, false);
			if(curField->isModified())
			{
				fileChangedList += "\n - " + curField->getName();
				if(i>10)
				{
					fileChangedList += "\n...";
					break;
				}
				i++;
			}
		}
		int reponse = QMessageBox::warning(this, tr("Sauvegarder"), tr("Voulez-vous enregistrer les changements de %1 ?\n\nFichiers modifiés :%2").arg(fieldArchive->name()).arg(fileChangedList), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if(reponse == QMessageBox::Yes)				enregistrer();
		else if(reponse == QMessageBox::Cancel)		return reponse;
		if(quit)	return reponse;
	}
	
	if(!quit)
	{
		if(varDialog)	varDialog->setFieldArchive(NULL);

		if(fieldArchive!=NULL)
		{
			delete fieldArchive;
			fieldArchive = NULL;
		}
		field = NULL;

		liste->clear();
		liste2->clear();
		liste2->clearCopiedGroups();
		liste2->enableActions(false);
		liste3->clear();
		zoneScript->clear();
		zoneScript->clearCopiedOpcodes();
		zoneScript->enableActions(false);
		zoneImage->clear();
		if(fieldModel)	fieldModel->clear();
		zonePreview->setCurrentIndex(0);

		auteurLbl->hide();
		setWindowModified(false);
		setWindowTitle(PROG_FULLNAME);
		searchDialog->close();
		if(textDialog) 	textDialog->close();
		if(_walkmeshManager)	_walkmeshManager->close();
		if(_backgroundManager)	_backgroundManager->close();
		
		actionSave->setEnabled(false);
		actionSaveAs->setEnabled(false);
		actionExport->setEnabled(false);
		actionMassExport->setEnabled(false);
		actionImport->setEnabled(false);
		actionClose->setEnabled(false);
		actionFind->setEnabled(false);
		actionText->setEnabled(false);
		actionModels->setEnabled(false);
		actionEncounter->setEnabled(false);
		actionTut->setEnabled(false);
		actionMisc->setEnabled(false);
		actionWalkmesh->setEnabled(false);
		actionBackground->setEnabled(false);
		actionUndo->setEnabled(false);
		actionRedo->setEnabled(false);

		hists.clear();
		restoreHists.clear();
	}
	
	return QMessageBox::Yes;
}

void Window::ouvrirFichier()
{
	QString cheminFic = Config::value("open_path").toString();
	if(cheminFic.isEmpty()) {
		cheminFic = Data::ff7DataPath();
		if(!cheminFic.isEmpty())
			cheminFic.append("field/");
	}

	cheminFic = QFileDialog::getOpenFileName(this, tr("Ouvrir un fichier"), cheminFic, tr("Fichiers compatibles (*.lgp *.DAT *.bin *.iso);;Fichiers Lgp (*.lgp);;Fichier DAT (*.DAT);;Image disque (*.bin *.iso)"));
	if(!cheminFic.isNull())	{
		int index;
		if((index = cheminFic.lastIndexOf('/')) == -1)	index = cheminFic.size();
		Config::setValue("open_path", cheminFic.left(index));
		ouvrir(cheminFic);
	}
}

void Window::ouvrirDossier()
{
	QString cheminFic = QFileDialog::getExistingDirectory(this,
														  tr("Sélectionnez un dossier contenant des fichiers .DAT issus de Final Fantasy VII (PlayStation)"),
														  Config::value("open_dir_path").toString());

	if(!cheminFic.isNull())	{
		Config::setValue("open_dir_path", cheminFic);
		ouvrir(cheminFic, true);
	}
}

void Window::ouvrir(const QString &cheminFic, bool isDir)
{
	fermer();
	
	setEnabled(false);
	fieldArchive = new FieldArchive(cheminFic, isDir);

	progression->setValue(0);
	progression->show();
	setCursor(Qt::WaitCursor);

	connect(fieldArchive, SIGNAL(progress(int)), progression, SLOT(setValue(int)));
	connect(fieldArchive, SIGNAL(nbFilesChanged(int)), progression, SLOT(setMaximum(int)));
	
	QList<QTreeWidgetItem *> items;
	quint8 error = fieldArchive->open(items);
	fieldArchive->close();
	
	setCursor(Qt::ArrowCursor);
	progression->hide();
	
	setEnabled(true);
	QString out;
	switch(error)
	{
	case 1:
		out = tr("Le fichier est inaccessible");
		break;
	case 2:
		out = tr("Le fichier est invalide");
		break;
	case 3:
		out = tr("Rien trouvé !");
		break;
	}
	if(!out.isEmpty())
	{
		QMessageBox::warning(this, tr("Erreur"), out);
		return;
	}
	
	liste->addTopLevelItems(items);

	QString previousSessionField = Config::value("currentField").toString();
	if(!previousSessionField.isEmpty()) {
		items = liste->findItems(previousSessionField, Qt::MatchExactly);
		if(!items.isEmpty())	liste->setCurrentItem(items.first());
		else if(liste->topLevelItemCount() > 0)		liste->setCurrentItem(liste->topLevelItem(0));
	}
	else if(liste->topLevelItemCount() > 0)			liste->setCurrentItem(liste->topLevelItem(0));

	liste->setFocus();
	if(varDialog)	varDialog->setFieldArchive(fieldArchive);
	searchDialog->setFieldArchive(fieldArchive);
	actionBackground->setEnabled(true);
	actionEncounter->setEnabled(true);
	actionTut->setEnabled(true);
	actionMisc->setEnabled(true);
	actionWalkmesh->setEnabled(true);
	actionExport->setEnabled(true);
	actionMassExport->setEnabled(true);
	actionSaveAs->setEnabled(true);
	actionImport->setEnabled(true);
	if(fieldArchive->isLgp()/* || fieldArchive->isIso()*/) {
		actionModels->setEnabled(true);
	}
	actionClose->setEnabled(true);
	actionFind->setEnabled(true);
	actionText->setEnabled(true);

//	fieldArchive->searchAll();
}

void Window::ouvrirField()
{
	disconnect(liste2, SIGNAL(itemSelectionChanged()), this, SLOT(afficherGrpScripts()));
	disconnect(liste3, SIGNAL(itemSelectionChanged()), this, SLOT(afficherScripts()));
	
	liste2->clear();
	liste3->clear();
	zoneScript->enableActions(false);
	zoneScript->saveExpandedItems();
	zoneScript->clear();

	QList<QTreeWidgetItem *> selectedItems = liste->selectedItems();
	
	if(selectedItems.isEmpty())	return;
	
	int id = selectedItems.first()->data(0, Qt::UserRole).toInt();
	liste->scrollToItem(selectedItems.first());

//	Data::currentCharNames.clear();
	Data::currentHrcNames = 0;
	Data::currentAnimNames = 0;

	setWindowTitle((!fieldArchive->isDirectory() ? "" : "[*]") + selectedItems.first()->text(0) + (!fieldArchive->isDirectory() ? " ([*]" + fieldArchive->name() + ")" : "") + " - " + PROG_FULLNAME);

	field = fieldArchive->field(id, true, true);
	if(field == NULL) {
		zoneImage->clear();
		liste2->setEnabled(false);
		liste3->clear();
		liste3->setEnabled(false);
		zoneScript->clear();
		zoneScript->setEnabled(false);
		return;
	}
	if(textDialog && textDialog->isVisible())
		textDialog->setField(field);
	if(_walkmeshManager && _walkmeshManager->isVisible())
		_walkmeshManager->fill(field);
	if(_backgroundManager && _backgroundManager->isVisible())
		_backgroundManager->fill(field);

	//Réouvrir fichier pour afficher le background
	zoneImage->fill(field);

	zonePreview->setCurrentIndex(0);
	
	auteurLbl->setText(tr("Auteur : %1").arg(field->scriptsAndTexts()->author()));
	auteurLbl->show();
	
	emit fieldIDChanged(id);
	liste2->setEnabled(true);
	liste2->fill(field->scriptsAndTexts());

	searchDialog->updateRunSearch();

	connect(liste2, SIGNAL(itemSelectionChanged()), SLOT(afficherGrpScripts()));
}

void Window::afficherGrpScripts()
{
	if(field == NULL) {
		liste3->clear();
		liste3->setEnabled(false);
		zoneScript->clear();
		zoneScript->setEnabled(false);
		return;
	}

	liste3->setEnabled(true);
	zoneScript->setEnabled(true);

	disconnect(liste3, SIGNAL(itemSelectionChanged()), this, SLOT(afficherScripts()));
	
	liste3->clear();
	zoneScript->enableActions(false);
	zoneScript->saveExpandedItems();
	zoneScript->clear();
	
	if(liste2->selectedItems().isEmpty())
	{
		zonePreview->setCurrentIndex(0);
		return;
	}
	
	emit grpScriptIDChanged(liste2->selectedID());
	GrpScript *currentGrpScript = liste2->currentGrpScript();
	if(currentGrpScript==NULL)	return;

	liste3->fill(currentGrpScript);

	connect(liste3, SIGNAL(itemSelectionChanged()), SLOT(afficherScripts()));
	liste3->setCurrentRow(0);

	bool modelLoaded = false;

	int modelID = field->scriptsAndTexts()->getModelID(liste2->selectedID());
	Data::currentModelID = modelID;
	if(fieldModel && modelID != -1) {
		modelLoaded = fieldModel->load(field, modelID);
	}

	zonePreview->setCurrentIndex((int)modelLoaded);
}

void Window::afficherScripts()
{
	if(field == NULL) {
		zoneScript->clear();
		zoneScript->setEnabled(false);
		return;
	}

	zoneScript->setEnabled(true);

	zoneScript->saveExpandedItems();
	disconnect(zoneScript, SIGNAL(itemSelectionChanged()), this, SLOT(emitOpcodeID()));
	zoneScript->enableActions(false);
	zoneScript->clear();
	
	if(liste3->selectedItems().isEmpty())	return;
	
	emit scriptIDChanged(liste3->selectedID());
	zoneScript->enableActions(true);
	
	Script *currentScript = liste3->currentScript();
	if(currentScript==NULL)	return;
	zoneScript->fill(field, liste2->currentGrpScript(), currentScript);
	zoneScript->setIsInit(liste3->selectedID()==0);
	zoneScript->scroll(0, false);
	
	connect(zoneScript, SIGNAL(itemSelectionChanged()), SLOT(emitOpcodeID()));
}

void Window::filterMap()
{
	QList<QTreeWidgetItem *> items = liste->findItems(lineSearch->text(), Qt::MatchStartsWith);
	if(!items.isEmpty()) {
		liste->scrollToItem(items.first(), QAbstractItemView::PositionAtTop);
	}
}

void Window::refresh()
{
	liste2->localeRefresh();
	liste3->localeRefresh();
}

void Window::activerSave(bool enabled)
{
	if(field!=NULL)		field->setModified(enabled);
	actionSave->setEnabled(enabled);
	setWindowModified(enabled);
	
	if(enabled && !liste->selectedItems().isEmpty())
		liste->selectedItems().first()->setForeground(0, QColor(0xd1,0x1d,0x1d));
	else if(!enabled) {
		int i, size=liste->topLevelItemCount();
		for(i=0 ; i<size ; ++i) {
			QTreeWidgetItem *item = liste->topLevelItem(i);
			if(item->foreground(0).color()==qRgb(0xd1,0x1d,0x1d))
				item->setForeground(0, QColor(0x1d,0xd1,0x1d));
		}
	}
}

void Window::changeHistoric(const Historic &hist)
{
	actionUndo->setEnabled(true);
	actionRedo->setEnabled(false);
	Historic newHist = hist;
	newHist.fieldID = searchDialog->fieldID;
	newHist.groupID = searchDialog->grpScriptID;
	newHist.scriptID = searchDialog->scriptID;
	hists.push(newHist);
	restoreHists.clear();
}

void Window::undo()
{
	Historic hist = hists.pop();
	actionUndo->setEnabled(!hists.isEmpty());
	zoneScript->setFocus();

	Script *script = fieldArchive->field(hist.fieldID)->scriptsAndTexts()->grpScript(hist.groupID)->getScript(hist.scriptID);
	int firstOpcode = hist.opcodeIDs.first();
	QByteArray sav;

	switch(hist.type) {
	case HIST_ADD:
		for(int i=hist.opcodeIDs.size()-1 ; i>=0 ; --i) {
			hist.data.prepend(script->getOpcode(hist.opcodeIDs.at(i))->toByteArray());
			script->delOpcode(hist.opcodeIDs.at(i));
		}
		break;
	case HIST_REM:
		for(int i=0 ; i<hist.opcodeIDs.size() ; ++i)
			script->insertOpcode(hist.opcodeIDs.at(i), Script::createOpcode(hist.data.at(i)));
		hist.data.clear();
		break;
	case HIST_MOD:
		sav = script->getOpcode(firstOpcode)->toByteArray();
		script->setOpcode(firstOpcode, Script::createOpcode(hist.data.first()));
		hist.data.replace(0, sav);
		break;
	case HIST_UPW:
		script->moveOpcode(firstOpcode-1, DIR_DOWN);
		break;
	case HIST_DOW:
		script->moveOpcode(firstOpcode+1, DIR_UP);
		break;
	}
	restoreHists.push(hist);
	actionRedo->setEnabled(true);
	activerSave(true);

	afficherScripts();//Refresh view
	gotoOpcode(hist.fieldID, hist.groupID, hist.scriptID, firstOpcode);
}

void Window::redo()
{
	Historic hist = restoreHists.pop();
	actionRedo->setEnabled(!restoreHists.isEmpty());
	zoneScript->setFocus();

	Script *script = fieldArchive->field(hist.fieldID)->scriptsAndTexts()->grpScript(hist.groupID)->getScript(hist.scriptID);
	int firstOpcode = hist.opcodeIDs.first();
	QByteArray sav;
	
	switch(hist.type) {
	case HIST_ADD:
		for(int i=0 ; i<hist.opcodeIDs.size() ; ++i)
			script->insertOpcode(hist.opcodeIDs.at(i), Script::createOpcode(hist.data.at(i)));
		hist.data.clear();
		break;
	case HIST_REM:
		for(int i=hist.opcodeIDs.size()-1 ; i>=0 ; --i) {
			hist.data.prepend(script->getOpcode(hist.opcodeIDs.at(i))->toByteArray());
			script->delOpcode(hist.opcodeIDs.at(i));
		}
		break;
	case HIST_MOD:
		sav = script->getOpcode(firstOpcode)->toByteArray();
		script->setOpcode(firstOpcode, Script::createOpcode(hist.data.first()));
		hist.data.replace(0, sav);
		break;
	case HIST_UPW:
		script->moveOpcode(--firstOpcode, DIR_DOWN);
	break;
	case HIST_DOW:
		script->moveOpcode(++firstOpcode, DIR_UP);
	break;
	}

	hists.push(hist);
	actionUndo->setEnabled(true);
	activerSave(true);

	afficherScripts();//Refresh view
	gotoOpcode(hist.fieldID, hist.groupID, hist.scriptID, firstOpcode);
}

void Window::enregistrer() { enregistrerSous(true); }

void Window::enregistrerSous(bool currentPath)
{
	QString cheminFic;
	if(!currentPath)
	{
		cheminFic = QFileDialog::getSaveFileName(this, tr("Enregistrer Sous"), fieldArchive->path(), !fieldArchive->isDirectory() ? tr("Fichier Lgp (*.lgp)") : tr("Fichier DAT (*.DAT)"));
		if(cheminFic.isNull())		return;
	}
	
	setEnabled(false);

	progression->setValue(0);
	progression->show();
	setCursor(Qt::WaitCursor);
	quint8 error = 0;
	
	// QTime t;t.start();
	if(!fieldArchive->isDirectory() || currentPath)
		error = fieldArchive->save(cheminFic);
	else {
		// Cas où on veut enregistrer un seul DAT sous...
		QTemporaryFile tempFic;
		if(tempFic.open())
		{
			QFile DATfic(fieldArchive->path()+"/"+field->getName()+".DAT");
			if(DATfic.open(QIODevice::ReadOnly))
			{
				DATfic.seek(4);
				tempFic.write(field->save(DATfic.readAll(), true));
				DATfic.reset();
				if(cheminFic!=DATfic.fileName() || DATfic.remove())
				{
					tempFic.copy(cheminFic);
				}
				else
					error = 4;
			}
			else
				error = 1;
		}
		else
			error = 2;
	}
	fieldArchive->close();
	// qDebug("Temps total enregistrement : %d ms", t.elapsed());
	
	setCursor(Qt::ArrowCursor);
	progression->hide();
	QString out;
	switch(error)
	{
	case 0:	activerSave(false);break;
	case 1:	out = tr("Le fichier est inaccessible");break;
	case 2:	out = tr("Impossible de créer un fichier temporaire");break;
	case 3:	out = tr("Erreur de réouverture du fichier");break;
	case 4:	out = tr("Impossible d'écrire dans l'archive, vérifiez les droits d'écriture.");break;
	case 5:	out = tr("Problème de validation");break;
	}
	if(!out.isEmpty())	QMessageBox::warning(this, tr("Erreur"), out);
	
	setEnabled(true);
}

bool Window::gotoField(int fieldID)
{
	int i, size=liste->topLevelItemCount();
	for(i=0 ; i<size ; ++i) {
		QTreeWidgetItem *item = liste->topLevelItem(i);
		if(item->data(0, Qt::UserRole).toInt() == fieldID) {
			blockSignals(true);
			liste->setCurrentItem(item);
			liste->scrollToItem(item);
			blockSignals(false);
			return true;
		}
	}
	return false;
}

void Window::gotoOpcode(int fieldID, int grpScriptID, int scriptID, int opcodeID)
{
	if(gotoField(fieldID)) {
		blockSignals(true);
		liste2->scroll(grpScriptID, false);
		liste3->scroll(scriptID, false);
		zoneScript->scroll(opcodeID);
		blockSignals(false);
	}
}

void Window::gotoText(int fieldID, int textID, int from, int size)
{
	if(textDialog) {
		textDialog->blockSignals(true);
	}
	if(gotoField(fieldID)) {
		textManager(false); // show texts dialog
		if(textDialog) {
			textDialog->gotoText(textID, from, size);
		}
	}
	if(textDialog) {
		textDialog->blockSignals(false);
	}
}

void Window::emitOpcodeID()
{
	if(zoneScript->selectedID() != -1)	emit opcodeIDChanged(zoneScript->selectedID());
}

/* void Window::notifyFileChanged(const QString &path)
{
	if(!QFile::exists(path)) {
		QMessageBox::warning(this, tr("Fichier supprimé"), tr("Le fichier '%1' a été supprimé par un programme externe !").arg(path));
	}
	else
	{
		int reponse = QMessageBox::warning(this, tr("Fichier modifié"), tr("Le fichier '%1' a été modifié par un programme externe.\nVoulez-vous recharger ce fichier ?").arg(path)
			, QMessageBox::Yes | QMessageBox::No);
		if(reponse == QMessageBox::Yes) {
			ouvrir(path);
		}
	}
}

void Window::notifyDirectoryChanged(const QString &path)
{
	if(!QFile::exists(path)) {
		QMessageBox::warning(this, tr("Dossier supprimé"), tr("Le dossier '%1' a été supprimé par un programme externe !").arg(path));
	}
	else
	{
		int reponse = QMessageBox::warning(this, tr("Dossier modifié"), tr("Le dossier '%1' a été modifié par un programme externe.\nVoulez-vous recharger ce dossier ?").arg(path)
			, QMessageBox::Yes | QMessageBox::No);
		if(reponse == QMessageBox::Yes) {
			ouvrir(path, true);
		}
	}
} */

void Window::exporter()
{
	if(!field) return;

	int index;
	QString types, name, selectedFilter,
			fieldDec = tr("Field décompressé (*.dec)"),
			fieldLzs = tr("Field compressé (*.lzs)"),
			dat = tr("Fichier DAT (*.DAT)");

	name = liste->selectedItems().first()->text(0);

	if(fieldArchive->isLgp()) {
		types = fieldDec+";;"+fieldLzs;
	} else {
		types = dat;
		name = name.toUpper();
	}

	QString path = Config::value("exportPath").toString().isEmpty() ? fieldArchive->chemin() : Config::value("exportPath").toString()+"/";
	path = QFileDialog::getSaveFileName(this, tr("Exporter le fichier courant"), path+name, types, &selectedFilter);
	if(path.isNull())		return;
	qint8 error=4;
	
	if(selectedFilter == dat) {
		error = field->exporterDat(path, fieldArchive->getFieldData(field, false));
	} else {
		error = field->exporter(path, fieldArchive->getFieldData(field, false), selectedFilter == fieldLzs);
	}
	
	QString out;
	switch(error)
	{
	case 0:
		index = path.lastIndexOf('/');
		Config::setValue("exportPath", index == -1 ? path : path.left(index));
		break;
	case 1:	out = tr("L'archive Lgp est inaccessible");break;
	case 2:	out = tr("Erreur de réouverture du fichier");break;
	case 3:	out = tr("Impossible de créer le nouveau fichier");break;
	case 4:	out = tr("Pas encore implémenté !");break;
	}
	if(!out.isEmpty())	QMessageBox::warning(this, tr("Erreur"), out);
}

void Window::massExport()
{
	if(!fieldArchive) return;

	MassExportDialog *massExportDialog = new MassExportDialog(this);
	massExportDialog->fill(fieldArchive, 0);
	if(massExportDialog->exec() == QDialog::Accepted) {
		QList<int> selectedFields = massExportDialog->selectedFields();
		if(!selectedFields.isEmpty()) {
			QString extension, path;
			int currentField=0;
			QProgressDialog progressDialog(this, Qt::Dialog | Qt::WindowCloseButtonHint);
			progressDialog.setWindowModality(Qt::WindowModal);
			progressDialog.setCancelButtonText(tr("Arrêter"));
			progressDialog.setRange(0, selectedFields.size()-1);

			if(massExportDialog->exportBackground()) {
				progressDialog.setLabelText(tr("Exportation des décors..."));
				switch(massExportDialog->exportBackgroundFormat()) {
				case 0:		extension = "png"; break;
				case 1:		extension = "jpg"; break;
				case 2:		extension = "bmp"; break;
				}

				foreach(const int &fieldID, selectedFields) {
					QCoreApplication::processEvents();
					if(progressDialog.wasCanceled()) 	break;

					Field *f = fieldArchive->field(fieldID);
					if(f) {
						path = QDir::cleanPath(QString("%1/%2.%3").arg(massExportDialog->directory(), f->getName(), extension));

						if(massExportDialog->overwrite() || !QFile::exists(path)) {
							QPixmap background = f->openBackground();
							if(!background.isNull())
								background.save(path);
						}
					}
					progressDialog.setValue(currentField++);
				}
			} else if(massExportDialog->exportAkao()) {
				progressDialog.setLabelText(tr("Exportation des sons..."));
				extension = "akao";

				foreach(const int &fieldID, selectedFields) {
					QCoreApplication::processEvents();
					if(progressDialog.wasCanceled()) 	break;

					Field *f = fieldArchive->field(fieldID);
					if(f) {
						TutFile *akaoList = f->tutosAndSounds();
						if(!akaoList->isOpen())
							akaoList->open(fieldArchive->getFieldData(f));
						int akaoCount = akaoList->size();
						for(int i=0 ; i<akaoCount ; ++i) {
							if(!akaoList->isTut(i)) {
								path = QDir::cleanPath(QString("%1/%2-%3.%4").arg(massExportDialog->directory(), f->getName()).arg(i).arg(extension));
								if(massExportDialog->overwrite() || !QFile::exists(path)) {
									QFile tutExport(path);
									if(tutExport.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
										tutExport.write(akaoList->data(i));
										tutExport.close();
									}
								}
							}
						}
					}
					progressDialog.setValue(currentField++);
				}
			}
		}
	}
}

void Window::importer()
{
	if(!field) return;

	int index;
	QString name, selectedFilter;
	QStringList filter;
	filter << tr("Écran PC (*)")
		   << tr("Fichier DAT (*.DAT)")
		   << tr("Écran PC décompressé (*)")
		   << tr("Fichier DAT décompressé (*)");

	name = liste->selectedItems().first()->text(0);
	if(!fieldArchive->isLgp())
		name = name.toUpper();

	QString path = Config::value("importPath").toString().isEmpty() ? fieldArchive->chemin() : Config::value("importPath").toString()+"/";
	path = QFileDialog::getOpenFileName(this, tr("Importer un fichier"), path+name, filter.join(";;"), &selectedFilter);
	if(path.isNull())		return;

	bool isDat = selectedFilter == filter.at(1) || selectedFilter == filter.at(3);

	ImportDialog dialog((isDat && !fieldArchive->isLgp()) || (!isDat && fieldArchive->isLgp()), isDat, this);
	if(dialog.exec() != QDialog::Accepted) {
		return;
	}

	Field::FieldParts parts = dialog.getParts();
	if(parts == 0) {
		return;
	}
	
	qint8 error = field->importer(path, filter.indexOf(selectedFilter), parts);
	
	QString out;
	switch(error)
	{
	case 0:
		activerSave(true);
		// remove field history
		for(int i=hists.size()-1 ; i>=0 ; --i) {
			if(hists.at(i).fieldID == searchDialog->fieldID) {
				hists.remove(i);
			}
		}
		actionUndo->setEnabled(!hists.isEmpty());
		for(int i=restoreHists.size()-1 ; i>=0 ; --i) {
			if(restoreHists.at(i).fieldID == searchDialog->fieldID) {
				restoreHists.remove(i);
			}
		}
		actionRedo->setEnabled(!restoreHists.isEmpty());
		index = path.lastIndexOf('/');
		Config::setValue("importPath", index == -1 ? path : path.left(index));
		ouvrirField();
		break;
	case 1:	out = tr("L'archive Lgp est inaccessible");break;
	case 2:	out = tr("Erreur de réouverture du fichier");break;
	case 3:	out = tr("Erreur lors de l'ouverture du fichier");break;
	}
	if(!out.isEmpty())	QMessageBox::warning(this, tr("Erreur"), out);
}

void Window::varManager()
{
	if(!varDialog) {
		varDialog = new VarManager(fieldArchive, this);
	}
	varDialog->show();
}

void Window::runFF7()
{
	QString FF7Exe = Data::ff7AppPath();
	QString FF7ExeDir = FF7Exe.left(FF7Exe.lastIndexOf('/'));

	if(!QProcess::startDetached("\"" % FF7Exe % "\"", QStringList(), FF7ExeDir)) {
		QMessageBox::warning(this, tr("Erreur"), tr("Final Fantasy VII n'a pas pu être lancé.\n%1").arg(FF7Exe));
	}
}

void Window::searchManager()
{
	searchDialog->setOpcode(zoneScript->selectedOpcode());
	searchDialog->setScriptExec(liste2->selectedID(), liste3->selectedID()-1);
	searchDialog->show();
}

void Window::textManager(bool activate)
{
	if(field) {
		if(!textDialog) {
			textDialog = new TextManager(this);
			connect(textDialog, SIGNAL(modified()), SLOT(activerSave()));
			connect(textDialog, SIGNAL(textIDChanged(int)), searchDialog, SLOT(changeTextID(int)));
			connect(textDialog, SIGNAL(fromChanged(int)), searchDialog, SLOT(changeFrom(int)));
		}

		textDialog->setField(field);
		textDialog->show();
		if(activate) {
			textDialog->activateWindow();
		} else {
			searchDialog->raise();
		}
	}
}

void Window::modelManager()
{
	if(field) {
		ModelManager modelManager(field, fieldModel, this);
		if(modelManager.exec()==QDialog::Accepted)
		{
			activerSave(true);
		}
	}
}

void Window::encounterManager()
{
	if(field) {
		EncounterFile *encounter = field->encounter();
		if(encounter->isOpen()) {
			EncounterWidget dialog(encounter, this);
			if(dialog.exec()==QDialog::Accepted)
			{
				if(encounter->isModified())
					activerSave(true);
			}
		} else {
			QMessageBox::warning(this, tr("Erreur d'ouverture"), tr("Impossible d'ouvrir les combats aléatoires !"));
		}
	}
}

void Window::tutManager()
{
	if(field) {
		TutFile *tut = field->tutosAndSounds(), *tutPC = fieldArchive->getTut(field->getName());
		if(tut->isOpen()) {
			TutWidget dialog(field, tut, tutPC, this);
			if(dialog.exec()==QDialog::Accepted)
			{
				if(tut->isModified() || (tutPC != NULL && tutPC->isModified()))
					activerSave(true);
			}
		} else {
			QMessageBox::warning(this, tr("Erreur d'ouverture"), tr("Impossible d'ouvrir les sons et les tutoriels !"));
		}
	}
}

void Window::walkmeshManager()
{
	if(field) {
		if(!_walkmeshManager) {
			_walkmeshManager = new WalkmeshManager(this);
			connect(_walkmeshManager, SIGNAL(modified()), SLOT(activerSave()));
		}

		_walkmeshManager->fill(field);
		_walkmeshManager->show();
		_walkmeshManager->activateWindow();
	}
}

void Window::backgroundManager()
{
	if(fieldArchive && field) {
		if(!_backgroundManager) {
			_backgroundManager = new BGDialog(this);
		}

		_backgroundManager->fill(field);
		_backgroundManager->show();
		_backgroundManager->activateWindow();
	}
}

void Window::miscManager()
{
	if(field) {
		InfFile *inf = field->getInf();
		if(inf->isOpen()) {
			MiscWidget dialog(inf, field, this);
			if(dialog.exec()==QDialog::Accepted)
			{
				if(inf->isModified() || field->isModified()) {
					activerSave(true);
					auteurLbl->setText(tr("Auteur : %1").arg(field->scriptsAndTexts()->author()));
				}
			}
		} else {
			QMessageBox::warning(this, tr("Erreur d'ouverture"), tr("Impossible d'ouvrir les infos diverses !"));
		}
	}
}

void Window::config()
{
	ConfigWindow configWindow(this);
	if(configWindow.exec() == QDialog::Accepted) {
		actionJp_txt->setChecked(Config::value("jp_txt", false).toBool());
		if(textDialog) {
			textDialog->updateText();
		}
		afficherScripts();
	}
}

void Window::about()
{
	QDialog apropos(this, Qt::Dialog | Qt::CustomizeWindowHint);
	apropos.setFixedSize(200, 200);

	QFont font;
	font.setPointSize(12);

	QLabel image(&apropos);
	image.setPixmap(QPixmap(":/images/reactor.png"));
	image.move(82, 49);
	
	QLabel desc1(PROG_FULLNAME, &apropos);
	desc1.setFont(font);
	desc1.setFixedWidth(200);
	desc1.setAlignment(Qt::AlignHCenter);

	font.setPointSize(8);

	QLabel desc2(tr("Par myst6re"), &apropos);
	desc2.move(9, 40);
	desc2.setFont(font);

	QLabel desc3("myst6re@gmail.com", &apropos);
	desc3.move(9, 52);
	desc3.setFont(font);
	desc3.setTextFormat(Qt::PlainText);
	desc3.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

	QLabel desc4(QString("Qt %1").arg(QT_VERSION_STR), &apropos);
	desc4.move(9, 150);
	desc4.setFont(font);
	QPalette pal = desc4.palette();
	pal.setColor(QPalette::WindowText, QColor(0xAA,0xAA,0xAA));
	desc4.setPalette(pal);

	QPushButton button(tr("Fermer"), &apropos);
	button.move(8, 169);
	connect(&button, SIGNAL(released()), &apropos, SLOT(close()));

	apropos.exec();
}
