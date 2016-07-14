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
	_backgroundManager(0), _progressDialog(0)
{
	setWindowTitle();
	setWindowState(Qt::WindowMaximized);

	taskBarButton = new QTaskBarButton(this);
	taskBarButton->setMinimum(0);

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

	/* "File" Menu */
	menu = menuBar->addMenu(tr("&File"));

	actionOpen = menu->addAction(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton), tr("&Open..."), this, SLOT(openFile()), QKeySequence("Ctrl+O"));
	menu->addAction(tr("Open &Directory..."), this, SLOT(openDir()), QKeySequence("Shift+Ctrl+O"));
	_recentMenu = new QMenu(tr("&Recent files"), this);
	fillRecentMenu();
	connect(_recentMenu, SIGNAL(triggered(QAction*)), SLOT(openRecentFile(QAction*)));
	menu->addMenu(_recentMenu);
	actionSave = menu->addAction(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton), tr("&Save"), this, SLOT(save()), QKeySequence("Ctrl+S"));
	actionSaveAs = menu->addAction(tr("Save &As..."), this, SLOT(saveAs()), QKeySequence("Shift+Ctrl+S"));
	actionExport = menu->addAction(tr("&Export the current field..."), this, SLOT(exporter()), QKeySequence("Ctrl+E"));
	actionMassExport = menu->addAction(tr("&Mass Export..."), this, SLOT(massExport()), QKeySequence("Shift+Ctrl+E"));
	actionImport = menu->addAction(tr("&Import the current field..."), this, SLOT(importer()), QKeySequence("Ctrl+I"));
