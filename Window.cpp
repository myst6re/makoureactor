/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2013 Arzel Jérôme <myst6re@gmail.com>
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
#include "Parameters.h"
#include "core/field/GrpScript.h"
#include "widgets/ConfigWindow.h"
#include "widgets/EncounterWidget.h"
#include "widgets/MiscWidget.h"
#include "widgets/ImportDialog.h"
#include "widgets/MassExportDialog.h"
#include "widgets/MassImportDialog.h"
#include "widgets/FontManager.h"
#include "core/Config.h"
#include "Data.h"
#include "core/field/FieldArchivePC.h"
#include "core/field/FieldArchivePS.h"
#include "widgets/OperationsManager.h"

Window::Window() :
	fieldArchive(0), field(0), firstShow(true), varDialog(0),
	_textDialog(0), _modelManager(0), _tutManager(0), _walkmeshManager(0),
	_backgroundManager(0)
{
	qreal scale= qApp->desktop()->logicalDpiX()/96;
	setWindowTitle();
	setWindowState(Qt::WindowMaximized);

	taskBarButton = new QTaskBarButton(this);
	taskBarButton->setMinimum(0);

	progressDialog = new QProgressDialog(this, Qt::Dialog | Qt::WindowCloseButtonHint);
	progressDialog->setWindowModality(Qt::WindowModal);
	progressDialog->setAutoClose(false);
	progressDialog->hide();

	authorLbl = new QLabel();
	authorLbl->setMargin(2);
	authorLbl->setWordWrap(true);

	QWidget *toolBarRight = new QWidget();
	QHBoxLayout *toolBarRightLayout = new QHBoxLayout(toolBarRight);
	toolBarRightLayout->addStretch();
	toolBarRightLayout->addWidget(authorLbl, 0, Qt::AlignRight);
	toolBarRightLayout->setContentsMargins(QMargins());
	
	QMenu *menu;
	QAction *actionOpen, *actionFind, *action;
	QMenuBar *menuBar = new QMenuBar(0);
	
	/* Menu 'Fichier' */
	menu = menuBar->addMenu(tr("&Fichier"));
	
	actionOpen = menu->addAction(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton), tr("&Ouvrir..."), this, SLOT(openFile()), QKeySequence("Ctrl+O"));
	menu->addAction(tr("Ouvrir un &dossier..."), this, SLOT(openDir()), QKeySequence("Shift+Ctrl+O"));
	actionSave = menu->addAction(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton), tr("Enregi&strer"), this, SLOT(save()), QKeySequence("Ctrl+S"));
	actionSaveAs = menu->addAction(tr("Enre&gistrer Sous..."), this, SLOT(saveAs()), QKeySequence("Shift+Ctrl+S"));
	actionExport = menu->addAction(tr("&Exporter l'écran courant..."), this, SLOT(exporter()), QKeySequence("Ctrl+E"));
	actionMassExport = menu->addAction(tr("Exporter en &masse..."), this, SLOT(massExport()), QKeySequence("Shift+Ctrl+E"));
	actionImport = menu->addAction(tr("&Importer dans l'écran courant..."), this, SLOT(importer()), QKeySequence("Ctrl+I"));
