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
	setMinimumSize(700, 600);
	resize(900, 700);

	statusBar()->show();
	progression = new QProgressBar(statusBar());
	progression->setFixedSize(160,16);
	progression->setMinimum(0);
	progression->setAlignment(Qt::AlignCenter);
	progression->hide();
	authorLbl = new QLabel(statusBar());
	authorLbl->setMargin(2);
	authorLbl->hide();
	statusBar()->addPermanentWidget(authorLbl);
	statusBar()->addPermanentWidget(progression);
	
	QMenu *menu;
	QAction *actionOpen, *action;
	
	/* Menu 'Fichier' */
	menu = menuBar()->addMenu(tr("&Fichier"));
	
	actionOpen = menu->addAction(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton), tr("&Ouvrir..."), this, SLOT(openFile()), QKeySequence("Ctrl+O"));
	menu->addAction(tr("Ouvrir un &dossier Field (PS)..."), this, SLOT(openDir()), QKeySequence("Shift+Ctrl+O"));
	actionSave = menu->addAction(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton), tr("Enregi&strer"), this, SLOT(save()), QKeySequence("Ctrl+S"));
	actionSaveAs = menu->addAction(tr("Enre&gistrer Sous..."), this, SLOT(saveAs()), QKeySequence("Shift+Ctrl+S"));
	actionExport = menu->addAction(tr("&Exporter l'écran courant..."), this, SLOT(exporter()), QKeySequence("Ctrl+E"));
	actionMassExport = menu->addAction(tr("Exporter en &masse..."), this, SLOT(massExport()), QKeySequence("Shift+Ctrl+E"));
	actionImport = menu->addAction(tr("&Importer dans l'écran courant..."), this, SLOT(importer()), QKeySequence("Ctrl+I"));
	menu->addSeparator();
	actionClose = menu->addAction(QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton), tr("Fe&rmer"), this, SLOT(close()));
	menu->addAction(tr("&Quitter"), this, SLOT(close()), QKeySequence::Quit);
	
	/* Menu 'Outils' */
	menu = menuBar()->addMenu(tr("&Outils"));
	
	menu->addAction(tr("&Gestionnaire de variables..."), this, SLOT(varManager()), QKeySequence("Ctrl+G"));
	actionFind = menu->addAction(QIcon(":/images/find.png"), tr("Rec&hercher..."), this, SLOT(searchManager()), QKeySequence("Ctrl+F"));
	actionText = menu->addAction(tr("&Textes..."), this, SLOT(textManager()), QKeySequence("Ctrl+T"));
	actionModels = menu->addAction(tr("&Modèles 3D..."), this, SLOT(modelManager()), QKeySequence("Ctrl+M"));
	actionEncounter = menu->addAction(tr("&Rencontres aléatoires..."), this, SLOT(encounterManager()), QKeySequence("Ctrl+N"));
	actionTut = menu->addAction(tr("&Tutoriels/Musiques..."), this, SLOT(tutManager()), QKeySequence("Ctrl+Q"));
	actionWalkmesh = menu->addAction(tr("&Zones..."), this, SLOT(walkmeshManager()), QKeySequence("Ctrl+W"));
	actionBackground = menu->addAction(tr("&Background..."), this, SLOT(backgroundManager()), QKeySequence("Ctrl+B"));
	actionMisc = menu->addAction(tr("&Divers..."), this, SLOT(miscManager()));

	menu = menuBar()->addMenu(tr("&Paramètres"));

	actionJp_txt = menu->addAction(tr("Caractères japonais"), this, SLOT(jpText(bool)));
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
	connect(menuLang, SIGNAL(triggered(QAction*)), this, SLOT(changeLanguage(QAction*)));

	menu->addAction(tr("Configuration..."), this, SLOT(config()));

	toolBar = new QToolBar(tr("Barre d'outils &principale"));
	toolBar->setObjectName("toolBar");
	toolBar->setIconSize(QSize(16,16));
	addToolBar(toolBar);
	toolBar->addAction(actionOpen);
	actionOpen->setStatusTip(tr("Ouvrir un fichier"));
	toolBar->addAction(actionSave);
	actionSave->setStatusTip(tr("Enregistrer"));
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
	
	fieldList = new QTreeWidget(this);
	fieldList->setColumnCount(2);
	fieldList->setHeaderLabels(QStringList() << tr("Fichier") << tr("Id"));
	fieldList->setFixedWidth(120);
	fieldList->setIndentation(0);
	fieldList->setItemsExpandable(false);
	fieldList->setSortingEnabled(true);
	fieldList->setColumnWidth(1,28);
	fieldList->setAutoScroll(false);
	fieldList->resizeColumnToContents(0);
	fieldList->setFont(font);
	fieldList->sortByColumn(1, Qt::AscendingOrder);
	connect(fieldList, SIGNAL(itemSelectionChanged()), SLOT(openField()));

	groupScriptList = new GrpScriptList(this);
	groupScriptList->setFixedWidth(180);
	groupScriptList->setFont(font);
	connect(groupScriptList, SIGNAL(changed()), SLOT(setModified()));
	
	scriptList = new ScriptList(this);
	scriptList->setFont(font);
	
	opcodeList = new OpcodeList(this);
	connect(opcodeList, SIGNAL(changed()), SLOT(setModified()));
	connect(opcodeList, SIGNAL(changed()), SLOT(refresh()));

	compileScriptLabel = new QLabel(this);
	compileScriptLabel->hide();
	QPalette pal = compileScriptLabel->palette();
	pal.setColor(QPalette::Active, QPalette::WindowText, Qt::red);
	pal.setColor(QPalette::Inactive, QPalette::WindowText, Qt::red);
	compileScriptLabel->setPalette(pal);
	compileScriptIcon = new QLabel(this);
	compileScriptIcon->setPixmap(QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical).pixmap(16));
	compileScriptIcon->hide();
	connect(opcodeList, SIGNAL(changed()), SLOT(compile()));

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
	
	gridLayout->addWidget(fieldList, 0, 0);
	gridLayout->addWidget(lineSearch, 1, 0);
	gridLayout->addWidget(zonePreview, 2, 0, 1, 2);
	
	QVBoxLayout *layout2 = new QVBoxLayout;
	layout2->addWidget(groupScriptList->toolBar());
	layout2->addWidget(groupScriptList);
	layout2->setSpacing(2);
	layout2->setContentsMargins(QMargins());
	gridLayout->addLayout(layout2, 0, 1, 2, 1);
	
	gridLayout->addWidget(scriptList, 0, 2, 3, 1);

	QHBoxLayout *compileLayout = new QHBoxLayout;
	compileLayout->addWidget(compileScriptIcon);
	compileLayout->addWidget(compileScriptLabel, 1);
	compileLayout->setContentsMargins(QMargins());

	layout2 = new QVBoxLayout;
	layout2->addWidget(opcodeList->toolBar());
	layout2->addWidget(opcodeList);
	layout2->addLayout(compileLayout);
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

	groupScriptList->toolBar()->setVisible(Config::value("grpToolbarVisible", true).toBool());
	opcodeList->toolBar()->setVisible(Config::value("scriptToolbarVisible", true).toBool());

	restoreState(Config::value("windowState").toByteArray());
	restoreGeometry(Config::value("windowGeometry").toByteArray());
	fieldList->setFocus();
	fieldArchive = NULL;
	close();
}