//	actionMassImport = menu->addAction(tr("Importer en m&asse..."), this, SLOT(massImport()), QKeySequence("Shift+Ctrl+I"));
	actionArchive = menu->addAction(tr("Archive Mana&ger..."), this, SLOT(archiveManager()), QKeySequence("Ctrl+K"));
	menu->addSeparator();
	actionRun = menu->addAction(QIcon(":/images/ff7.png"), tr("R&un FF7"), this, SLOT(runFF7()));
	actionRun->setShortcut(Qt::Key_F8);
	actionRun->setShortcutContext(Qt::ApplicationShortcut);
	actionRun->setEnabled(!Data::ff7AppPath().isEmpty());
	menu->addSeparator();
	actionClose = menu->addAction(QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton), tr("C&lose"), this, SLOT(closeFile()));
	menu->addAction(tr("E&xit"), this, SLOT(close()), QKeySequence::Quit)->setMenuRole(QAction::QuitRole);

	/* "Tools" Menu */
	menu = menuBar->addMenu(tr("T&ools"));
	menu->addAction(tr("&Texts..."), this, SLOT(textManager()), QKeySequence("Ctrl+T"));
	actionModels = menu->addAction(tr("Field &Models..."), this, SLOT(modelManager()), QKeySequence("Ctrl+M"));
	actionEncounter = menu->addAction(tr("Encounte&rs..."), this, SLOT(encounterManager()), QKeySequence("Ctrl+N"));
	menu->addAction(tr("Tutorials/&Sounds..."), this, SLOT(tutManager()), QKeySequence("Ctrl+Q"));
	menu->addAction(tr("&Walkmesh..."), this, SLOT(walkmeshManager()), QKeySequence("Ctrl+W"));
	menu->addAction(tr("&Background..."), this, SLOT(backgroundManager()), QKeySequence("Ctrl+B"));
	actionMisc = menu->addAction(tr("M&iscellaneous..."), this, SLOT(miscManager()));
	menu->addSeparator();
	menu->addAction(tr("Variable Mana&ger..."), this, SLOT(varManager()), QKeySequence("Ctrl+G"));
	actionFind = menu->addAction(QIcon(":/images/find.png"), tr("&Find..."), this, SLOT(searchManager()), QKeySequence::Find);
	actionMiscOperations = menu->addAction(tr("Miscellaneous Oper&ations..."), this, SLOT(miscOperations()));
	//menu->addAction(tr("&Police de caractères..."), this, SLOT(fontManager()), QKeySequence("Ctrl+P"));

	/* "Settings" Menu */
	menu = menuBar->addMenu(tr("&Settings"));

	actionJp_txt = menu->addAction(tr("&Japanese Characters"), this, SLOT(jpText(bool)));
	actionJp_txt->setCheckable(true);
	actionJp_txt->setChecked(Config::value("jp_txt", false).toBool());

	menuLang = menu->addMenu(tr("&Language"));
	QDir dir(Config::programResourceDir());
	QStringList stringList = dir.entryList(QStringList("Makou_Reactor_*.qm"), QDir::Files, QDir::Name);
	action = menuLang->addAction(tr("English (default)"));
	action->setData("en");
	action->setCheckable(true);
	action->setChecked(Config::value("lang").toString()=="en");

	menuLang->addSeparator();
	QTranslator translator;
	foreach(const QString &str, stringList) {
		translator.load(dir.filePath(str));
		action = menuLang->addAction(translator.translate("Window", "English"));
		QString lang = str.mid(14, 2);
		action->setData(lang);
		action->setCheckable(true);
		action->setChecked(Config::value("lang").toString()==lang);
	}
	connect(menuLang, SIGNAL(triggered(QAction*)), this, SLOT(changeLanguage(QAction*)));

	menu->addAction(tr("&Configuration..."), this, SLOT(config()))->setMenuRole(QAction::PreferencesRole);

	/* Toolbar */
	toolBar = new QToolBar(tr("Main &toolbar"));
	toolBar->setObjectName("toolbar");
	toolBar->setIconSize(QSize(16, 16));
	addToolBar(toolBar);
	toolBar->addAction(actionOpen);
	actionOpen->setStatusTip(tr("Open a file"));
	toolBar->addAction(actionSave);
	actionSave->setStatusTip(tr("Save"));
	toolBar->addSeparator();
	toolBar->addAction(actionFind);
	actionFind->setStatusTip(tr("Find"));
	toolBar->addAction(actionRun);
	authorAction = toolBar->addWidget(toolBarRight);

	fieldList = new FieldList(this);

	zoneImage = new ApercuBG();
	if(Config::value("OpenGL", true).toBool()) {
		fieldModel = new FieldModel();
//		modelThread = new FieldModelThread(this);

//		connect(modelThread, SIGNAL(modelLoaded(Field*,FieldModelFile*,int,int,bool)), SLOT(showModel(Field*,FieldModelFile*)));
	} else {
		fieldModel = 0;
	}

	zonePreview = new QStackedWidget(this);
	zonePreview->addWidget(zoneImage);
	if(fieldModel) {
		zonePreview->addWidget(fieldModel);
	}

	QWidget *fullFieldList = new QWidget(this);
	QVBoxLayout *fieldListLayout = new QVBoxLayout(fullFieldList);
	fieldListLayout->addWidget(fieldList, 1);
	fieldListLayout->addWidget(fieldList->lineSearch());
	zonePreview->setContentsMargins(fieldListLayout->contentsMargins());

	horizontalSplitter = new Splitter(Qt::Vertical, this);
	horizontalSplitter->addWidget(fullFieldList);
	horizontalSplitter->addWidget(zonePreview);
	horizontalSplitter->setStretchFactor(0, 10);
	horizontalSplitter->setStretchFactor(1, 2);
	horizontalSplitter->setCollapsible(0, false);
	horizontalSplitter->restoreState(Config::value("horizontalSplitterState").toByteArray());
	horizontalSplitter->setCollapsed(1, !Config::value("backgroundVisible", true).toBool());

	_scriptManager = new ScriptManager(this);

	verticalSplitter = new Splitter(Qt::Horizontal, this);
	verticalSplitter->addWidget(horizontalSplitter);
	verticalSplitter->addWidget(_scriptManager);
	verticalSplitter->setStretchFactor(0, 3);
	verticalSplitter->setStretchFactor(1, 9);
	verticalSplitter->setCollapsible(1, false);
	verticalSplitter->restoreState(Config::value("verticalSplitterState").toByteArray());
	verticalSplitter->setCollapsed(0, !Config::value("fieldListVisible", true).toBool());

	setCentralWidget(verticalSplitter);

	searchDialog = new Search(this);
	menuBar->addMenu(createPopupMenu());
	menuBar->addAction("&?", this, SLOT(about()))->setMenuRole(QAction::AboutRole);

	setMenuBar(menuBar);

	connect(fieldList, SIGNAL(itemSelectionChanged()), SLOT(openField()));
	connect(zoneImage, SIGNAL(clicked()), SLOT(backgroundManager()));
	connect(searchDialog, SIGNAL(found(int,int,int,int)), SLOT(gotoOpcode(int,int,int,int)));
	connect(searchDialog, SIGNAL(foundText(int,int,int,int)), SLOT(gotoText(int,int,int,int)));
	connect(_scriptManager, SIGNAL(groupScriptCurrentChanged(int)), SLOT(showModel(int)));
	connect(_scriptManager, SIGNAL(editText(int)), SLOT(textManager(int)));
	connect(_scriptManager, SIGNAL(changed()), SLOT(setModified()));

	fieldList->sortItems(Config::value("fieldListSortColumn").toInt(),
	                     Qt::SortOrder(Config::value("fieldListSortOrder").toBool()));

	restoreState(Config::value("windowState").toByteArray());
	restoreGeometry(Config::value("windowGeometry").toByteArray());

	closeFile();
}