//	actionMassImport = menu->addAction(tr("Importer en m&asse..."), this, SLOT(massImport()), QKeySequence("Shift+Ctrl+I"));
	actionArchive = menu->addAction(tr("Ges&tionnaire d'archive..."), this, SLOT(archiveManager()), QKeySequence("Ctrl+K"));
	menu->addSeparator();
	actionRun = menu->addAction(QIcon(":/images/ff7.png"), tr("Lancer FF7"), this, SLOT(runFF7()));
	actionRun->setShortcut(Qt::Key_F8);
	actionRun->setShortcutContext(Qt::ApplicationShortcut);
	actionRun->setEnabled(!Data::ff7AppPath().isEmpty());
	menu->addSeparator();
	actionClose = menu->addAction(QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton), tr("Fe&rmer"), this, SLOT(closeFile()));
	menu->addAction(tr("&Quitter"), this, SLOT(close()), QKeySequence::Quit)->setMenuRole(QAction::QuitRole);
	
	/* Menu 'Outils' */
	menu = menuBar->addMenu(tr("&Outils"));
	menu->addAction(tr("&Textes..."), this, SLOT(textManager()), QKeySequence("Ctrl+T"));
	actionModels = menu->addAction(tr("&Modèles 3D..."), this, SLOT(modelManager()), QKeySequence("Ctrl+M"));
	actionEncounter = menu->addAction(tr("&Rencontres aléatoires..."), this, SLOT(encounterManager()), QKeySequence("Ctrl+N"));
	menu->addAction(tr("T&utoriels/Musiques..."), this, SLOT(tutManager()), QKeySequence("Ctrl+Q"));
	menu->addAction(tr("&Zones..."), this, SLOT(walkmeshManager()), QKeySequence("Ctrl+W"));
	menu->addAction(tr("&Background..."), this, SLOT(backgroundManager()), QKeySequence("Ctrl+B"));
	actionMisc = menu->addAction(tr("&Divers..."), this, SLOT(miscManager()));
	menu->addSeparator();
	menu->addAction(tr("&Gestionnaire de variables..."), this, SLOT(varManager()), QKeySequence("Ctrl+G"));
	actionFind = menu->addAction(QIcon(":/images/find.png"), tr("Rec&hercher..."), this, SLOT(searchManager()), QKeySequence::Find);
	actionMiscOperations = menu->addAction(tr("Opér&ations diverses..."), this, SLOT(miscOperations()));
	//menu->addAction(tr("&Police de caractères..."), this, SLOT(fontManager()), QKeySequence("Ctrl+P"));

	menu = menuBar->addMenu(tr("&Paramètres"));

	actionJp_txt = menu->addAction(tr("Caractères &japonais"), this, SLOT(jpText(bool)));
	actionJp_txt->setCheckable(true);
	actionJp_txt->setChecked(Config::value("jp_txt", false).toBool());

	menuLang = menu->addMenu(tr("&Langues"));
	QDir dir(Config::programResourceDir());
	QStringList stringList = dir.entryList(QStringList("Makou_Reactor_*.qm"), QDir::Files, QDir::Name);
	action = menuLang->addAction(tr("Français (défaut)"));
	action->setData("fr");
	action->setCheckable(true);
	action->setChecked(Config::value("lang").toString()=="fr");

	menuLang->addSeparator();
	QTranslator translator;
	foreach(const QString &str, stringList) {
		translator.load(Config::programResourceDir()+"/"+str);
		action = menuLang->addAction(translator.translate("Window", "Français"));
		QString lang = str.mid(14, 2);
		action->setData(lang);
		action->setCheckable(true);
		action->setChecked(Config::value("lang").toString()==lang);
	}
	connect(menuLang, SIGNAL(triggered(QAction*)), this, SLOT(changeLanguage(QAction*)));

	menu->addAction(tr("&Configuration..."), this, SLOT(config()))->setMenuRole(QAction::PreferencesRole);

	toolBar = new QToolBar(tr("Barre d'outils &principale"));
	toolBar->setObjectName("toolbar");
	toolBar->setIconSize(QSize(16*scale, 16*scale));
	addToolBar(toolBar);
	toolBar->addAction(actionOpen);
	actionOpen->setStatusTip(tr("Ouvrir un fichier"));
	toolBar->addAction(actionSave);
	actionSave->setStatusTip(tr("Enregistrer"));
	toolBar->addSeparator();
	toolBar->addAction(actionFind);
	actionFind->setStatusTip(tr("Rechercher"));
	toolBar->addAction(actionRun);
	authorAction = toolBar->addWidget(toolBarRight);

	QFont font;
	font.setPointSize(8);

	lineSearch = new QLineEdit(this);
	lineSearch->setFixedWidth(120*scale);
	lineSearch->setStatusTip(tr("Recherche rapide"));
	lineSearch->setPlaceholderText(tr("Rechercher..."));
	
	fieldList = new QTreeWidget(this);
	fieldList->setColumnCount(2);
	fieldList->setHeaderLabels(QStringList() << tr("Fichier") << tr("Id"));
	fieldList->setFixedWidth(120*scale);
	fieldList->setMinimumHeight(120*scale);
	fieldList->setIndentation(0);
	fieldList->setItemsExpandable(false);
	fieldList->setSortingEnabled(true);
	fieldList->setColumnWidth(1,28*scale);
	fieldList->setAutoScroll(false);
	fieldList->resizeColumnToContents(0);
	fieldList->setFont(font);
	fieldList->sortByColumn(1, Qt::AscendingOrder);
	connect(fieldList, SIGNAL(itemSelectionChanged()), SLOT(openField()));

	groupScriptList = new GrpScriptList(this);
	groupScriptList->setFixedWidth(176*scale);
	groupScriptList->setMinimumHeight(176*scale);
	groupScriptList->setFont(font);
	connect(groupScriptList, SIGNAL(changed()), SLOT(setModified()));

	scriptList = new ScriptList(this);
	scriptList->setFixedWidth(88*scale);
	scriptList->setMinimumHeight(88*scale);
	scriptList->setFont(font);
	
	opcodeList = new OpcodeList(this);
	connect(opcodeList, SIGNAL(changed()), SLOT(setModified()));
	connect(opcodeList, SIGNAL(changed()), SLOT(refresh()));
	connect(opcodeList, SIGNAL(editText(int)), SLOT(textManager(int)));

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
	zoneImage->setFixedSize(300*scale, 225*scale);
	if(Config::value("OpenGL", true).toBool()) {
		fieldModel = new FieldModel();
		fieldModel->setFixedSize(300*scale, 225*scale);
//		modelThread = new FieldModelThread(this);

//		connect(modelThread, SIGNAL(modelLoaded(Field*,FieldModelFile*,int,int,bool)), SLOT(showModel(Field*,FieldModelFile*)));
	} else {
		fieldModel = 0;
	}

	zonePreview = new QStackedWidget(this);
	zonePreview->setContentsMargins(QMargins());
	zonePreview->setFixedSize(300*scale, 225*scale);
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
	menuBar->addMenu(createPopupMenu());
	menuBar->addAction("&?", this, SLOT(about()))->setMenuRole(QAction::AboutRole);
	
	setMenuBar(menuBar);

	connect(zoneImage, SIGNAL(clicked()), SLOT(backgroundManager()));
	connect(searchDialog, SIGNAL(found(int,int,int,int)), SLOT(gotoOpcode(int,int,int,int)));
	connect(searchDialog, SIGNAL(foundText(int,int,int,int)), SLOT(gotoText(int,int,int,int)));
	connect(lineSearch, SIGNAL(textEdited(QString)), SLOT(filterMap()));
	connect(lineSearch, SIGNAL(returnPressed()), SLOT(filterMap()));

	groupScriptList->toolBar()->setVisible(Config::value("grpToolbarVisible", true).toBool());
	opcodeList->toolBar()->setVisible(Config::value("scriptToolbarVisible", true).toBool());

	restoreState(Config::value("windowState").toByteArray());
	restoreGeometry(Config::value("windowGeometry").toByteArray());
	fieldList->setFocus();
	closeFile();
}

Window::~Window()
{
	Config::flush();
	if(fieldArchive)	fieldArchive->close();
}