Window::~Window()
{
	Config::flush();
	if(fieldArchive)	fieldArchive->close();
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
	if(!isEnabled() || close(true)==QMessageBox::Cancel)	event->ignore();
	else {
		Config::setValue("windowState", saveState());
		Config::setValue("windowGeometry", saveGeometry());
		Config::setValue("grpToolbarVisible", !groupScriptList->toolBar()->isHidden());
		Config::setValue("scriptToolbarVisible", !opcodeList->toolBar()->isHidden());
		event->accept();
	}
}

QMenu *Window::createPopupMenu()
{
	QMenu *menu = new QMenu(tr("&Affichage"), this);
	menu->addAction(toolBar->toggleViewAction());
	menu->addSeparator();
	menu->addAction(groupScriptList->toolBar()->toggleViewAction());
	menu->addAction(opcodeList->toolBar()->toggleViewAction());
	return menu;
}

FieldArchive::Sorting Window::getFieldSorting()
{
	switch(fieldList->sortColumn()) {
	case 0:
		return FieldArchive::SortByName;
	default:
		return FieldArchive::SortByMapId;
	}
}

void Window::jpText(bool enabled)
{
	Config::setValue("jp_txt", enabled);
	if(textDialog) {
		textDialog->updateText();
	}
	showScripts();
}