Window::~Window()
{
	Config::flush();
	if(fieldArchive) {
		fieldArchive->close();
	}
}

QProgressDialog *Window::progressDialog()
{
	if (!_progressDialog) {
		_progressDialog = new QProgressDialog(this, Qt::Dialog | Qt::WindowCloseButtonHint);
		_progressDialog->setWindowModality(Qt::WindowModal);
		_progressDialog->setAutoClose(false);
		_progressDialog->hide();
	}
	return _progressDialog;
}

void Window::closeEvent(QCloseEvent *event)
{
	if(!isEnabled() || closeFile(true) == QMessageBox::Cancel) {
		event->ignore();
	} else {
		Config::setValue("windowState", saveState());
		Config::setValue("windowGeometry", saveGeometry());
		Config::setValue("horizontalSplitterState", horizontalSplitter->saveState());
		Config::setValue("verticalSplitterState", verticalSplitter->saveState());
		Config::setValue("fieldListVisible", !verticalSplitter->isCollapsed(0));
		Config::setValue("backgroundVisible", !horizontalSplitter->isCollapsed(1));
		Config::setValue("fieldListSortColumn", fieldList->sortColumn());
		Config::setValue("fieldListSortOrder", int(fieldList->header()->sortIndicatorOrder()));
		_scriptManager->saveConfig();
		if(_walkmeshManager) {
			_walkmeshManager->saveConfig();
		}
		event->accept();
	}
}

QMenu *Window::createPopupMenu()
{
	QMenu *menu = new QMenu(tr("&View"), this);
	menu->addAction(toolBar->toggleViewAction());
	QAction *action;
	action = menu->addAction(tr("Field List"), this, SLOT(toggleFieldList()));
	action->setCheckable(true);
	action->setChecked(!verticalSplitter->isCollapsed(0));
	action = menu->addAction(tr("Background Preview"), this, SLOT(toggleBackgroundPreview()));
	action->setCheckable(true);
	action->setChecked(!horizontalSplitter->isCollapsed(1));
	menu->addSeparator();
	foreach(QAction *action, _scriptManager->actions()) {
		menu->addAction(action);
	}
	return menu;
}

void Window::fillRecentMenu()
{
	_recentMenu->clear();

	foreach(const QString &recentFile, Config::value("recentFiles").toStringList()) {
		_recentMenu->addAction(QDir::toNativeSeparators(recentFile));
	}

	if(_recentMenu->actions().isEmpty()) {
		_recentMenu->setDisabled(true);
	}
}

void Window::toggleFieldList()
{
	verticalSplitter->toggleCollapsed(0);
}

void Window::toggleBackgroundPreview()
{
	horizontalSplitter->toggleCollapsed(1);
}

FieldArchive::Sorting Window::getFieldSorting()
{
	return fieldList->getFieldSorting();
}