void Window::closeEvent(QCloseEvent *event)
{
	if(!isEnabled() || closeFile(true)==QMessageBox::Cancel)	event->ignore();
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
	if(_textDialog) {
		_textDialog->updateText();
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

int Window::closeFile(bool quit)
{
	if(fieldList->currentItem() != NULL)
		Config::setValue("currentField", fieldList->currentItem()->text(0));

	if(actionSave->isEnabled() && fieldArchive!=NULL)
	{
		QString fileChangedList;
		int i=0;
		for(int j=0 ; j<fieldArchive->size() ; ++j) {
			Field *curField = fieldArchive->field(j, false);
			if(curField && curField->isOpen() && curField->isModified())
			{
				fileChangedList += "\n - " + curField->name();
				if(i>10)
				{
					fileChangedList += "\n...";
					break;
				}
				i++;
			}
		}
		if(!fileChangedList.isEmpty()) {
			fileChangedList.prepend(tr("\n\nFichiers modifiés :"));
		}
		int reponse = QMessageBox::warning(this, tr("Sauvegarder"), tr("Voulez-vous enregistrer les changements de %1 ?%2").arg(fieldArchive->io()->name()).arg(fileChangedList), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
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

		fieldList->blockSignals(true);
		groupScriptList->blockSignals(true);
		scriptList->blockSignals(true);
		opcodeList->blockSignals(true);
		fieldList->clear();
		groupScriptList->clear();
		groupScriptList->clearCopiedGroups();
		groupScriptList->enableActions(false);
		scriptList->clear();
		opcodeList->clear();
		opcodeList->clearCopiedOpcodes();
		fieldList->blockSignals(false);
		groupScriptList->blockSignals(false);
		scriptList->blockSignals(false);
		opcodeList->blockSignals(false);
		zoneImage->clear();
		if(fieldModel) {
			fieldModel->clear();
//			if(fieldArchive && fieldArchive->io()->isPC()) {
//				modelThread->cancel();
//				modelThread->wait();
//			}
		}
		zonePreview->setCurrentIndex(0);

		authorAction->setVisible(false);
		setWindowModified(false);
		setWindowTitle();
		searchDialog->setFieldArchive(NULL);
		if(_textDialog) {
			_textDialog->clear();
			_textDialog->setEnabled(false);
		}
		if(_modelManager) {
			_modelManager->close();
			_modelManager->deleteLater();
			_modelManager = 0;
		}
		if(_tutManager) {
			_tutManager->clear();
			_tutManager->setEnabled(false);
		}
		if(_walkmeshManager) {
			_walkmeshManager->clear();
			_walkmeshManager->setEnabled(false);
		}
		if(_backgroundManager) {
			_backgroundManager->clear();
			_backgroundManager->setEnabled(false);
		}
		
		actionSave->setEnabled(false);
		actionSaveAs->setEnabled(false);
		actionExport->setEnabled(false);
		actionMassExport->setEnabled(false);
//		actionMassImport->setEnabled(false);
		actionImport->setEnabled(false);
		actionArchive->setEnabled(false);
		actionClose->setEnabled(false);
		actionModels->setEnabled(false);
		actionEncounter->setEnabled(false);
		actionMisc->setEnabled(false);
		actionMiscOperations->setEnabled(false);
	}
	
	return QMessageBox::Yes;
}

void Window::openFile(const QString &path)
{
	QString filePath;

	if(path.isEmpty()) {
		filePath = Config::value("open_path").toString();
		if(filePath.isEmpty()) {
			filePath = Data::ff7DataPath();
			if(!filePath.isEmpty())
				filePath.append("/field");
		}
		QStringList filter;
		filter.append(tr("Fichiers compatibles (*.lgp *.DAT *.bin *.iso *.img)"));
		filter.append(tr("Fichiers Lgp (*.lgp)"));
		filter.append(tr("Fichier DAT (*.DAT)"));
		filter.append(tr("Fichier Field PC (*)"));
		filter.append(tr("Image disque (*.bin *.iso *.img)"));

		QString selectedFilter = filter.value(Config::value("open_path_selected_filter").toInt(), filter.first());

		filePath = QFileDialog::getOpenFileName(this, tr("Ouvrir un fichier"), filePath, filter.join(";;"), &selectedFilter);
		if(filePath.isNull())	{
			return;
		}

		int index = filePath.lastIndexOf('/');
		if(index == -1)	index = filePath.size();
		Config::setValue("open_path", filePath.left(index));
		Config::setValue("open_path_selected_filter", filter.indexOf(selectedFilter));
	} else {
		filePath = path;
	}

	bool isPS;
	FieldArchiveIO::Type type;
	QString ext = filePath.mid(filePath.lastIndexOf('.') + 1).toLower();

	if(ext == "iso" || ext == "bin" || ext == "img") {
		isPS = true;
		type = FieldArchiveIO::Iso;
	} else {
		if(ext == "dat") {
			isPS = true;
			type = FieldArchiveIO::File;
		} else if(ext == "lgp") {
			isPS = false;
			type = FieldArchiveIO::Lgp;
		} else {
			isPS = false;
			type = FieldArchiveIO::File;
		}
	}

	open(filePath, type, isPS);
}

void Window::openDir()
{
	QString filePath = QFileDialog::getExistingDirectory(this,
														  tr("Sélectionnez un dossier contenant des fichiers field issus de Final Fantasy VII"),
														  Config::value("open_dir_path").toString());
	if(filePath.isNull())	{
		return;
	}

	Config::setValue("open_dir_path", filePath);

	QMessageBox question(QMessageBox::Question, tr("Type de fichiers"),
						 tr("Quel type de fichiers voulez-vous chercher ?\n"
							" - Les fichiers field PlayStation (\"EXEMPLE.DAT\")\n"
							" - Les fichiers field PC (\"exemple\")\n"),
						 QMessageBox::NoButton, this);
	QAbstractButton *psButton = question.addButton(tr("PS"), QMessageBox::AcceptRole);
	QAbstractButton *pcButton = question.addButton(tr("PC"), QMessageBox::AcceptRole);
	question.addButton(QMessageBox::Cancel);
	question.exec();
	if(question.clickedButton() != psButton
			&& question.clickedButton() != pcButton) {
		return;
	}

	open(filePath, FieldArchiveIO::Dir, question.clickedButton() == psButton);
}

bool Window::observerWasCanceled() const
{
	return progressDialog->wasCanceled();
}

void Window::setObserverMaximum(unsigned int max)
{
	taskBarButton->setMaximum(max);
	progressDialog->setMaximum(max);
}

void Window::setObserverValue(int value)
{
	QApplication::processEvents();

	taskBarButton->setValue(value);
	progressDialog->setValue(value);
}

void Window::showProgression(const QString &message, bool canBeCanceled)
{
	setObserverValue(0);
	taskBarButton->setState(QTaskBarButton::Normal);
	progressDialog->setLabelText(message);
	progressDialog->setCancelButtonText(canBeCanceled ? tr("Annuler") : tr("Arrêter"));
	progressDialog->show();
}

void Window::hideProgression()
{
	taskBarButton->setState(QTaskBarButton::Invisible);
	progressDialog->hide();
	progressDialog->reset();
}

void Window::open(const QString &filePath, FieldArchiveIO::Type type, bool isPS)
{
	closeFile();

	if(isPS) {
		fieldArchive = new FieldArchivePS(filePath, type);
	} else {
		fieldArchive = new FieldArchivePC(filePath, type);
	}

	fieldArchive->setObserver(this);

	showProgression(tr("Ouverture..."), false);

	FieldArchiveIO::ErrorCode error = fieldArchive->open();

	hideProgression();

	QString out;
	switch(error)
	{
	case FieldArchiveIO::Ok:
	case FieldArchiveIO::Aborted:
		break;
	case FieldArchiveIO::FieldNotFound:
		if(fieldArchive->io()->type() == FieldArchiveIO::Lgp) {
			actionArchive->setEnabled(true);
			setWindowTitle();
			archiveManager();
			return;
		} else {
			out = tr("Rien trouvé !");
		}
		break;
	case FieldArchiveIO::ErrorOpening:
		out = tr("Le fichier est inaccessible");
		break;
	case FieldArchiveIO::ErrorOpeningTemp:
		out = tr("Impossible de créer un fichier temporaire");
		break;
	case FieldArchiveIO::ErrorRemoving:
		out = tr("Impossible de supprimer le fichier");
		break;
	case FieldArchiveIO::ErrorRenaming:
		out = tr("Impossible de renommer le fichier.");
		break;
	case FieldArchiveIO::ErrorCopying:
		out = tr("Impossible de copier le fichier");
		break;
	case FieldArchiveIO::Invalid:
		out = tr("Le fichier est invalide");
		break;
	case FieldArchiveIO::NotImplemented:
		out = tr("Cette erreur ne devrais pas s'afficher, merci de le signaler");
		break;
	}
	if(!out.isEmpty()) {
		QMessageBox::warning(this, tr("Erreur"), out);
		fieldArchive->close();
		return;
	}

	QList<QTreeWidgetItem *> items;

	for(int fieldID=0 ; fieldID<fieldArchive->size() ; ++fieldID) {
		Field *f = fieldArchive->field(fieldID, false);
		if(f) {
			const QString &name = f->name();

			QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << name << "");
			item->setData(0, Qt::UserRole, fieldID);
			items.append(item);

			int index;
			if((index = Data::field_names.indexOf(name)) != -1) {
				item->setText(1, QString("%1").arg(index, 3));
			} else {
				item->setText(1, "~");
			}
		}
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

	if(fieldArchive->size() > 0) {
		if(varDialog)	varDialog->setFieldArchive(fieldArchive);
		searchDialog->setFieldArchive(fieldArchive);
		actionEncounter->setEnabled(true);
		actionMisc->setEnabled(true);
		actionMiscOperations->setEnabled(true);
		actionExport->setEnabled(true);
		actionMassExport->setEnabled(true);
//		actionMassImport->setEnabled(true);
		actionImport->setEnabled(true);
		actionModels->setEnabled(true);
	}
	if(fieldArchive->io()->type() == FieldArchiveIO::Lgp) {
		actionArchive->setEnabled(true);
	}
	actionSaveAs->setEnabled(true);
	actionClose->setEnabled(true);

#ifdef DEBUG_FUNCTIONS
	//FieldArchivePC otherArch("C:/Users/Jérôme/Documents/neo_midgar_build/livraison-acro-2015-04-10/fflevel_compare_annexe.lgp", FieldArchiveIO::Lgp);
	//fieldArchive->compareTexts(&otherArch);
	fieldArchive->printTexts("field-texts.txt");
	fieldArchive->printAkaos("field-akaos.txt");
	fieldArchive->printModelLoaders("field-model-loaders-generic.txt");
	fieldArchive->printModelLoaders("field-model-loaders.txt", false);
	fieldArchive->printScripts("field-scripts.txt");
	//fieldArchive->searchAll();
#endif
}

void Window::setWindowTitle()
{
	QString windowTitle;
	if(fieldArchive) {
		QList<QTreeWidgetItem *> selectedItems = fieldList->selectedItems();
		QString current;
		if(!selectedItems.isEmpty()) {
			current = selectedItems.first()->text(0);
		}

		if(!fieldArchive->io()->hasName()) { // [*][current - ]PROG_NAME
			windowTitle = "[*]";

			if(!current.isEmpty()) {
				 windowTitle.append(current).append(" - ");
			}
		} else { // [current ](*archive) - PROG_NAME
			if(!current.isEmpty()) {
				 windowTitle.append(current).append(" (");
			}

			windowTitle.append("[*]").append(fieldArchive->io()->name());

			if(!current.isEmpty()) {
				 windowTitle.append(")");
			}

			windowTitle.append(" - ");
		}
	}

	QWidget::setWindowTitle(windowTitle.append(PROG_FULLNAME));
}

int Window::currentFieldId() const
{
	QList<QTreeWidgetItem *> selectedItems = fieldList->selectedItems();
	if(selectedItems.isEmpty())	return -1;

	return selectedItems.first()->data(0, Qt::UserRole).toInt();
}

int Window::currentGrpScriptId() const
{
	return groupScriptList->selectedID();
}

int Window::currentScriptId() const
{
	return scriptList->selectedID();
}

int Window::currentOpcodeId() const
{
	return opcodeList->selectedID();
}

void Window::openField(bool reload)
{
	if(!fieldArchive) return;

	disconnect(groupScriptList, SIGNAL(itemSelectionChanged()), this, SLOT(showGrpScripts()));
	disconnect(scriptList, SIGNAL(itemSelectionChanged()), this, SLOT(showScripts()));
	
	// Clear lists
	groupScriptList->clear();
	scriptList->clear();
	opcodeList->clear();
	
	int fieldId = currentFieldId();
	if(fieldId < 0)	return;
	fieldList->scrollToItem(fieldList->selectedItems().first());

//	Data::currentCharNames.clear();
	Data::currentHrcNames = 0;
	Data::currentAnimNames = 0;

	setWindowTitle();

//	if(fieldModel && fieldArchive->io()->isPC()) {
//		modelThread->cancel();
//		modelThread->wait();

//		if(_walkmeshManager)	_walkmeshManager->clear();
//	}

	if(field) {
		BackgroundFile *bgFile = field->background(false);
		if(!bgFile->isModified()) {
			bgFile->clear();
			bgFile->close();
		}
	}

	// Get and set field
	field = fieldArchive->field(fieldId, true, true);
	if(field == NULL) {
		zoneImage->clear();
		groupScriptList->setEnabled(false);
		scriptList->clear();
		scriptList->setEnabled(false);
		opcodeList->clear();
		opcodeList->setEnabled(false);
		return;
	}
//	if(fieldModel && fieldArchive->io()->isPC()) {
//		modelThread->setField(field);
//	}
	if(_textDialog && (reload || _textDialog->isVisible())) {
		_textDialog->setField(field, reload);
		_textDialog->setEnabled(true);
	}
	if(_modelManager && (reload || _modelManager->isVisible())) {
		_modelManager->fill(field, reload);
		_modelManager->setEnabled(true);
	}
	if(_tutManager && (reload || _tutManager->isVisible())) {
		TutFilePC *tutPC = NULL;
		if(fieldArchive->isPC()) {
			tutPC = ((FieldArchivePC *)fieldArchive)->tut(field->name());
		}
		_tutManager->fill(field, tutPC, reload);
		_tutManager->setEnabled(true);
	}
	if(_walkmeshManager && (reload || _walkmeshManager->isVisible())) {
		_walkmeshManager->fill(field, reload);
		_walkmeshManager->setEnabled(true);
	}
	if(_backgroundManager && (reload || _backgroundManager->isVisible())) {
		_backgroundManager->fill(field, reload);
		_backgroundManager->setEnabled(true);
	}

	// Show background preview
	zoneImage->fill(field, reload);
	zonePreview->setCurrentIndex(0);

	Section1File *scriptsAndTexts = field->scriptsAndTexts();

	if (scriptsAndTexts->isOpen()) {
		// Show author
		authorLbl->setText(tr("Auteur : %1").arg(scriptsAndTexts->author()));
		authorAction->setVisible(true);

		// Fill group script list
		groupScriptList->setEnabled(true);
		groupScriptList->fill(scriptsAndTexts);
	} else {
		groupScriptList->setEnabled(false);
	}

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
	
	scriptList->blockSignals(true);
	opcodeList->blockSignals(true);
	scriptList->clear();
	opcodeList->clear();
	scriptList->blockSignals(false);
	opcodeList->blockSignals(false);

	if(groupScriptList->selectedItems().isEmpty())
	{
		zonePreview->setCurrentIndex(0);
		return;
	}

	GrpScript *currentGrpScript = groupScriptList->currentGrpScript();
	if(currentGrpScript==NULL)	return;

	scriptList->fill(currentGrpScript);

	connect(scriptList, SIGNAL(itemSelectionChanged()), SLOT(showScripts()));
	scriptList->setCurrentRow(0);

	int modelID = field->scriptsAndTexts()->modelID(groupScriptList->selectedID());
	Data::currentModelID = modelID;
	if(fieldModel && modelID != -1) {
//		if(fieldArchive->io()->isPC()) {
//			modelThread->cancel();
//			modelThread->wait();
//			modelThread->setModel(modelID);
//			modelThread->start();
//		} else {
			showModel(field, field->fieldModel(modelID));
//		}
	} else {
		zonePreview->setCurrentIndex(0);
	}
}

void Window::showModel(Field *field, FieldModelFile *fieldModelFile)
{
	if(fieldModel && this->field == field) {
		fieldModel->setFieldModelFile(fieldModelFile);
	}
	zonePreview->setCurrentIndex(int(fieldModel && !fieldModelFile->isEmpty()));
}

void Window::showScripts()
{
	if(field == NULL) {
		opcodeList->clear();
		opcodeList->setEnabled(false);
		return;
	}

	opcodeList->setEnabled(true);

	opcodeList->clear();
	
	if(scriptList->selectedItems().isEmpty())	return;
	
	Script *currentScript = scriptList->currentScript();
	if(currentScript==NULL)	return;
	opcodeList->fill(field, groupScriptList->currentGrpScript(), currentScript);
	opcodeList->setIsInit(scriptList->selectedID()==0);
	opcodeList->scroll(0, false);
}

void Window::compile()
{
	Script *currentScript = scriptList->currentScript();
	if(currentScript==NULL)	return;

	int opcodeID;
	QString errorStr;

	if(!currentScript->compile(opcodeID, errorStr)) {
		compileScriptLabel->setText(tr("Erreur ligne %1 : %2").arg(opcodeID+1).arg(errorStr));
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

	int size=fieldList->topLevelItemCount();
	for(int i=0 ; i<size ; ++i) {
		QTreeWidgetItem *item = fieldList->topLevelItem(i);
		int fieldId = item->data(0, Qt::UserRole).toInt();
		if(fieldId >= 0) {
			Field *curField = fieldArchive->field(fieldId, false);
			if(curField && curField->isOpen()) {
				if(enabled && curField->isModified()) {
					item->setForeground(0, QColor(0xd1,0x1d,0x1d));
				} else if(!enabled && item->foreground(0).color() == QColor(0xd1,0x1d,0x1d)) {
					item->setForeground(0, QColor(0x1d,0xd1,0x1d));
				}
			}
		}
	}
}

void Window::save() { saveAs(true); }

void Window::saveAs(bool currentPath)
{
	if(!fieldArchive) return;

	int fieldID, groupID, scriptID, opcodeID;
	QString errorStr;

	setEnabled(false);
	bool compiled = fieldArchive->compileScripts(fieldID, groupID, scriptID, opcodeID, errorStr);
	setEnabled(true);

	if(!compiled) {
		QMessageBox::warning(this, tr("Erreur de compilation"), tr("Erreur de compilation des scripts :\n"
																   "écran %1 (%2), groupe %3 (%4), script %5, ligne %6 : %7")
							 .arg(fieldArchive->field(fieldID)->name())
							 .arg(fieldID)
							 .arg(fieldArchive->field(fieldID)->scriptsAndTexts()->grpScript(groupID)->name())
							 .arg(groupID).arg(scriptID)
							 .arg(opcodeID+1).arg(errorStr));
		gotoOpcode(fieldID, groupID, scriptID, opcodeID);
		opcodeList->setErrorLine(opcodeID);
		return;
	}

	QString path;
	if(!currentPath)
	{
		if(fieldArchive->io()->type() == FieldArchiveIO::Dir) {
			path = QFileDialog::getExistingDirectory(this, tr("Enregistrer dossier sous"), fieldArchive->io()->path());
			if(path.isNull())		return;
		} else {
			QString filter;
			if(fieldArchive->io()->type() == FieldArchiveIO::Lgp) {
				filter = tr("Fichier Lgp (*.lgp)");
			} else if(fieldArchive->io()->type() == FieldArchiveIO::File) {
				filter = tr("Fichier DAT (*.DAT)");
			} else if(fieldArchive->io()->type() == FieldArchiveIO::Iso) {
				filter = tr("Fichier Iso (*.iso *.bin *.img)");
			} else {
				return;
			}
			path = QFileDialog::getSaveFileName(this, tr("Enregistrer sous"), fieldArchive->io()->path(), filter);
			if(path.isNull())		return;
		}
	}

	showProgression(tr("Enregistrement..."), fieldArchive->io()->type() == FieldArchiveIO::Lgp);
	quint8 error = 0;
	
	// QTime t;t.start();
	error = fieldArchive->save(path);
	// qDebug("Total save time: %d ms", t.elapsed());

	hideProgression();
	QString out;
	switch(error)
	{
	case FieldArchiveIO::Ok:
		setModified(false);
		setWindowTitle();
		break;
	case FieldArchiveIO::Aborted:
		break;
	case FieldArchiveIO::FieldNotFound:
		out = tr("Rien trouvé !");
		break;
	case FieldArchiveIO::ErrorOpening:
		out = tr("Le fichier est inaccessible");
		break;
	case FieldArchiveIO::ErrorOpeningTemp:
		out = tr("Impossible de créer un fichier temporaire");
		break;
	case FieldArchiveIO::ErrorRemoving:
		out = tr("Impossible de supprimer le fichier, vérifiez les droits d'écriture.");
		break;
	case FieldArchiveIO::ErrorRenaming:
		out = tr("Impossible de renommer le fichier, vérifiez les droits d'écriture.");
		break;
	case FieldArchiveIO::ErrorCopying:
		out = tr("Impossible de copier le fichier, vérifiez les droits d'écriture.");
		break;
	case FieldArchiveIO::Invalid:
		out = tr("L'archive est invalide");
		break;
	case FieldArchiveIO::NotImplemented:
		out = tr("Cette fonctionnalité n'est pas terminée");
		break;
	}
	if(!out.isEmpty())	QMessageBox::warning(this, tr("Erreur"), out);
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
	if(_textDialog) {
		_textDialog->blockSignals(true);
	}
	if(gotoField(fieldID)) {
		textManager(textID, from, size, false); // show texts dialog
	}
	if(_textDialog) {
		_textDialog->blockSignals(false);
	}
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
	if(!field || !fieldArchive) return;

	int index;
	QString types, name, selectedFilter,
			fieldLzs = tr("Écran PC (*)"),
			dat = tr("Fichier DAT (*.DAT)"),
			fieldDec = tr("Écran PC décompressé (*)");

	name = fieldList->selectedItems().first()->text(0);

	if(fieldArchive->io()->isPC()) {
		types = fieldLzs+";;"+fieldDec;
	} else {
		types = dat;
		name = name.toUpper();
	}

	QString path = Config::value("exportPath").toString().isEmpty() ? fieldArchive->io()->directory() : Config::value("exportPath").toString()+"/";
	path = QFileDialog::getSaveFileName(this, tr("Exporter le fichier courant"), path+name, types, &selectedFilter);
	if(path.isNull())		return;
	qint8 error=4;
	bool compressed = selectedFilter != fieldDec;
	
	if (field->isModified()) {
		error = field->save(path, compressed);
	} else {
		error = fieldArchive->io()->exportFieldData(field, QString(), path, !compressed);
	}
	
	QString out;
	switch(error)
	{
	case 0:
		index = path.lastIndexOf('/');
		Config::setValue("exportPath", index == -1 ? path : path.left(index));
		break;
	case 1:	out = tr("L'archive Lgp est inaccessible");break;
	case 2:	out = tr("Erreur lors de l'ouverture du fichier");break;
	case 3:	out = tr("Impossible de créer le nouveau fichier");break;
	case 4:	out = tr("Pas encore implémenté !");break;
	}
	if(!out.isEmpty())	QMessageBox::warning(this, tr("Erreur"), out);
}

void Window::massExport()
{
	if(!fieldArchive) return;

	MassExportDialog *massExportDialog = new MassExportDialog(this);
	massExportDialog->fill(fieldArchive, currentFieldId());
	if(massExportDialog->exec() == QDialog::Accepted) {
		QList<int> selectedFields = massExportDialog->selectedFields();
		if(!selectedFields.isEmpty()) {
			QMap<FieldArchive::ExportType, QString> toExport;

			showProgression(tr("Exportation..."), false);

			if(massExportDialog->exportModule(MassExportDialog::Fields)) {
				toExport.insert(FieldArchive::Fields, massExportDialog->moduleFormat(MassExportDialog::Fields));
			}
			if(massExportDialog->exportModule(MassExportDialog::Backgrounds)) {
				toExport.insert(FieldArchive::Backgrounds, massExportDialog->moduleFormat(MassExportDialog::Backgrounds));
			}
			if(massExportDialog->exportModule(MassExportDialog::Akaos)) {
				toExport.insert(FieldArchive::Akaos, massExportDialog->moduleFormat(MassExportDialog::Akaos));
			}
			if(massExportDialog->exportModule(MassExportDialog::Texts)) {
				toExport.insert(FieldArchive::Texts, massExportDialog->moduleFormat(MassExportDialog::Texts));
			}

			if(!fieldArchive->exportation(selectedFields, massExportDialog->directory(),
									  massExportDialog->overwrite(), toExport)
					&& !observerWasCanceled()) {
				QMessageBox::warning(this, tr("Erreur"), tr("Une erreur s'est produite lors de l'exportation"));
			}

			hideProgression();
		}
	}
}

void Window::massImport()
{
	if(!fieldArchive) return;

	MassImportDialog *massImportDialog = new MassImportDialog(this);
	massImportDialog->fill(fieldArchive, currentFieldId());
	if(massImportDialog->exec() == QDialog::Accepted) {
		QList<int> selectedFields = massImportDialog->selectedFields();
		if(!selectedFields.isEmpty()) {
			QMap<Field::FieldSection, QString> toImport;

			showProgression(tr("Importation..."), false);

			if(massImportDialog->importModule(MassImportDialog::Texts)) {
				toImport.insert(Field::Scripts, massImportDialog->moduleFormat(MassImportDialog::Texts));
			}

			if(!fieldArchive->importation(selectedFields, massImportDialog->directory(),
									  toImport)
					&& !observerWasCanceled()) {
				QMessageBox::warning(this, tr("Erreur"), tr("Une erreur s'est produite lors de l'importation"));
			}

			hideProgression();
		}
	}
}

void Window::importer()
{
	if(!field) return;

	int index;
	QString name, selectedFilter;
	QStringList filter;
	filter << tr("Fichier DAT (*.DAT)")
		   << tr("Écran PC (*)")
		   << tr("Fichier DAT décompressé (*)")
		   << tr("Écran PC décompressé (*)");

	name = fieldList->selectedItems().first()->text(0);
	if(fieldArchive->io()->isPS())
		name = name.toUpper();

	QString path = Config::value("importPath").toString().isEmpty() ? fieldArchive->io()->directory() : Config::value("importPath").toString()+"/";
	path = QFileDialog::getOpenFileName(this, tr("Importer un fichier"), path+name, filter.join(";;"), &selectedFilter);
	if(path.isNull())		return;

	bool isDat = selectedFilter == filter.at(0) || selectedFilter == filter.at(2);
	bool isCompressed = selectedFilter == filter.at(0) || selectedFilter == filter.at(1);

	ImportDialog dialog((isDat && fieldArchive->io()->isPS())
						|| (!isDat && fieldArchive->io()->isPC()),
						isDat, path, this);
	if(dialog.exec() != QDialog::Accepted) {
		return;
	}

	Field::FieldSections parts = dialog.parts();
	if(parts == 0) {
		return;
	}

	if(parts.testFlag(Field::Background)) {
		QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Attention"), tr("L'algorithme d'importation des décors "
													   "donne de mauvais résultats en jeu, "
													   "vous êtes prévenus !"), QMessageBox::Ok | QMessageBox::Cancel);
		if(button != QMessageBox::Ok) {
			return;
		}
	}

	QFile addDevice(dialog.additionalPath());
	
	qint8 error = field->importer(path, isDat, isCompressed, parts, &addDevice);

	QString out;
	switch(error)
	{
	case 0:
		setModified();
		index = path.lastIndexOf('/');
		Config::setValue("importPath", index == -1 ? path : path.left(index));
		openField(true);
		break;
	case 1:	out = tr("Erreur lors de l'ouverture du fichier");	break;
	case 2:	out = tr("Le fichier est invalide");				break;
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
	QStringList args;

#ifndef Q_OS_WIN // For others systems like Linux, we try to launch ff7 with WINE
	args.append(FF7Exe);
	FF7Exe = "wine";
#else
	FF7Exe = "\"" % FF7Exe % "\"";
#endif

	if(!QProcess::startDetached(FF7Exe, args, FF7ExeDir)) {
		QMessageBox::warning(this, tr("Erreur"), tr("Final Fantasy VII n'a pas pu être lancé.\n%1")
							 .arg(QDir::toNativeSeparators(FF7Exe)));
	}
}

void Window::searchManager()
{
	if(_textDialog && _textDialog->isVisible()) {
		QString selectedText = _textDialog->selectedText();
		if(!selectedText.isEmpty()) {
			searchDialog->setText(selectedText);
		}
	}
	searchDialog->setOpcode(opcodeList->selectedOpcode());
	searchDialog->setScriptExec(groupScriptList->selectedID(), scriptList->selectedID()-1);
	searchDialog->show();
	searchDialog->activateWindow();
	searchDialog->raise();
}

void Window::textManager(int textID, int from, int size, bool activate)
{
	if(!_textDialog) {
		_textDialog = new TextManager(this);
		connect(_textDialog, SIGNAL(modified()), SLOT(setModified()));
	}

	if(field && field->scriptsAndTexts()->isOpen()) {
		_textDialog->setField(field);
		_textDialog->setEnabled(true);
	} else {
		_textDialog->clear();
		_textDialog->setEnabled(false);
	}
	_textDialog->show();
	if(activate) {
		_textDialog->activateWindow();
	} else {
		searchDialog->raise();
	}
	if(textID >= 0) {
		_textDialog->gotoText(textID, from, size);
	}
}

void Window::modelManager()
{
	if(!_modelManager) {
		if(!field)	return;
		if(field->isPC()) {
			_modelManager = new ModelManagerPC(fieldModel, this);
		} else {
			_modelManager = new ModelManagerPS(fieldModel, this);
		}
		connect(_modelManager, SIGNAL(modified()), SLOT(setModified()));
	}

	if(field) {
//		zonePreview->setCurrentIndex(0);
//		if(fieldModel) {
//			fieldModel->clear();
//		}

		_modelManager->fill(field);
		_modelManager->setEnabled(true);

//		bool modelLoaded = false;
//		if(fieldModel && Data::currentModelID != -1) {
//			modelLoaded = fieldModel->load(field, Data::currentModelID);
//		}
//		zonePreview->setCurrentIndex((int)modelLoaded);
	} else {
		_modelManager->clear();
		_modelManager->setEnabled(false);
	}
	_modelManager->show();
	_modelManager->activateWindow();
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
					setModified();
			}
		} else {
			QMessageBox::warning(this, tr("Erreur d'ouverture"), tr("Impossible d'ouvrir les combats aléatoires !"));
		}
	}
}