void Window::changeLanguage(QAction *action)
{
	Config::setValue("lang", action->data());
	foreach(QAction *act, menuLang->actions())
		act->setChecked(false);

	action->setChecked(true);
	restartNow();
}

void Window::restartNow()
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

int Window::close(bool quit)
{
	if(fieldList->currentItem() != NULL)
		Config::setValue("currentField", fieldList->currentItem()->text(0));

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
		if(reponse == QMessageBox::Yes)				save();
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

		fieldList->clear();
		groupScriptList->clear();
		groupScriptList->clearCopiedGroups();
		groupScriptList->enableActions(false);
		scriptList->clear();
		opcodeList->clear();
		opcodeList->clearCopiedOpcodes();
		zoneImage->clear();
		if(fieldModel)	fieldModel->clear();
		zonePreview->setCurrentIndex(0);

		authorLbl->hide();
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
	}
	
	return QMessageBox::Yes;
}

void Window::openFile()
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
		open(cheminFic);
	}
}

void Window::openDir()
{
	QString cheminFic = QFileDialog::getExistingDirectory(this,
														  tr("Sélectionnez un dossier contenant des fichiers .DAT issus de Final Fantasy VII (PlayStation)"),
														  Config::value("open_dir_path").toString());

	if(!cheminFic.isNull())	{
		Config::setValue("open_dir_path", cheminFic);
		open(cheminFic, true);
	}
}