void Window::jpText(bool enabled)
{
	Config::setValue("jp_txt", enabled);
	if(_textDialog) {
		_textDialog->updateText();
	}
	_scriptManager->fillOpcodes();
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
	QString title, text;
	QTranslator translator;
	if(translator.load(QString("Makou_Reactor_%1")
	                   .arg(Config::value("lang").toString()),
	                   qApp->applicationDirPath())) {
		title = translator.translate("Window", "Settings changed");
		text = translator.translate("Window", "Restart the program for the settings to take effect.");
	} else {
		title = "Settings changed";
		text = "Restart the program for the settings to take effect.";
	}
	QMessageBox::information(this, title, text);
}

int Window::closeFile(bool quit)
{
	if(fieldList->currentItem() != NULL) {
		Config::setValue("currentField", fieldList->currentItem()->text(0));
	}

	if(actionSave->isEnabled() && fieldArchive != NULL) {
		QString fileChangedList;
		int i = 0;
		for(int j = 0 ; j < fieldArchive->size() ; ++j) {
			Field *curField = fieldArchive->field(j, false);
			if(curField && curField->isOpen() && curField->isModified()) {
				fileChangedList += "\n - " + curField->name();
				if(i > 10) {
					fileChangedList += "\n...";
					break;
				}
				i++;
			}
		}
		if(!fileChangedList.isEmpty()) {
			fileChangedList.prepend(tr("\n\nEdited files:"));
		}
		int reponse = QMessageBox::warning(this, tr("Save"), tr("Would you like to save changes of %1?%2").arg(fieldArchive->io()->name()).arg(fileChangedList), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if(reponse == QMessageBox::Yes) {
			save();
		} else if(reponse == QMessageBox::Cancel) {
			return reponse;
		}
		if(quit) {
			return reponse;
		}
	}

	if(!quit) {
		if(varDialog) {
			varDialog->setFieldArchive(NULL);
		}

		if(fieldArchive != NULL) {
			delete fieldArchive;
			fieldArchive = NULL;
		}
		field = NULL;

		fieldList->blockSignals(true);
		fieldList->clear();
		fieldList->setEnabled(false);
		fieldList->blockSignals(false);

		disableEditors();
		_scriptManager->removeCopiedReferences();
		if(_modelManager) {
			_modelManager->close();
			_modelManager->deleteLater();
			_modelManager = 0;
		}
		setWindowModified(false);
		setWindowTitle();
		searchDialog->setFieldArchive(NULL);

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

void Window::openRecentFile(QAction *action)
{
	openFile(QDir::fromNativeSeparators(action->text()));
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
		filter.append(tr("Compatible Files (*.lgp *.DAT *.bin *.iso *.img)"));
		filter.append(tr("Lgp Files (*.lgp)"));
		filter.append(tr("DAT File (*.DAT)"));
		filter.append(tr("PC Field File (*)"));
		filter.append(tr("Disc Image (*.bin *.iso *.img)"));

		QString selectedFilter = filter.value(Config::value("open_path_selected_filter").toInt(), filter.first());

		filePath = QFileDialog::getOpenFileName(this, tr("Open a file"), filePath, filter.join(";;"), &selectedFilter);
		if(filePath.isNull()) {
			return;
		}

		int index = filePath.lastIndexOf('/');
		if(index == -1)	index = filePath.size();
		Config::setValue("open_path", filePath.left(index));
		Config::setValue("open_path_selected_filter", filter.indexOf(selectedFilter));
		QStringList recentFiles = Config::value("recentFiles").toStringList();
		if(!recentFiles.contains(filePath)) {
			recentFiles.prepend(filePath);
			if(recentFiles.size() > 20) {
				recentFiles.removeLast();
			}
			Config::setValue("recentFiles", recentFiles);
			fillRecentMenu();
		}
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
														  tr("Select a folder containing the Final Fantasy VII field files"),
														  Config::value("open_dir_path").toString());
	if(filePath.isNull())	{
		return;
	}

	Config::setValue("open_dir_path", filePath);

	QMessageBox question(QMessageBox::Question, tr("File Type"),
						 tr("What type of file to look for?\n"
							" - Playstation field files (\"EXAMPLE.DAT\")\n"
							" - PC Field File (\"example\")\n"),
						 QMessageBox::NoButton, this);
	QAbstractButton *psButton = question.addButton(tr("PS"), QMessageBox::AcceptRole),
	                *pcButton = question.addButton(tr("PC"), QMessageBox::AcceptRole);
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
	return _progressDialog && _progressDialog->wasCanceled();
}

void Window::setObserverMaximum(unsigned int max)
{
	taskBarButton->setMaximum(max);
	progressDialog()->setMaximum(max);
}

void Window::setObserverValue(int value)
{
	QApplication::processEvents();

	taskBarButton->setValue(value);
	progressDialog()->setValue(value);
}

void Window::showProgression(const QString &message, bool canBeCanceled)
{
	setObserverValue(0);
	taskBarButton->setState(QTaskBarButton::Normal);
	progressDialog()->setLabelText(message);
	progressDialog()->setCancelButtonText(canBeCanceled ? tr("Cancel") : tr("Stop"));
	progressDialog()->show();
}

void Window::hideProgression()
{
	taskBarButton->setState(QTaskBarButton::Invisible);
	progressDialog()->hide();
	progressDialog()->reset();
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

	showProgression(tr("Opening..."), false);

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
			out = tr("Nothing found!");
		}
		break;
	case FieldArchiveIO::ErrorOpening:
		out = tr("The file is inaccessible");
		break;
	case FieldArchiveIO::ErrorOpeningTemp:
		out = tr("Can not create temporary file");
		break;
	case FieldArchiveIO::ErrorRemoving:
		out = tr("Unable to remove the file");
		break;
	case FieldArchiveIO::ErrorRenaming:
		out = tr("Failed to rename file.");
		break;
	case FieldArchiveIO::ErrorCopying:
		out = tr("Failed to copy file");
		break;
	case FieldArchiveIO::Invalid:
		out = tr("Invalid file");
		break;
	case FieldArchiveIO::NotImplemented:
		out = tr("This error should not appear, thank you for reporting it");
		break;
	}
	if(!out.isEmpty()) {
		QMessageBox::warning(this, tr("Error"), out);
		fieldArchive->close();
		return;
	}

	fieldList->fill(fieldArchive);
	fieldList->setEnabled(true);
	zonePreview->setEnabled(true);

	// Select memorized entry
	QString previousSessionField = Config::value("currentField").toString();
	if(!previousSessionField.isEmpty()) {
		QList<QTreeWidgetItem *> items = fieldList->findItems(previousSessionField, Qt::MatchExactly);
		if(!items.isEmpty()) {
			fieldList->setCurrentItem(items.first());
		}
	}
	// Select first entry
	if(!fieldList->currentItem() && fieldList->topLevelItemCount() > 0) {
		fieldList->setCurrentItem(fieldList->topLevelItem(0));
	}

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
	//FieldArchivePC otherArch("", FieldArchiveIO::Lgp);
	//fieldArchive->compareTexts(&otherArch);
	//fieldArchive->searchBackgroundZ();
	/* const QStringList &fieldNames = Data::field_names;
	QStringList fieldNamesPC = fieldNames;
	Data::toPCMaplist(fieldNamesPC);
	FieldArchiveIterator it(*fieldArchive);
	while (it.hasNext()) {
		Field *f = it.next();
		if (f && f->isOpen()) {
			QString name = fieldNamesPC.value(fieldNames.indexOf(f->name()), f->name());
			fieldArchive->printBackgroundTiles(f, QString("field-background-tiles-%1/field-background-tiles-%2.txt")
			                                   .arg(fieldArchive->isPC() ? "PC" : "PS")
			                                   .arg(name));
		}
	}
	if (fieldArchive->isPS()) {
		it.toFront();
		while (it.hasNext()) {
			Field *f = it.next();
			if (f && f->isOpen()) {
				QString name = fieldNamesPC.value(fieldNames.indexOf(f->name()), f->name());
				fieldArchive->printBackgroundTiles(f, QString("field-background-tiles-ps-from-pc/field-background-tiles-%2.txt")
				                                   .arg(name), true);
			}
		}
	} */
	//fieldArchive->printTexts("field-texts.txt");
	//fieldArchive->printAkaos("field-akaos.txt");
	//fieldArchive->printModelLoaders("field-model-loaders-generic.txt");
	//fieldArchive->printModelLoaders("field-model-loaders.txt", false);
	//fieldArchive->printScripts("field-scripts.txt");
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
	return fieldList->currentFieldId();
}