void Window::tutManager()
{
	if(!_tutManager) {
		_tutManager = new TutWidget(this);
		connect(_tutManager, SIGNAL(modified()), SLOT(setModified()));
	}

	if(field && field->tutosAndSounds()->isOpen()) {
		TutFilePC *tutPC = NULL;
		if(fieldArchive->isPC()) {
			tutPC = ((FieldArchivePC *)fieldArchive)->tut(field->name());
		}
		_tutManager->fill(field, tutPC);
		_tutManager->setEnabled(true);
	} else {
		_tutManager->clear();
		_tutManager->setEnabled(false);
	}
	_tutManager->show();
	_tutManager->activateWindow();
}

void Window::walkmeshManager()
{
	if(!_walkmeshManager) {
		_walkmeshManager = new WalkmeshManager(this);
		connect(_walkmeshManager, SIGNAL(modified()), SLOT(setModified()));
	}

	if(field) {
		_walkmeshManager->fill(field);
		_walkmeshManager->setEnabled(true);
	} else {
		_walkmeshManager->clear();
		_walkmeshManager->setEnabled(false);
	}
	_walkmeshManager->show();
	_walkmeshManager->activateWindow();
}

void Window::backgroundManager()
{
	if(!_backgroundManager) {
		_backgroundManager = new BGDialog(this);
	}

	if(field) {
		_backgroundManager->fill(field);
		_backgroundManager->setEnabled(true);
	} else {
		_backgroundManager->clear();
		_backgroundManager->setEnabled(false);
	}
	_backgroundManager->show();
	_backgroundManager->activateWindow();
}