void Window::open(const QString &cheminFic, bool isDir)
{
	close();
	
	setEnabled(false);
	fieldArchive = new FieldArchive(cheminFic, isDir);

	progression->setValue(0);
	progression->show();
	setCursor(Qt::WaitCursor);

	connect(fieldArchive, SIGNAL(progress(int)), progression, SLOT(setValue(int)));
	connect(fieldArchive, SIGNAL(nbFilesChanged(int)), progression, SLOT(setMaximum(int)));
	
	QList<QTreeWidgetItem *> items;
	quint8 error = fieldArchive->open(items);
//	fieldArchive->close();
	
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
	
	fieldList->addTopLevelItems(items);

	QString previousSessionField = Config::value("currentField").toString();
	if(!previousSessionField.isEmpty()) {
		items = fieldList->findItems(previousSessionField, Qt::MatchExactly);
		if(!items.isEmpty())	fieldList->setCurrentItem(items.first());
		else if(fieldList->topLevelItemCount() > 0)		fieldList->setCurrentItem(fieldList->topLevelItem(0));
	}
	else if(fieldList->topLevelItemCount() > 0)			fieldList->setCurrentItem(fieldList->topLevelItem(0));

	fieldList->setFocus();
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

void Window::openField()
{
	disconnect(groupScriptList, SIGNAL(itemSelectionChanged()), this, SLOT(showGrpScripts()));
	disconnect(scriptList, SIGNAL(itemSelectionChanged()), this, SLOT(showScripts()));
	
	// Clear lists
	groupScriptList->clear();
	scriptList->clear();
	opcodeList->clear();

	QList<QTreeWidgetItem *> selectedItems = fieldList->selectedItems();
	if(selectedItems.isEmpty())	return;
	
	int id = selectedItems.first()->data(0, Qt::UserRole).toInt();
	fieldList->scrollToItem(selectedItems.first());

//	Data::currentCharNames.clear();
	Data::currentHrcNames = 0;
	Data::currentAnimNames = 0;

	setWindowTitle((!fieldArchive->isDirectory() ? "" : "[*]") + selectedItems.first()->text(0) + (!fieldArchive->isDirectory() ? " ([*]" + fieldArchive->name() + ")" : "") + " - " + PROG_FULLNAME);

	// Get and set field
	field = fieldArchive->field(id, true, true);
	if(field == NULL) {
		zoneImage->clear();
		groupScriptList->setEnabled(false);
		scriptList->clear();
		scriptList->setEnabled(false);
		opcodeList->clear();
		opcodeList->setEnabled(false);
		return;
	}
	if(textDialog && textDialog->isVisible())
		textDialog->setField(field);
	if(_walkmeshManager && _walkmeshManager->isVisible())
		_walkmeshManager->fill(field);
	if(_backgroundManager && _backgroundManager->isVisible())
		_backgroundManager->fill(field);

	// Show background preview
	zoneImage->fill(field);
	zonePreview->setCurrentIndex(0);

	// Show author
	authorLbl->setText(tr("Auteur : %1").arg(field->scriptsAndTexts()->author()));
	authorLbl->show();

	emit fieldIDChanged(id);
	// Fill group script list
	groupScriptList->setEnabled(true);
	groupScriptList->fill(field->scriptsAndTexts());

	searchDialog->updateRunSearch();

	connect(groupScriptList, SIGNAL(itemSelectionChanged()), SLOT(showGrpScripts()));
}

void Window::showGrpScripts()
{
	if(field == NULL) {
		scriptList->clear();
		scriptList->setEnabled(false);
		opcodeList->clear();
		opcodeList->setEnabled(false);
		return;
	}

	scriptList->setEnabled(true);
	opcodeList->setEnabled(true);

	disconnect(scriptList, SIGNAL(itemSelectionChanged()), this, SLOT(showScripts()));
	
	scriptList->clear();
	opcodeList->clear();
	
	if(groupScriptList->selectedItems().isEmpty())
	{
		zonePreview->setCurrentIndex(0);
		return;
	}
	
	emit grpScriptIDChanged(groupScriptList->selectedID());
	GrpScript *currentGrpScript = groupScriptList->currentGrpScript();
	if(currentGrpScript==NULL)	return;

	scriptList->fill(currentGrpScript);

	connect(scriptList, SIGNAL(itemSelectionChanged()), SLOT(showScripts()));
	scriptList->setCurrentRow(0);

	bool modelLoaded = false;

	int modelID = field->scriptsAndTexts()->getModelID(groupScriptList->selectedID());
	Data::currentModelID = modelID;
	if(fieldModel && modelID != -1) {
		modelLoaded = fieldModel->load(field, modelID);
	}

	zonePreview->setCurrentIndex((int)modelLoaded);
}

void Window::showScripts()
{
	if(field == NULL) {
		opcodeList->clear();
		opcodeList->setEnabled(false);
		return;
	}

	opcodeList->setEnabled(true);

	disconnect(opcodeList, SIGNAL(itemSelectionChanged()), this, SLOT(emitOpcodeID()));
	opcodeList->clear();
	
	if(scriptList->selectedItems().isEmpty())	return;
	
	emit scriptIDChanged(scriptList->selectedID());
	
	Script *currentScript = scriptList->currentScript();
	if(currentScript==NULL)	return;
	opcodeList->fill(field, groupScriptList->currentGrpScript(), currentScript);
	opcodeList->setIsInit(scriptList->selectedID()==0);
	opcodeList->scroll(0, false);
	
	connect(opcodeList, SIGNAL(itemSelectionChanged()), SLOT(emitOpcodeID()));
}

void Window::compile()
{
	Script *currentScript = scriptList->currentScript();
	if(currentScript==NULL)	return;

	int opcodeID;
	QString errorStr;

	if(!currentScript->compile(opcodeID, errorStr)) {
		compileScriptLabel->setText(QString("Erreur ligne %1 : %2").arg(opcodeID+1).arg(errorStr));
		compileScriptLabel->show();
		compileScriptIcon->show();
		opcodeList->setErrorLine(opcodeID);
	} else {
		compileScriptLabel->hide();
		compileScriptIcon->hide();
		opcodeList->setErrorLine(-1);
	}
}

void Window::filterMap()
{
	QList<QTreeWidgetItem *> items = fieldList->findItems(lineSearch->text(), Qt::MatchStartsWith);
	if(!items.isEmpty()) {
		fieldList->scrollToItem(items.first(), QAbstractItemView::PositionAtTop);
	}
}

void Window::refresh()
{
	groupScriptList->localeRefresh();
	scriptList->localeRefresh();
}

void Window::setModified(bool enabled)
{
	if(field != NULL)		field->setModified(enabled);
	actionSave->setEnabled(enabled);
	setWindowModified(enabled);
	
	if(enabled && !fieldList->selectedItems().isEmpty())
		fieldList->selectedItems().first()->setForeground(0, QColor(0xd1,0x1d,0x1d));
	else if(!enabled) {
		int i, size=fieldList->topLevelItemCount();
		for(i=0 ; i<size ; ++i) {
			QTreeWidgetItem *item = fieldList->topLevelItem(i);
			if(item->foreground(0).color()==qRgb(0xd1,0x1d,0x1d))
				item->setForeground(0, QColor(0x1d,0xd1,0x1d));
		}
	}
}

void Window::save() { saveAs(true); }

void Window::saveAs(bool currentPath)
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
//	fieldArchive->close();
	// qDebug("Temps total enregistrement : %d ms", t.elapsed());
	
	setCursor(Qt::ArrowCursor);
	progression->hide();
	QString out;
	switch(error)
	{
	case 0:	setModified(false);break;
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
	int i, size=fieldList->topLevelItemCount();
	for(i=0 ; i<size ; ++i) {
		QTreeWidgetItem *item = fieldList->topLevelItem(i);
		if(item->data(0, Qt::UserRole).toInt() == fieldID) {
			blockSignals(true);
			fieldList->setCurrentItem(item);
			fieldList->scrollToItem(item);
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
		groupScriptList->scroll(grpScriptID, false);
		scriptList->scroll(scriptID, false);
		opcodeList->scroll(opcodeID);
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
	if(opcodeList->selectedID() != -1)	emit opcodeIDChanged(opcodeList->selectedID());
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
			open(path);
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
			open(path, true);
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

	name = fieldList->selectedItems().first()->text(0);

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

	name = fieldList->selectedItems().first()->text(0);
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
	qDebug() << "importation success";
	QString out;
	switch(error)
	{
	case 0:
		qDebug() << "setModified";
		setModified(true);
		index = path.lastIndexOf('/');
		Config::setValue("importPath", index == -1 ? path : path.left(index));
		qDebug() << "openField";
		openField();
		qDebug() << "/openField";
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
	searchDialog->setOpcode(opcodeList->selectedOpcode());
	searchDialog->setScriptExec(groupScriptList->selectedID(), scriptList->selectedID()-1);
	searchDialog->show();
}

void Window::textManager(bool activate)
{
	if(field) {
		if(!textDialog) {
			textDialog = new TextManager(this);
			connect(textDialog, SIGNAL(modified()), SLOT(setModified()));
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
			setModified(true);
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
					setModified(true);
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
					setModified(true);
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
			connect(_walkmeshManager, SIGNAL(modified()), SLOT(setModified()));
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
					setModified(true);
					authorLbl->setText(tr("Auteur : %1").arg(field->scriptsAndTexts()->author()));
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
		showScripts();
	}
}

void Window::about()
{
	QDialog about(this, Qt::Dialog | Qt::CustomizeWindowHint);
	about.setFixedSize(200, 270);

	QFont font;
	font.setPointSize(12);

	QLabel image(&about);
	image.setPixmap(QPixmap(":/images/reactor.png"));
	image.move(82, about.height() - 150);
	
	QLabel desc1(PROG_FULLNAME, &about);
	desc1.setFont(font);
	desc1.setFixedWidth(200);
	desc1.setAlignment(Qt::AlignHCenter);

	font.setPointSize(8);

	QLabel desc2(tr("Par myst6re<br/><a href=\"https://sourceforge.net/projects/makoureactor/\">sourceforge.net/projects/makoureactor</a><br/><br/>Merci à :<ul style=\"margin:0\"><li>Squall78</li><li>Synergy Blades</li><li>Akari</li><li>Asa</li></ul>"), &about);
	desc2.setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	desc2.setTextFormat(Qt::RichText);
	desc2.setOpenExternalLinks(true);
	desc2.move(9, 40);
	desc2.setFont(font);

	QPushButton button(tr("Fermer"), &about);
	button.move(8, about.height()-8-button.sizeHint().height());
	connect(&button, SIGNAL(released()), &about, SLOT(close()));

	QLabel desc4(QString("Qt %1").arg(QT_VERSION_STR), &about);
	QPalette pal = desc4.palette();
	pal.setColor(QPalette::WindowText, QColor(0xAA,0xAA,0xAA));
	desc4.setPalette(pal);
	desc4.move(9, about.height()-16-desc4.sizeHint().height()-button.sizeHint().height());
	desc4.setFont(font);

	about.exec();
}