void Window::disableEditors()
{
	zoneImage->clear();
	if(fieldModel) {
		fieldModel->clear();
//		if(fieldArchive && fieldArchive->io()->isPC()) {
//			modelThread->cancel();
//			modelThread->wait();
//		}
	}
	zonePreview->setCurrentIndex(0);
	zonePreview->setEnabled(false);

	_scriptManager->clear();
	_scriptManager->setEnabled(false);

	authorAction->setVisible(false);
	if(_textDialog) {
		_textDialog->clear();
		_textDialog->setEnabled(false);
	}
	if(_modelManager) {
		_modelManager->clear();
		_modelManager->setEnabled(false);
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
}

void Window::openField(bool reload)
{
	if(!fieldArchive) {
		return;
	}

	int fieldId = currentFieldId();
	if(fieldId < 0) {
		disableEditors();
		return;
	}
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
	field = fieldArchive->field(fieldId, true);
	if(!field) {
		disableEditors();
		return;
	}

	if(fieldModel) {
		fieldModel->clear();
		//if(fieldArchive->io()->isPC()) {
		//	modelThread->setField(field);
		//}
	}
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
			tutPC = static_cast<FieldArchivePC *>(fieldArchive)->tut(field->name());
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
	zonePreview->setEnabled(true);

	Section1File *scriptsAndTexts = field->scriptsAndTexts();

	if (scriptsAndTexts->isOpen()) {
		// Show author
		authorLbl->setText(tr("Author: %1").arg(scriptsAndTexts->author()));
		authorAction->setVisible(true);

		// Fill group script list
		_scriptManager->fill(field);
		_scriptManager->setEnabled(true);
	} else {
		_scriptManager->clear();
		_scriptManager->setEnabled(false);
	}

	searchDialog->updateRunSearch();
}

void Window::showModel(int grpScriptID)
{
	if(grpScriptID >= 0) {
		int modelID = field->scriptsAndTexts()->modelID(grpScriptID);
		Data::currentModelID = modelID;
		if(fieldModel && modelID > -1) {
			// if(fieldArchive->io()->isPC()) {
			//	modelThread->cancel();
			//	modelThread->wait();
			//	modelThread->setModel(modelID);
			//	modelThread->start();
			// } else {
				showModel(field, field->fieldModel(modelID));
			// }
			return;
		}
	}
	zonePreview->setCurrentIndex(0);
}

void Window::showModel(Field *field, FieldModelFile *fieldModelFile)
{
	if(fieldModel && this->field == field) {
		fieldModel->setFieldModelFile(fieldModelFile);
	}
	zonePreview->setCurrentIndex(int(fieldModel && fieldModelFile->isValid()));
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
		QMessageBox::warning(this, tr("Compilation Error"), tr("Error Compiling Scripts:\n"
																   "scene %1 (%2), group %3 (%4), script %5, line %6: %7")
							 .arg(fieldArchive->field(fieldID)->name())
							 .arg(fieldID)
							 .arg(fieldArchive->field(fieldID)->scriptsAndTexts()->grpScript(groupID)->name())
							 .arg(groupID).arg(scriptID)
							 .arg(opcodeID+1).arg(errorStr));
		gotoOpcode(fieldID, groupID, scriptID, opcodeID);
		_scriptManager->opcodeList()->setErrorLine(opcodeID);
		return;
	}

	QString path;
	if(!currentPath)
	{
		if(fieldArchive->io()->type() == FieldArchiveIO::Dir) {
			path = QFileDialog::getExistingDirectory(this, tr("Save Directory As"), fieldArchive->io()->path());
			if(path.isNull())		return;
		} else {
			QString filter;
			if(fieldArchive->io()->type() == FieldArchiveIO::Lgp) {
				filter = tr("Lgp File (*.lgp)");
			} else if(fieldArchive->io()->type() == FieldArchiveIO::File) {
				filter = tr("DAT File (*.DAT)");
			} else if(fieldArchive->io()->type() == FieldArchiveIO::Iso) {
				filter = tr("Iso File (*.iso *.bin *.img)");
			} else {
				return;
			}
			path = QFileDialog::getSaveFileName(this, tr("Save As"), fieldArchive->io()->path(), filter);
			if(path.isNull())		return;
		}
	}

	showProgression(tr("Saving..."), fieldArchive->io()->type() == FieldArchiveIO::Lgp);
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
		out = tr("Nothing found!");
		break;
	case FieldArchiveIO::ErrorOpening:
		out = tr("The file is inaccessible");
		break;
	case FieldArchiveIO::ErrorOpeningTemp:
		out = tr("Can not create temporary file");
		break;
	case FieldArchiveIO::ErrorRemoving:
		out = tr("Unable to remove the file, check write permissions.");
		break;
	case FieldArchiveIO::ErrorRenaming:
		out = tr("Unable to rename the file, check write permissions.");
		break;
	case FieldArchiveIO::ErrorCopying:
		out = tr("Unable to copy the file, check write permissions.");
		break;
	case FieldArchiveIO::Invalid:
		out = tr("Invalid archive");
		break;
	case FieldArchiveIO::NotImplemented:
		out = tr("This feature is not complete");
		break;
	}
	if(!out.isEmpty())	QMessageBox::warning(this, tr("Error"), out);
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
		_scriptManager->blockSignals(true);
		_scriptManager->gotoOpcode(grpScriptID, scriptID, opcodeID);
		_scriptManager->blockSignals(false);
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
			fieldLzs = tr("PC Field (*)"),
			dat = tr("DAT File (*.DAT)"),
			fieldDec = tr("Uncompressed PC Field (*)");

	name = fieldList->selectedItems().first()->text(0);

	if(fieldArchive->io()->isPC()) {
		types = fieldLzs+";;"+fieldDec;
	} else {
		types = dat;
		name = name.toUpper();
	}

	QString path = Config::value("exportPath").toString().isEmpty() ? fieldArchive->io()->directory() : Config::value("exportPath").toString()+"/";
	path = QFileDialog::getSaveFileName(this, tr("Export the current file"), path+name, types, &selectedFilter);
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
	case 1:	out = tr("Lgp archive is inaccessible");break;
	case 2:	out = tr("Error reopening file");break;
	case 3:	out = tr("Unable to create the new file");break;
	case 4:	out = tr("Not yet implemented!");break;
	}
	if(!out.isEmpty())	QMessageBox::warning(this, tr("Error"), out);
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

			showProgression(tr("Export..."), false);

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
				QMessageBox::warning(this, tr("Error"), tr("An error occured when exporting"));
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

			showProgression(tr("Import..."), false);

			if(massImportDialog->importModule(MassImportDialog::Texts)) {
				toImport.insert(Field::Scripts, massImportDialog->moduleFormat(MassImportDialog::Texts));
			}

			if(!fieldArchive->importation(selectedFields, massImportDialog->directory(),
									  toImport)
					&& !observerWasCanceled()) {
				QMessageBox::warning(this, tr("Error"), tr("An error occurred when importing"));
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
	filter << tr("DAT File (*.DAT)")
		   << tr("PC Field (*)")
		   << tr("Uncompressed DAT File (*)")
		   << tr("Uncompressed PC Field (*)");

	name = fieldList->selectedItems().first()->text(0);
	if(fieldArchive->io()->isPS())
		name = name.toUpper();

	QString path = Config::value("importPath").toString().isEmpty() ? fieldArchive->io()->directory() : Config::value("importPath").toString()+"/";
	path = QFileDialog::getOpenFileName(this, tr("Import a file"), path+name, filter.join(";;"), &selectedFilter);
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
		QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Warning"), tr("The background importation algorithm give bad results in-game, you have been warned!")
				, QMessageBox::Ok | QMessageBox::Cancel);
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
	case 1:	out = tr("Error reopening file");	break;
	case 2:	out = tr("Invalid file");				break;
	}
	if(!out.isEmpty())	QMessageBox::warning(this, tr("Error"), out);
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