void Window::miscManager()
{
	if(field) {
		InfFile *inf = field->inf();
		if(inf->isOpen()) {
			MiscWidget dialog(inf, field, this);
			if(dialog.exec()==QDialog::Accepted)
			{
				if(inf->isModified() || field->isModified()) {
					setModified();
					authorLbl->setText(tr("Auteur : %1").arg(field->scriptsAndTexts()->author()));
				}
			}
		} else {
			QMessageBox::warning(this, tr("Erreur d'ouverture"), tr("Impossible d'ouvrir les infos diverses !"));
		}
	}
}

void Window::archiveManager()
{
	if(fieldArchive && fieldArchive->io()->type() == FieldArchiveIO::Lgp) {
		LgpDialog dialog((Lgp *)fieldArchive->io()->device(), this);
		connect(&dialog, SIGNAL(modified()), SLOT(setModified()));
		dialog.exec();
	}
}

void Window::miscOperations()
{
	if(!fieldArchive) {
		return;
	}

	OperationsManager dialog(fieldArchive->isPC(), this);
	if(dialog.exec() == QDialog::Accepted) {
		OperationsManager::Operations operations = dialog.selectedOperations();

		showProgression(tr("Application en cours..."), false);

		if(operations.testFlag(OperationsManager::CleanUnusedTexts)) {
			fieldArchive->cleanTexts();
		}
		if(!observerWasCanceled() && operations.testFlag(OperationsManager::RemoveTexts)) {
			fieldArchive->removeTexts();
		}
		if(!observerWasCanceled() && operations.testFlag(OperationsManager::RemoveBattles)) {
			fieldArchive->removeBattles();
		}
		if(!observerWasCanceled() && fieldArchive->isPC() && operations.testFlag(OperationsManager::CleanModelLoaderPC)) {
			((FieldArchivePC *)fieldArchive)->cleanModelLoader();
		}
		if(!observerWasCanceled() && fieldArchive->isPC() && operations.testFlag(OperationsManager::RemoveUnusedSectionPC)) {
			((FieldArchivePC *)fieldArchive)->removeUnusedSections();
		}

		hideProgression();

		if(fieldArchive->isModified()) {
			setModified();
		}
	}
}