#ifdef Q_OS_LINUX // For others systems like Linux, we try to launch ff7 with WINE
	args.append(FF7Exe);
	FF7Exe = "wine";
#elif defined(Q_OS_MAC) // For mac os run with open -a so user can point to the App bundle directly
	args.append(FF7Exe);
	FF7Exe = "open -a";
#else
	FF7Exe = "\"" % FF7Exe % "\"";
#endif

	if(!QProcess::startDetached(FF7Exe, args, FF7ExeDir)) {
		QMessageBox::warning(this, tr("Error"), tr("Final Fantasy VII couldn't be launched\n%1")
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
	searchDialog->setOpcode(_scriptManager->selectedOpcode());
	searchDialog->setScriptExec(_scriptManager->currentGrpScriptId(),
	                            _scriptManager->currentScriptId() - 1);
	searchDialog->show();
	searchDialog->activateWindow();
	searchDialog->raise();
}

void Window::textManager(int textID, int from, int size, bool activate)
{
	if(!_textDialog) {
		_textDialog = new TextManager(this);
		connect(_textDialog, SIGNAL(modified()), SLOT(setModified()));
		connect(_textDialog, SIGNAL(modified()), SLOT(setModified()));
		connect(_scriptManager, SIGNAL(changed()), _textDialog, SLOT(updateFromScripts()));
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
			QMessageBox::warning(this, tr("Opening error"), tr("Can not open encounters!"));
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
			tutPC = static_cast<FieldArchivePC *>(fieldArchive)->tut(field->name());
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
		connect(_backgroundManager, SIGNAL(modified()), SLOT(setModified()));
		connect(_backgroundManager, SIGNAL(modified()), zoneImage, SLOT(drawBackground()));
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
					authorLbl->setText(tr("Author: %1").arg(field->scriptsAndTexts()->author()));
				}
			}
		} else {
			QMessageBox::warning(this, tr("Opening error"), tr("Can not open miscellaneous informations!"));
		}
	}
}