void Window::fontManager()
{
	FontManager dialog(this);
	dialog.exec();
}

void Window::config()
{
	ConfigWindow configWindow(this);
	if(configWindow.exec() == QDialog::Accepted) {
		actionJp_txt->setChecked(Config::value("jp_txt", false).toBool());
		if(_textDialog) {
			_textDialog->updateText();
		}
		actionRun->setEnabled(!Data::ff7AppPath().isEmpty());
		showScripts();
	}
}

void Window::about()
{
	qreal scale = qApp->desktop()->logicalDpiX()/96;

	QDialog about(this, Qt::Dialog | Qt::CustomizeWindowHint);
	about.setFixedSize(200*scale, 270*scale);

	QFont font;
	font.setPointSize(12);

	QLabel image(&about);
	image.setScaledContents(true);
	image.setPixmap(QPixmap(":/images/reactor.png"));
	image.move(82*scale, about.height() - 150*scale);
	
	QLabel desc1(PROG_FULLNAME, &about);
	desc1.setFont(font);
	desc1.setFixedWidth(about.width());
	desc1.setAlignment(Qt::AlignHCenter);

	font.setPointSize(8);

	QLabel desc2(tr("Par myst6re<br/><a href=\"https://github.com/myst6re/makoureactor/\">github.com/myst6re/makoureactor</a><br/><br/>Merci à :<ul style=\"margin:0\"><li>Squall78</li><li>Synergy Blades</li><li>Akari</li><li>Asa</li><li>Aali</li></ul>"), &about);
	desc2.setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	desc2.setTextFormat(Qt::RichText);
	desc2.setOpenExternalLinks(true);
	desc2.move(9*scale, 40*scale);
	desc2.setFont(font);

	QPushButton button(tr("Fermer"), &about);
	button.move(8*scale, about.height()-8*scale-button.sizeHint().height());
	connect(&button, SIGNAL(released()), &about, SLOT(close()));

	QLabel desc4(QString("Qt %1").arg(QT_VERSION_STR), &about);
	QPalette pal = desc4.palette();
	pal.setColor(QPalette::WindowText, QColor(0xAA,0xAA,0xAA));
	desc4.setPalette(pal);
	desc4.move(9*scale, about.height()-16*scale-desc4.sizeHint().height()-button.sizeHint().height());
	desc4.setFont(font);

	about.exec();
}