void Window::archiveManager()
{
	if(fieldArchive && fieldArchive->io()->type() == FieldArchiveIO::Lgp) {
		LgpDialog dialog(static_cast<Lgp *>(fieldArchive->io()->device()), this);
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

		showProgression(tr("Applying..."), false);

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
			static_cast<FieldArchivePC *>(fieldArchive)->cleanModelLoader();
		}
		if(!observerWasCanceled() && fieldArchive->isPC() && operations.testFlag(OperationsManager::RemoveUnusedSectionPC)) {
			static_cast<FieldArchivePC *>(fieldArchive)->removeUnusedSections();
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
		_scriptManager->fillOpcodes();
	}
}

void Window::about()
{
	QDialog about(this, Qt::Dialog | Qt::CustomizeWindowHint);
	about.setFixedSize(200, 270);

	QFont font;
	font.setPointSize(12);

	QLabel image(&about);
	image.setScaledContents(true);
	image.setPixmap(QPixmap(":/images/reactor.png"));
	image.move(82, about.height() - 150);
	
	QLabel desc1(PROG_FULLNAME, &about);
	desc1.setFont(font);
	desc1.setFixedWidth(about.width());
	desc1.setAlignment(Qt::AlignHCenter);

	font.setPointSize(8);

	QLabel desc2(tr("By myst6re<br/><a href=\"https://github.com/myst6re/makoureactor/\">github.com/myst6re/makoureactor</a><br/><br/>Thanks to:<ul style=\"margin:0\"><li>Squall78</li><li>Synergy Blades</li><li>Akari</li><li>Asa</li><li>Aali</li></ul>"), &about);
	desc2.setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	desc2.setTextFormat(Qt::RichText);
	desc2.setOpenExternalLinks(true);
	desc2.move(9, 40);
	desc2.setFont(font);

	QPushButton button(tr("Close"), &about);
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
