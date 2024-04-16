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
#include "Window.h"
#include "core/field/GrpScript.h"
#include "widgets/ConfigWindow.h"
#include "widgets/EncounterWidget.h"
#include "widgets/MiscWidget.h"
#include "widgets/ImportDialog.h"
#include "widgets/ExportChunksDialog.h"
#include "widgets/MassExportDialog.h"
#include "widgets/MassImportDialog.h"
#include "widgets/PsfDialog.h"
#include "widgets/AboutDialog.h"
#include "core/Config.h"
#include "Data.h"
#include "core/field/FieldArchivePC.h"
#include "core/field/FieldArchivePS.h"
#include "widgets/OperationsManager.h"
#include "widgets/EmptyFieldWidget.h"

#include <FF7Text>

Window::Window() :
    fieldArchive(nullptr), field(nullptr), firstShow(true), varDialog(nullptr),
    _textDialog(nullptr), _modelManager(nullptr), _tutManager(nullptr), _walkmeshManager(nullptr),
    _backgroundManager(nullptr), _lgpWidget(nullptr), _progressDialog(nullptr), timer(this)
{
	qApp->setPalette(Config::paletteForSetting());
#if defined(Q_OS_WIN) || defined(Q_OS_DARWIN)
	if (Config::value("color-scheme", 0).toInt() != 0) {
		qApp->setStyle(QStyleFactory::create("Fusion"));
		if (Config::value("color-scheme", 0).toInt() == 2) {
			qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
		}
	}
#endif

	const QString &colorMode = Config::iconThemeColor();
	qDebug() << "MODE" << colorMode;
	QIcon::setFallbackSearchPaths(QIcon::fallbackSearchPaths() << ":/icons/common");
	QIcon::setFallbackSearchPaths(QIcon::fallbackSearchPaths() << QStringLiteral(":/icons/mr-%1").arg(colorMode));
	if (QIcon::themeName().isEmpty()) {
		QIcon::setThemeName(QStringLiteral("mr-%1").arg(colorMode));
	} else {
		QIcon::setFallbackThemeName(QStringLiteral("mr-%1").arg(colorMode));
	}
	setWindowTitle();

	taskBarButton = new QTaskBarButton(this);
	taskBarButton->setMinimum(0);

	authorLbl = new QLabel();
	authorLbl->setMargin(2);
	authorLbl->setWordWrap(true);

	QWidget *toolBarRight = new QWidget();

	_tabBar = new QTabBar();
	_tabBar->setDrawBase(false);
	_tabBar->addTab(tr("Field Scripts"));
	_tabBar->addTab(QIcon::fromTheme(QStringLiteral("archive-generic")), tr("Archive Manager"));

	connect(_tabBar, &QTabBar::currentChanged, this, &Window::setEditorPageIndex);

	QHBoxLayout *toolBarRightLayout = new QHBoxLayout(toolBarRight);
	toolBarRightLayout->addWidget(_tabBar);
	toolBarRightLayout->addStretch();
	toolBarRightLayout->addWidget(authorLbl, 0, Qt::AlignRight);
	toolBarRightLayout->setContentsMargins(QMargins());

	QMenu *menu;
	QAction *actionOpen, *actionFind, *action;
	QMenuBar *menuBar = new QMenuBar(nullptr);

	/* "File" Menu */
	QMenu *fileMenu = menuBar->addMenu(tr("&File"));

	actionOpen = fileMenu->addAction(QIcon::fromTheme(QStringLiteral("document-open")), tr("&Open..."), this, [&] { openFile(QString());}, QKeySequence("Ctrl+O"));
	fileMenu->addAction(QIcon::fromTheme(QStringLiteral("document-open-folder")), tr("Open &Directory..."), this, &Window::openDir, QKeySequence("Shift+Ctrl+O"));
	_recentMenu = new QMenu(tr("&Recent files"), this);
	_recentMenu->setIcon(QIcon::fromTheme(QStringLiteral("document-open-recent")));
	fillRecentMenu();
	connect(_recentMenu, &QMenu::triggered, this, &Window::openRecentFile);
	fileMenu->addMenu(_recentMenu);
	actionSave = fileMenu->addAction(QIcon::fromTheme(QStringLiteral("document-save")), tr("&Save"), this, &Window::save, QKeySequence("Ctrl+S"));
	actionSaveAs = fileMenu->addAction(QIcon::fromTheme(QStringLiteral("document-save-as")), tr("Save &As..."), this, &Window::saveAs, QKeySequence("Shift+Ctrl+S"));
	actionExport = fileMenu->addAction(QIcon::fromTheme(QStringLiteral("document-export")), tr("&Export the current map..."), this, &Window::exportCurrentMap, QKeySequence("Ctrl+E"));
	actionChunks = fileMenu->addAction(QIcon::fromTheme(QStringLiteral("document-chunks-export")), tr("Expor&t map into chunks..."), this, &Window::exportCurrentMapIntoChunks, QKeySequence("Ctrl+U"));
	actionMassExport = fileMenu->addAction(QIcon::fromTheme(QStringLiteral("document-export")), tr("&Mass Export..."), this, &Window::massExport, QKeySequence("Shift+Ctrl+E"));
	actionImport = fileMenu->addAction(QIcon::fromTheme(QStringLiteral("document-import")),tr("&Import to current map..."), this, &Window::importToCurrentMap, QKeySequence("Ctrl+I"));
//	actionMassImport = fileMenu->addAction(QIcon::fromTheme(QStringLiteral("document-import")), tr("Mass im&port..."), this, &Window::massImport, QKeySequence("Shift+Ctrl+I"));
	fileMenu->addSeparator();
	actionRun = fileMenu->addAction(QIcon::fromTheme(QStringLiteral("ff7")), tr("R&un FF7"), this, &Window::runFF7);
	actionRun->setShortcut(Qt::Key_F8);
	actionRun->setShortcutContext(Qt::ApplicationShortcut);
	actionRun->setEnabled(!Data::ff7AppPath().isEmpty());
	fileMenu->addSeparator();
	actionClose = fileMenu->addAction(QIcon::fromTheme(QStringLiteral("document-close")), tr("C&lose"), this, &Window::closeFile);
	fileMenu->addAction(QIcon::fromTheme(QStringLiteral("application-exit")), tr("E&xit"), this, &Window::close, QKeySequence::Quit)->setMenuRole(QAction::QuitRole);

	/* "Tools" Menu */
	menu = menuBar->addMenu(tr("T&ools"));
	QAction *actionText = menu->addAction(QIcon(":/images/text-editor.png"), tr("&Texts..."), this, [&] {textManager(-1, 0, 0, true);}, QKeySequence("Ctrl+T"));
	actionModels = menu->addAction(QIcon(":/images/model.png"), tr("Map &Models..."), this, &Window::modelManager, QKeySequence("Ctrl+M"));
	actionEncounter = menu->addAction(tr("Encounte&rs..."), this, &Window::encounterManager, QKeySequence("Ctrl+N"));
	menu->addAction(QIcon::fromTheme(QStringLiteral("new-audio-alarm")), tr("Mu&sics/Tutorials..."), this, &Window::tutManager, QKeySequence("Ctrl+K"));
	QAction *actionWalkmesh = menu->addAction(QIcon::fromTheme(QStringLiteral("kstars_grid")), tr("&Walkmesh..."), this, &Window::walkmeshManager, QKeySequence("Ctrl+W"));
	menu->addAction(QIcon::fromTheme(QStringLiteral("view-preview")), tr("&Background..."), this, &Window::backgroundManager, QKeySequence("Ctrl+B"));
	actionMisc = menu->addAction(tr("M&iscellaneous..."), this, &Window::miscManager);
	menu->addSeparator();
	menu->addAction(tr("Variable Mana&ger..."), this, &Window::varManager, QKeySequence("Ctrl+G"));
	actionFind = menu->addAction(QIcon::fromTheme(QStringLiteral("edit-find")), tr("&Find..."), this, &Window::searchManager, QKeySequence::Find);
	actionMiscOperations = menu->addAction(tr("B&atch processing..."), this, &Window::miscOperations);

	/* "Settings" Menu */
	menu = menuBar->addMenu(tr("&Settings"));

	actionJp_txt = menu->addAction(tr("&Japanese Characters"), this, &Window::jpText);
	actionJp_txt->setCheckable(true);
	actionJp_txt->setChecked(Config::value("jp_txt", false).toBool());
	FF7Text::setJapanese(actionJp_txt->isChecked());
	connect(actionJp_txt, &QAction::toggled, FF7Text::get(), &FF7Text::setJapanese);

	menuLang = menu->addMenu(tr("&Language"));
	QDir dir(Config::programLanguagesDir());
	QStringList stringList = dir.entryList(QStringList("Makou_Reactor_*.qm"), QDir::Files, QDir::Name);
	action = menuLang->addAction(tr("English (default)"));
	action->setData(QVariant());
	action->setCheckable(true);
	action->setChecked(Config::value("lang").toString() == "en" || ! Config::value("lang").isValid());

	menuLang->addSeparator();
	QTranslator translator;
	for (const QString &str : std::as_const(stringList)) {
		if (translator.load(dir.filePath(str))) {
			action = menuLang->addAction(translator.translate("Window", "English"));
			QString lang = str.mid(14, 2);
			action->setData(lang);
			action->setCheckable(true);
			action->setChecked(Config::value("lang").toString() == lang);
		}
	}
	connect(menuLang, &QMenu::triggered, this, &Window::changeLanguage);

	menu->addAction(QIcon::fromTheme(QStringLiteral("configure")), tr("&Configuration..."), this, &Window::config)->setMenuRole(QAction::PreferencesRole);

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
	toolBar->addSeparator();
	toolBar->addAction(actionText);
	actionText->setStatusTip(tr("Text editor"));
	toolBar->addAction(actionModels);
	actionModels->setStatusTip(tr("Model loader editor"));
	toolBar->addAction(actionWalkmesh);
	actionWalkmesh->setStatusTip(tr("Walkmesh editor"));
	toolBar->addSeparator();

	toolBar->addWidget(toolBarRight);

	_fieldList = new FieldList(this);

	zoneImage = new ApercuBG();
	if (Config::value("OpenGL", true).toBool()) {
		fieldModel = new FieldModel();
//		modelThread = new FieldModelThread(this);

//		connect(modelThread, &FieldModelThread::modelLoaded, this, &Window::showModel);
	} else {
		fieldModel = nullptr;
	}

	zonePreview = new QStackedWidget(this);
	zonePreview->addWidget(zoneImage);
	if (fieldModel) {
		zonePreview->addWidget(fieldModel);
	}

	QWidget *fullFieldList = new QWidget(this);
	QVBoxLayout *fieldListLayout = new QVBoxLayout(fullFieldList);
	fieldListLayout->addWidget(_fieldList->toolBar());
	fieldListLayout->addWidget(_fieldList, 1);
	fieldListLayout->addWidget(_fieldList->lineSearch());
	fieldListLayout->setSpacing(2);
	zonePreview->setContentsMargins(fieldListLayout->contentsMargins());

	horizontalSplitter = new Splitter(Qt::Vertical, this);
	horizontalSplitter->addWidget(fullFieldList);
	horizontalSplitter->addWidget(zonePreview);
	horizontalSplitter->setStretchFactor(0, 10);
	horizontalSplitter->setStretchFactor(1, 2);
	horizontalSplitter->setCollapsible(0, false);
	horizontalSplitter->restoreState(Config::value("horizontalSplitterState").toByteArray());
	horizontalSplitter->setCollapsed(1, !Config::value("backgroundVisible", true).toBool());

	_fieldStackedWidget = new QStackedWidget(this);
	_fieldStackedWidget->setContentsMargins(QMargins());

	_scriptManager = new ScriptManager(this);
	EmptyFieldWidget *emptyFieldWidget = new EmptyFieldWidget();

	_fieldStackedWidget->addWidget(_scriptManager);
	_fieldStackedWidget->addWidget(emptyFieldWidget);

	verticalSplitter = new Splitter(Qt::Horizontal, this);
	verticalSplitter->addWidget(horizontalSplitter);
	verticalSplitter->addWidget(_fieldStackedWidget);
	verticalSplitter->setStretchFactor(0, 3);
	verticalSplitter->setStretchFactor(1, 9);
	verticalSplitter->setCollapsible(1, false);
	verticalSplitter->restoreState(Config::value("verticalSplitterState").toByteArray());
	verticalSplitter->setCollapsed(0, !Config::value("fieldListVisible", true).toBool());

	_mainStackedWidget = new QStackedWidget(this);
	_mainStackedWidget->addWidget(verticalSplitter);
	_mainStackedWidget->setContentsMargins(QMargins());

	setCentralWidget(_mainStackedWidget);

	searchDialog = new Search(this);
	menuBar->addMenu(createPopupMenu());
#ifndef Q_OS_MAC
	menuBar->addAction(tr("&?"), this, &Window::about)->setMenuRole(QAction::AboutRole);
#else
	fileMenu->addAction(tr("&?"), this, &Window::about)->setMenuRole(QAction::AboutRole);
#endif

	setMenuBar(menuBar);

	connect(_fieldList, &FieldList::itemSelectionChanged, this, [&] {openField(false);});
	connect(_fieldList, &FieldList::changed, this, [&] {setModified(true);});
	connect(_fieldList, &FieldList::fieldDeleted, this, &Window::setFieldDeleted);
	connect(zoneImage, &ApercuBG::clicked, this, &Window::backgroundManager);
	connect(searchDialog, &Search::found, this, &Window::gotoOpcode);
	connect(searchDialog, &Search::foundText, this, &Window::gotoText);
	connect(_scriptManager, &ScriptManager::groupScriptCurrentChanged, this, qOverload<int>(&Window::showModel));
	connect(_scriptManager, &ScriptManager::editText, this, [&](int id) {textManager(id, 0, 0, true);});
	connect(_scriptManager, &ScriptManager::changed, this, [&] {setModified(true);});
	connect(_scriptManager, &ScriptManager::searchOpcode, this, &Window::searchOpcode);
	connect(emptyFieldWidget, &EmptyFieldWidget::createMapClicked, this, &Window::createCurrentMap);
	connect(emptyFieldWidget, &EmptyFieldWidget::importMapClicked, this, &Window::importToCurrentMap);

	connect(&timer, &QTimer::timeout, this, &Window::processEvents);

	_fieldList->sortItems(Config::value("fieldListSortColumn", 1).toInt(),
	                     Qt::SortOrder(Config::value("fieldListSortOrder").toBool()));

	restoreState(Config::value("windowState").toByteArray());
	restoreGeometry(Config::value("windowGeometry").toByteArray());


	closeFile();
}

Window::~Window()
{
	if (fieldArchive) {
		fieldArchive->close();
	}
}

void Window::setEditorPageIndex(int index)
{
	if (index == 1) {
		archiveManager();
	} else {
		_mainStackedWidget->setCurrentIndex(0); // Back to standard view
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
	if (!isEnabled() || closeFile(true) == QMessageBox::Cancel) {
		event->ignore();
	} else {
		Config::setValue("windowState", saveState());
		Config::setValue("windowGeometry", saveGeometry());
		Config::setValue("horizontalSplitterState", horizontalSplitter->saveState());
		Config::setValue("verticalSplitterState", verticalSplitter->saveState());
		Config::setValue("fieldListVisible", !verticalSplitter->isCollapsed(0));
		Config::setValue("backgroundVisible", !horizontalSplitter->isCollapsed(1));
		Config::setValue("fieldListSortColumn", _fieldList->sortColumn());
		Config::setValue("fieldListSortOrder", int(_fieldList->header()->sortIndicatorOrder()));
		_scriptManager->saveConfig();
		if (_walkmeshManager) {
			_walkmeshManager->saveConfig();
		}
		if (_backgroundManager) {
			_backgroundManager->saveConfig();
		}
		Config::flush();
		event->accept();
	}
}

QMenu *Window::createPopupMenu()
{
	QMenu *menu = new QMenu(tr("&View"), this);
	menu->addAction(toolBar->toggleViewAction());
	QAction *action;
	action = menu->addAction(tr("Map List"), this, &Window::toggleFieldList);
	action->setCheckable(true);
	action->setChecked(!verticalSplitter->isCollapsed(0));
	action = menu->addAction(tr("Background Preview"), this, &Window::toggleBackgroundPreview);
	action->setCheckable(true);
	action->setChecked(!horizontalSplitter->isCollapsed(1));
	menu->addSeparator();
	for (QAction *action : _scriptManager->actions()) {
		menu->addAction(action);
	}
	return menu;
}

void Window::fillRecentMenu()
{
	_recentMenu->clear();

	for (const QString &recentFile : Config::value("recentFiles").toStringList()) {
		_recentMenu->addAction(QDir::toNativeSeparators(recentFile));
	}

	if (_recentMenu->actions().isEmpty()) {
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
	return _fieldList->getFieldSorting();
}

void Window::jpText(bool enabled)
{
	Config::setValue("jp_txt", enabled);
	if (_textDialog) {
		_textDialog->updateText();
	}
	_scriptManager->fillOpcodes();
}

void Window::changeLanguage(QAction *action)
{
	Config::setValue("lang", action->data());
	QList<QAction *> actions = menuLang->actions();
	for (QAction *act : std::as_const(actions)) {
		act->setChecked(false);
	}

	action->setChecked(true);
	restartNow();
}

void Window::restartNow()
{
	QString title, text;
	QTranslator translator;
	if (translator.load(QString("Makou_Reactor_%1")
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
	if (_fieldList->currentItem() != nullptr) {
		Config::setValue("currentField", _fieldList->currentItem()->text(0));
	}

	if (actionSave->isEnabled() && fieldArchive != nullptr) {
		QString fileChangedList;
		FieldArchiveIterator it(*fieldArchive);

		int i = 0;
		while (it.hasNext()) {
			Field *curField = it.next(false);
			if (curField && curField->isOpen() && curField->isModified()) {
				fileChangedList += "\n - " + curField->name();
				if (i > 10) {
					fileChangedList += "\n...";
					break;
				}
				i++;
			}
		}

		if (!fileChangedList.isEmpty()) {
			fileChangedList.prepend(tr("\n\nEdited files:"));
		}
		int reponse = QMessageBox::warning(this, tr("Save"), tr("Would you like to save changes of %1?%2").arg(fieldArchive->io()->name(), fileChangedList), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (reponse == QMessageBox::Yes) {
			save();
		} else if (reponse == QMessageBox::Cancel) {
			return reponse;
		}
		if (quit) {
			return reponse;
		}
	}

	if (!quit) {
		if (varDialog) {
			varDialog->setFieldArchive(nullptr);
		}

		if (_lgpWidget != nullptr) {
			_mainStackedWidget->removeWidget(_lgpWidget);
			delete _lgpWidget;
			_lgpWidget = nullptr;
		}

		if (fieldArchive != nullptr) {
			delete fieldArchive;
			fieldArchive = nullptr;
		}
		field = nullptr;

		_fieldList->blockSignals(true);
		_fieldList->clear();
		_fieldList->setEnabled(false);
		_fieldList->blockSignals(false);

		disableEditors();
		_scriptManager->removeCopiedReferences();
		_scriptManager->clear();
		if (_modelManager) {
			_modelManager->close();
			_modelManager->deleteLater();
			_modelManager = nullptr;
		}
		setWindowModified(false);
		setWindowTitle();
		searchDialog->setFieldArchive(nullptr);

		actionSave->setEnabled(false);
		actionSaveAs->setEnabled(false);
		actionExport->setEnabled(false);
		actionChunks->setEnabled(false);
		actionMassExport->setEnabled(false);
//		actionMassImport->setEnabled(false);
		actionImport->setEnabled(false);
		_tabBar->setEnabled(false);
		actionClose->setEnabled(false);
		actionModels->setEnabled(false);
		actionEncounter->setEnabled(false);
		actionMisc->setEnabled(false);
		actionMiscOperations->setEnabled(false);
	}

	timer.stop();

	return QMessageBox::Yes;
}

void Window::openRecentFile(QAction *action)
{
	openFile(QDir::fromNativeSeparators(action->text()));
}

void Window::openFile(const QString &path)
{
	timer.stop();

	QString filePath;

	if (path.isEmpty()) {
		filePath = Config::value("open_path").toString();
		if (filePath.isEmpty()) {
			filePath = Data::ff7DataPath();
			if (!filePath.isEmpty())
				filePath.append("/field");
		}
		QStringList filter;
		filter.append(tr("Compatible Files (*.lgp *.DAT *.bin *.iso *.img *.lzs *.dec)"));
		filter.append(tr("Lgp Files (*.lgp)"));
		filter.append(tr("DAT File (*.DAT)"));
		filter.append(tr("PC field File (* *.lzs *.dec)"));
		filter.append(tr("Disc Image (*.bin *.iso *.img)"));

		QString selectedFilter = filter.value(Config::value("open_path_selected_filter").toInt(), filter.first());

		filePath = QFileDialog::getOpenFileName(this, tr("Open a file"), filePath, filter.join(";;"), &selectedFilter);
		if (filePath.isNull()) {
			return;
		}

		qsizetype index = filePath.lastIndexOf('/');
		if (index == -1) {
			index = filePath.size();
		}
		Config::setValue("open_path", filePath.left(index));
		Config::setValue("open_path_selected_filter", filter.indexOf(selectedFilter));
		QStringList recentFiles = Config::value("recentFiles").toStringList();
		if (!recentFiles.contains(filePath)) {
			recentFiles.prepend(filePath);
			if (recentFiles.size() > 20) {
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

	if (ext == "iso" || ext == "bin" || ext == "img") {
		isPS = true;
		type = FieldArchiveIO::Iso;
	} else {
		if (ext == "dat") {
			isPS = true;
			type = FieldArchiveIO::File;
		} else if (ext == "lgp") {
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
	if (filePath.isNull())	{
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
	if (question.clickedButton() != psButton
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
	taskBarButton->setMaximum(int(max));
	progressDialog()->setMaximum(int(max));
}

bool Window::observerRetry(const QString &message)
{
	return QMessageBox::Retry == QMessageBox::question(this, tr("Error"), message,
	                                                   QMessageBox::Retry | QMessageBox::Cancel, QMessageBox::Retry);
}

void Window::setObserverValue(int value)
{
	taskBarButton->setValue(value);
	progressDialog()->setValue(value);
}

void Window::showProgression(const QString &message, bool canBeCanceled)
{
	setObserverValue(0);
	taskBarButton->setState(QTaskBarButton::Normal);
	timer.start(700);
	progressDialog()->setLabelText(message);
	progressDialog()->setCancelButtonText(canBeCanceled ? tr("Cancel") : tr("Stop"));
	progressDialog()->show();
}

void Window::hideProgression()
{
	taskBarButton->setState(QTaskBarButton::Invisible);
	timer.stop();
	progressDialog()->hide();
	progressDialog()->reset();
}

void Window::processEvents() const
{
	QCoreApplication::processEvents();
}

void Window::open(const QString &filePath, FieldArchiveIO::Type type, bool isPS)
{
	closeFile();

	if (isPS) {
		fieldArchive = new FieldArchivePS(filePath, type);
	} else {
		fieldArchive = new FieldArchivePC(filePath, type);
	}

	fieldArchive->setObserver(this);

	showProgression(tr("Opening..."), false);

	FieldArchiveIO::ErrorCode error = fieldArchive->open();

	hideProgression();

	QString out;
	switch (error)
	{
	case FieldArchiveIO::Ok:
	case FieldArchiveIO::Aborted:
		break;
	case FieldArchiveIO::FieldNotFound:
		if (fieldArchive->io()->type() == FieldArchiveIO::Lgp) {
			_tabBar->setTabEnabled(_tabBar->count() - 1, true);
			setWindowTitle();
			archiveManager();
			return;
		} else {
			out = tr("Nothing found!");
		}
		break;
	case FieldArchiveIO::FieldExists:
		out = tr("The file already exists");
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
		out = tr("Failed to rename the file, check write permissions.");
		break;
	case FieldArchiveIO::ErrorCopying:
		out = tr("Failed to copy the file, check write permissions.");
		break;
	case FieldArchiveIO::Invalid:
		out = tr("Invalid file");
		break;
	case FieldArchiveIO::NotImplemented:
		out = tr("This error should not appear, thank you for reporting it");
		break;
	}
	if (!out.isEmpty()) {
		QMessageBox::warning(this, tr("Error"), out);
		fieldArchive->close();
		return;
	}

	_fieldList->fill(fieldArchive);
	zonePreview->setEnabled(true);
	_tabBar->setEnabled(true);

	// Select memorized entry
	QString previousSessionField = Config::value("currentField").toString();
	if (!previousSessionField.isEmpty()) {
		QList<QTreeWidgetItem *> items = _fieldList->findItems(previousSessionField, Qt::MatchExactly);
		if (!items.isEmpty()) {
			_fieldList->setCurrentItem(items.first());
		}
	}
	// Select first entry
	if (!_fieldList->currentItem() && _fieldList->topLevelItemCount() > 0) {
		_fieldList->setCurrentItem(_fieldList->topLevelItem(0));
	}

	_fieldList->setFocus();

	if (fieldArchive->size() > 0) {
		if (varDialog)	varDialog->setFieldArchive(fieldArchive);
		searchDialog->setFieldArchive(fieldArchive);
		actionEncounter->setEnabled(true);
		actionMisc->setEnabled(true);
		actionMiscOperations->setEnabled(true);
		actionMassExport->setEnabled(true);
//		actionMassImport->setEnabled(true);
		actionImport->setEnabled(true);
		actionModels->setEnabled(true);
	}
	if (fieldArchive->io()->type() == FieldArchiveIO::Lgp) {
		_tabBar->setTabEnabled(_tabBar->count() - 1, true);
	}
	actionSaveAs->setEnabled(true);
	actionClose->setEnabled(true);
	
	/* FieldArchiveIterator it(*fieldArchive);
	while (it.hasNext()) {
		FieldPC *field = static_cast<FieldPC *>(it.next());
		
		if (field && field->name() == "woa_3" && field->isOpen()) {
			BackgroundFile *bg = field->background();
			
			QByteArray oldData = field->sectionData(Field::Background);
			QByteArray newData = bg->save();
			
			if (oldData.mid(102) != newData.mid(102)) {
				qsizetype diffIndex = -1, firstDiffIndex = -1;
				for (qsizetype i = 102; i < std::min(oldData.size(), newData.size()); ++i) {
					if (oldData[i] != newData[i]) {
						diffIndex = i;
						if (firstDiffIndex == -1) {
							firstDiffIndex = i;
						}
					}
				}
				
				if (diffIndex == -1 && firstDiffIndex == -1 && oldData.indexOf("END") == newData.indexOf("END")) {
					qDebug() << field->name() << "OK";
					continue;
				}
				
				quint16 nbTiles1, nbTiles2 = 0, nbTiles3 = 0, nbTiles4 = 0;
				quint8 hasTiles2, hasTiles3, hasTiles4;
				memcpy(&nbTiles1, oldData.constData() + 44, 2);
				hasTiles2 = oldData.at(44 + 8 + nbTiles1 * 52);
				if (hasTiles2) {
					memcpy(&nbTiles2, oldData.constData() + 44 + 8 + nbTiles1 * 52 + 1 + 4, 2);
				}
				quint32 tiles3Pos = 44 + 8 + nbTiles1 * 52 + 1 + hasTiles2 * (26 + nbTiles2 * 52);
				hasTiles3 = oldData.at(tiles3Pos);
				if (hasTiles3) {
					memcpy(&nbTiles3, oldData.constData() + tiles3Pos + 1 + 4, 2);
				}
				quint32 tiles4Pos = tiles3Pos + 1 + hasTiles3 * (20 + nbTiles3 * 52);
				hasTiles4 = oldData.at(tiles4Pos);
				if (hasTiles4) {
					memcpy(&nbTiles4, oldData.constData() + tiles4Pos + 1 + 4, 2);
				}

				qDebug() << field->name() << firstDiffIndex << diffIndex;
				qDebug() << "PALETTE" << oldData.indexOf("PALETTE") << "BACK" << oldData.indexOf("BACK") << "TEXTURE" << oldData.indexOf("TEXTURE") << "END" << oldData.indexOf("END") << oldData.size();
				qDebug() << "nbTiles" << nbTiles1 << nbTiles2 << nbTiles3 << nbTiles4;
				qDebug() << "posSection2" << 44 + 8 + nbTiles1 * 52 << "posSection3" << tiles3Pos << "posSection4" << tiles4Pos;
				qDebug() << "hasSection2" << hasTiles2 << "hasSection3" << hasTiles3 << "hasSection4" << hasTiles4;
				
				if (diffIndex >= 50 && diffIndex < 50 + nbTiles1 * 52) {
					qDebug() << "Diff in tiles section 1" << "tile #" << (diffIndex - 50) / 52 << "pos in tile" << (diffIndex - 50) % 52;
				}
				if (hasTiles2 && diffIndex >= 77 + nbTiles1 * 52 && diffIndex < 77 + (nbTiles1 + nbTiles2) * 52) {
					qDebug() << "Diff in tiles section 2" << "tile #" << (diffIndex - 77 - nbTiles1 * 52) / 52 << "pos in tile" << (diffIndex - 77 - nbTiles1 * 52) % 52;
				}
				if (hasTiles3 && diffIndex >= tiles3Pos + 19 && diffIndex < tiles3Pos + 19 + nbTiles3 * 52) {
					qDebug() << "Diff in tiles section 3" << "tile #" << (diffIndex - (tiles3Pos + 19)) / 52 << "pos in tile" << (diffIndex - (tiles3Pos + 19)) % 52;
				}
				if (hasTiles4 && diffIndex >= tiles4Pos + 19 && diffIndex < tiles4Pos + 19 + nbTiles4 * 52) {
					qDebug() << "Diff in tiles section 4" << "tile #" << (diffIndex - (tiles4Pos + 19)) / 52 << "pos in tile" << (diffIndex - (tiles4Pos + 19)) % 52;
				}
				
				nbTiles2 = 0;
				nbTiles3 = 0;
				nbTiles4 = 0;
				memcpy(&nbTiles1, newData.constData() + 44, 2);
				hasTiles2 = newData.at(44 + 8 + nbTiles1 * 52);
				if (hasTiles2) {
					memcpy(&nbTiles2, newData.constData() + 44 + 8 + nbTiles1 * 52 + 1 + 4, 2);
				}
				hasTiles3 = newData.at(44 + 8 + nbTiles1 * 52 + 1 + hasTiles2 * (26 + nbTiles2 * 52));
				if (hasTiles3) {
					memcpy(&nbTiles3, newData.constData() + 44 + 8 + nbTiles1 * 52 + 1 + hasTiles2 * (26 + nbTiles2 * 52) + 1 + 4, 2);
				}
				hasTiles4 = newData.at(44 + 8 + nbTiles1 * 52 + 1 + hasTiles2 * (26 + nbTiles2 * 52) + 1 + hasTiles3 * (20 + nbTiles3 * 52));
				if (hasTiles4) {
					memcpy(&nbTiles4, newData.constData() + 44 + 8 + nbTiles1 * 52 + 1 + hasTiles2 * (26 + nbTiles2 * 52) + 1 + hasTiles3 * (20 + nbTiles3 * 52) + 1 + 4, 2);
				}

				qDebug() << "PALETTE" << newData.indexOf("PALETTE") << "BACK" << newData.indexOf("BACK") << "TEXTURE" << newData.indexOf("TEXTURE") << "END" << newData.indexOf("END") << newData.size();
				qDebug() << "nbTiles" << nbTiles1 << nbTiles2 << nbTiles3 << nbTiles4;
				qDebug() << "posSection2" << 44 + 8 + nbTiles1 * 52 << "posSection3" << 44 + 8 + nbTiles1 * 52 + 1 + hasTiles2 * (26 + nbTiles2 * 52) << "posSection4" << 44 + 8 + nbTiles1 * 52 + 1 + hasTiles2 * (26 + nbTiles2 * 52) + 1 + hasTiles3 * (20 + nbTiles3 * 52);
				qDebug() << "hasSection2" << hasTiles2 << "hasSection3" << hasTiles3 << "hasSection4" << hasTiles4;

				qDebug() << oldData.mid(firstDiffIndex - 8, 16).toHex() << newData.mid(firstDiffIndex - 8, 16).toHex();
				qDebug() << oldData.mid(diffIndex - 8, 16).toHex() << newData.mid(diffIndex - 8, 16).toHex();
			} else {
				qDebug() << field->name() << "OK";
			}
		}
	} */
	
	/* QMap<qsizetype, QSet<QString> > aNamesByBoneCount;
	FieldArchiveIterator it(*fieldArchive);
	while (it.hasNext()) {
		FieldPC *field = static_cast<FieldPC *>(it.next());
		
		if (field && field->isOpen()) {
			FieldModelLoaderPC *loader = field->fieldModelLoader();
			for (int model = 0; model < loader->modelCount(); ++model) {
				if (loader->ANames(model).size() >= 3) {
					FieldModelFilePC *fieldModel = field->fieldModel(model);
					if (!aNamesByBoneCount.contains(fieldModel->boneCount())) {
						aNamesByBoneCount.insert(fieldModel->boneCount(), QSet<QString>());
					}
					aNamesByBoneCount[fieldModel->boneCount()].insert(loader->AName(model, 0).left(4));
				}
			}
		}
	}
	
	for (const QSet<QString> aNames : aNamesByBoneCount) {
		qDebug() << aNames;
	} */

#ifdef DEBUG_FUNCTIONS
	//fieldArchive->printScriptsDirs("final_parody_scripts");
	//FieldArchivePC otherArch("", FieldArchiveIO::Lgp);
	//fieldArchive->compareTexts(&otherArch);
	//fieldArchive->printBackgroundTiles(true, false);
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
	//fieldArchive->printTextsDir("field-texts", true);
	//fieldArchive->printTexts("field-texts.txt", true);
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
	if (fieldArchive) {
		QList<QTreeWidgetItem *> selectedItems = _fieldList->selectedItems();
		QString current;
		if (!selectedItems.isEmpty()) {
			current = selectedItems.first()->text(0);
		}

		if (!fieldArchive->io()->hasName()) { // [*][current - ]MAKOU_REACTOR_NAME
			windowTitle = "[*]";

			if (!current.isEmpty()) {
				 windowTitle.append(current).append(" - ");
			}
		} else { // [current ](*archive) - MAKOU_REACTOR_NAME
			if (!current.isEmpty()) {
				 windowTitle.append(current).append(" (");
			}

			windowTitle.append("[*]").append(fieldArchive->io()->name());

			if (!current.isEmpty()) {
				 windowTitle.append(")");
			}

			windowTitle.append(" - ");
		}
	}

	QWidget::setWindowTitle(windowTitle.append(QString("%1 %2").arg(QLatin1String(MAKOU_REACTOR_NAME), QLatin1String(MAKOU_REACTOR_VERSION))));
}

void Window::disableEditors()
{
	zoneImage->clear();
	if (fieldModel) {
		fieldModel->clear();
//		if (fieldArchive && fieldArchive->io()->isPC()) {
//			modelThread->cancel();
//			modelThread->wait();
//		}
	}
	zonePreview->setCurrentIndex(0);
	zonePreview->setEnabled(false);

	_scriptManager->clear();
	_scriptManager->setEnabled(false);

	authorLbl->setVisible(false);
	if (_textDialog) {
		_textDialog->clear();
		_textDialog->setEnabled(false);
	}
	if (_modelManager) {
		_modelManager->clear();
		_modelManager->setEnabled(false);
	}
	if (_tutManager) {
		_tutManager->clear();
		_tutManager->setEnabled(false);
	}
	if (_walkmeshManager) {
		_walkmeshManager->clear();
		_walkmeshManager->setEnabled(false);
	}
	if (_backgroundManager) {
		_backgroundManager->clear();
		_backgroundManager->setEnabled(false);
	}
}

void Window::openField(bool reload)
{
	_fieldStackedWidget->setCurrentIndex(0);
	actionExport->setEnabled(false);
	actionChunks->setEnabled(false);

	if (!fieldArchive) {
		return;
	}

	int mapId = _fieldList->currentMapId();
	if (mapId < 0) {
		disableEditors();
		return;
	}
	_fieldList->scrollToItem(_fieldList->selectedItems().first());

//	Data::currentCharNames.clear();
	Data::currentHrcNames = nullptr;
	Data::currentAnimNames = nullptr;

	setWindowTitle();

//	if (fieldModel && fieldArchive->io()->isPC()) {
//		modelThread->cancel();
//		modelThread->wait();

//		if (_walkmeshManager)	_walkmeshManager->clear();
//	}

	if (field) {
		BackgroundFile *bgFile = field->background(false);
		if (!bgFile->isModified()) {
			bgFile->clear();
			bgFile->close();
		}
	}

	// Get and set field
	field = fieldArchive->field(mapId, true, true);
	if (!field) {
		if (fieldArchive->isPC()) {
			_fieldStackedWidget->setCurrentIndex(1);
		}
		disableEditors();
		return;
	}

	actionExport->setEnabled(true);
	actionChunks->setEnabled(true);

	if (fieldModel) {
		fieldModel->clear();
		//if (fieldArchive->io()->isPC()) {
		//	modelThread->setField(field);
		//}
	}
	if (_textDialog && (reload || _textDialog->isVisible())) {
		_textDialog->setField(field, reload);
		_textDialog->setEnabled(true);
	}
	if (_modelManager && (reload || _modelManager->isVisible())) {
		_modelManager->fill(field, reload);
		_modelManager->setEnabled(true);
	}
	if (_tutManager && (reload || _tutManager->isVisible())) {
		TutFilePC *tutPC = nullptr;
		if (fieldArchive->isPC()) {
			tutPC = static_cast<FieldArchivePC *>(fieldArchive)->tut(field->name());
		}
		_tutManager->fill(field, tutPC, reload);
		_tutManager->setEnabled(true);
	}
	if (_walkmeshManager && (reload || _walkmeshManager->isVisible())) {
		_walkmeshManager->fill(fieldArchive, field, reload);
		_walkmeshManager->setEnabled(true);
	}
	if (_backgroundManager && (reload || _backgroundManager->isVisible())) {
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
		authorLbl->setVisible(true);

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
	if (grpScriptID >= 0) {
		int modelID = field->scriptsAndTexts()->modelID(quint8(grpScriptID));
		Data::currentModelID = modelID;
		if (fieldModel && modelID > -1) {
			// if (fieldArchive->io()->isPC()) {
			//	modelThread->cancel();
			//	modelThread->wait();
			//	modelThread->setModel(modelID);
			//	modelThread->start();
			// } else {
				showModel(field, field->fieldModel(modelID));
			// }
			return;
		} else if (fieldModel && fieldModel->hasError()) {
			zoneImage->fill(field, true);
		}
	}
	zonePreview->setCurrentIndex(0);
}

void Window::showModel(Field *field, FieldModelFile *fieldModelFile)
{
	if (fieldModel && this->field == field) {
		fieldModel->setFieldModelFile(fieldModelFile);
	}
	zonePreview->setCurrentIndex(int(fieldModel && !fieldModel->hasError() && fieldModelFile->isValid()));
	
	if (fieldModel && fieldModel->hasError() && fieldModelFile->isValid()) {
		zoneImage->setPixmap(QPixmap::fromImage(fieldModelFile->skeleton().toImage(zoneImage->width(), zoneImage->height())));
	}
}

void Window::setModified(bool enabled)
{
	if (field != nullptr) {
		field->setModified(enabled);
	}

	actionSave->setEnabled(enabled);
	setWindowModified(enabled);

	QColor red = Data::color(Data::ColorRedForeground),
	       green = Data::color(Data::ColorGreenForeground);
	int size = _fieldList->topLevelItemCount();
	for (int i = 0; i < size; ++i) {
		QTreeWidgetItem *item = _fieldList->topLevelItem(i);
		int mapId = item->data(0, Qt::UserRole).toInt();
		if (mapId >= 0) {
			Field *curField = fieldArchive->field(mapId, false);
			if (curField) {
				if (enabled && curField->isModified()) {
					item->setForeground(0, red);
				} else if (!enabled && item->foreground(0).color() == red) {
					item->setForeground(0, green);
				}
			}
		}
	}
}

void Window::setFieldDeleted()
{
	field = nullptr;
	actionSave->setEnabled(true);
	setWindowModified(true);

	openField();
}

void Window::save() { saveAs(true); }

void Window::saveAs(bool currentPath)
{
	if (!fieldArchive) {
		return;
	}

	int mapID, groupID, scriptID, opcodeID;
	QString errorStr;

	setEnabled(false);
	bool compiled = fieldArchive->compileScripts(mapID, groupID, scriptID, opcodeID, errorStr);
	setEnabled(true);

	if (!compiled) {
		QMessageBox::warning(this, tr("Compilation Error"), tr("Error Compiling Scripts:\n"
		                                                       "scene %1 (%2), group %3 (%4), script %5, line %6: %7")
		                     .arg(fieldArchive->field(mapID)->name())
		                     .arg(mapID)
		                     .arg(fieldArchive->field(mapID)->scriptsAndTexts()->grpScript(groupID).name())
		                     .arg(groupID).arg(scriptID)
		                     .arg(opcodeID + 1).arg(errorStr));
		gotoOpcode(mapID, groupID, scriptID, opcodeID);
		_scriptManager->opcodeList()->setErrorLine(opcodeID);
		return;
	}

	QString path;
	if (!currentPath) {
		if (fieldArchive->io()->type() == FieldArchiveIO::Dir) {
			path = QFileDialog::getExistingDirectory(this, tr("Save Directory As"), fieldArchive->io()->path());
			if (path.isNull()) {
				return;
			}
		} else {
			QString filter;
			if (fieldArchive->io()->type() == FieldArchiveIO::Lgp) {
				filter = tr("Lgp File (*.lgp)");
			} else if (fieldArchive->io()->type() == FieldArchiveIO::File) {
				filter = tr("DAT File (*.DAT)");
			} else if (fieldArchive->io()->type() == FieldArchiveIO::Iso) {
				filter = tr("Iso File (*.iso *.bin *.img)");
			} else {
				return;
			}
			path = QFileDialog::getSaveFileName(this, tr("Save As"), fieldArchive->io()->path(), filter);
			if (path.isNull()) {
				return;
			}
		}
	}

	showProgression(tr("Saving..."), fieldArchive->io()->type() == FieldArchiveIO::Lgp);
	quint8 error = 0;
	
	// QTime t;t.start();
	error = fieldArchive->save(path);
	// qDebug("Total save time: %d ms", t.elapsed());

	hideProgression();
	QString out;
	switch (error) {
	case FieldArchiveIO::Ok:
		setModified(false);
		setWindowTitle();
		break;
	case FieldArchiveIO::Aborted:
		break;
	case FieldArchiveIO::FieldNotFound:
		out = tr("No maps found");
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
	if (!out.isEmpty()) {
		QMessageBox::warning(this, tr("Error"), fieldArchive->io()->errorString().isEmpty() ? out : fieldArchive->io()->errorString());
	}
}

bool Window::gotoField(int mapID)
{
	if (_fieldList->currentMapId() == mapID) {
		return true;
	}

	int i, size=_fieldList->topLevelItemCount();
	for (i=0; i<size; ++i) {
		QTreeWidgetItem *item = _fieldList->topLevelItem(i);
		if (item->data(0, Qt::UserRole).toInt() == mapID) {
			blockSignals(true);
			_fieldList->setCurrentItem(item);
			_fieldList->scrollToItem(item);
			blockSignals(false);
			return true;
		}
	}
	return false;
}

void Window::gotoOpcode(int mapID, int grpScriptID, int scriptID, int opcodeID)
{
	if (gotoField(mapID)) {
		_scriptManager->blockSignals(true);
		_scriptManager->gotoOpcode(grpScriptID, scriptID, opcodeID);
		showModel(grpScriptID);
		_scriptManager->blockSignals(false);
	}
}

void Window::gotoText(int mapID, int textID, qsizetype from, qsizetype size)
{
	if (_textDialog) {
		_textDialog->blockSignals(true);
	}
	if (gotoField(mapID)) {
		textManager(textID, from, size, false); // show texts dialog
	}
	if (_textDialog) {
		_textDialog->blockSignals(false);
	}
}

/* void Window::notifyFileChanged(const QString &path)
{
	if (!QFile::exists(path)) {
		QMessageBox::warning(this, tr("Fichier supprimé"), tr("Le fichier '%1' a été supprimé par un programme externe !").arg(path));
	}
	else
	{
		int reponse = QMessageBox::warning(this, tr("Fichier modifié"), tr("Le fichier '%1' a été modifié par un programme externe.\nVoulez-vous recharger ce fichier ?").arg(path)
			, QMessageBox::Yes | QMessageBox::No);
		if (reponse == QMessageBox::Yes) {
			open(path);
		}
	}
}

void Window::notifyDirectoryChanged(const QString &path)
{
	if (!QFile::exists(path)) {
		QMessageBox::warning(this, tr("Dossier supprimé"), tr("Le dossier '%1' a été supprimé par un programme externe !").arg(path));
	}
	else
	{
		int reponse = QMessageBox::warning(this, tr("Dossier modifié"), tr("Le dossier '%1' a été modifié par un programme externe.\nVoulez-vous recharger ce dossier ?").arg(path)
			, QMessageBox::Yes | QMessageBox::No);
		if (reponse == QMessageBox::Yes) {
			open(path, true);
		}
	}
} */

void Window::exportCurrentMap()
{
	if (!field || !fieldArchive) {
		return;
	}

	QString types, name, selectedFilter,
			fieldLzs = tr("PC Field Map (* *.lzs)"),
			dat = tr("Data DAT File (*.DAT)"),
			mim = tr("Textures MIM File (*.MIM)"),
			fieldDec = tr("Uncompressed PC Field Map (*.dec)");

	name = _fieldList->selectedItems().first()->text(0);

	if (fieldArchive->io()->isPC()) {
		types = fieldLzs+";;"+fieldDec;
	} else {
		types = dat+";;"+mim;
		name = name.toUpper();
	}

	QString path = Config::value("exportPath").toString().isEmpty() ? fieldArchive->io()->directory() : Config::value("exportPath").toString()+"/";
	path = QFileDialog::getSaveFileName(this, tr("Export the current file"), path+name, types, &selectedFilter);
	if (path.isNull()) {
		return;
	}
	int error = 4;
	QString errorString;
	bool decompressed = selectedFilter == fieldDec;
	
	if (field->isModified()) {
		error = field->save(path, !decompressed);
		if (error != 0) {
			errorString = field->errorString();
		}
	} else {
		QString extension;
		if (selectedFilter == dat) {
			extension = "DAT";
		} else if (selectedFilter == mim) {
			extension = "MIM";
		}
		error = fieldArchive->io()->exportFieldData(field, extension, path, decompressed);
	}
	
	QString out;
	switch (error) {
	case 0: {
			qsizetype index = path.lastIndexOf('/');
			Config::setValue("exportPath", index == -1 ? path : path.left(index));
		} break;
	case 1:
		out = tr("Archive is inaccessible");
		break;
	case 2:
		out = tr("Error reopening file");
		break;
	case 3:
		out = tr("Unable to create the new file");
		break;
	case 4:
		out = tr("Not yet implemented!");
		break;
	}
	if (!out.isEmpty()) {
		QMessageBox::warning(this, tr("Error"), errorString.isEmpty() ? out : errorString);
	}
}

void Window::exportCurrentMapIntoChunks()
{
	if (!field || !fieldArchive) {
		return;
	}

	QString path = Config::value("exportPath").toString().isEmpty() ? fieldArchive->io()->directory() : Config::value("exportPath").toString() + "/";
	path = QFileDialog::getExistingDirectory(this, tr("Choose a directory where to create chunks"), path);
	if (path.isNull()) {
		return;
	}
	
	ExportChunksDialog dialog(this);
	if (dialog.exec() != QDialog::Accepted) {
		return;
	}
	
	Field::FieldSections parts = dialog.parts();
	if (parts == 0) {
		return;
	}

	if (!field->exportToChunks(path, parts)) {
		QMessageBox::warning(this, tr("Error"), field->errorString());
		return;
	}

	Config::setValue("exportPath", path);
}

void Window::massExport()
{
	if (!fieldArchive) return;

	MassExportDialog *massExportDialog = new MassExportDialog(this);
	massExportDialog->fill(fieldArchive, _fieldList->currentMapId());
	if (massExportDialog->exec() == QDialog::Accepted) {
		QList<int> selectedFields = massExportDialog->selectedFields();
		if (!selectedFields.isEmpty()) {
			QMap<FieldArchive::ExportType, QString> toExport;

			showProgression(tr("Export..."), false);

			if (massExportDialog->exportModule(FieldArchive::Fields)) {
				toExport.insert(FieldArchive::Fields, massExportDialog->moduleFormat(FieldArchive::Fields));
			}
			if (massExportDialog->exportModule(FieldArchive::Backgrounds)) {
				toExport.insert(FieldArchive::Backgrounds, massExportDialog->moduleFormat(FieldArchive::Backgrounds));
			}
			if (massExportDialog->exportModule(FieldArchive::Akaos)) {
				toExport.insert(FieldArchive::Akaos, massExportDialog->moduleFormat(FieldArchive::Akaos));
			}
			if (massExportDialog->exportModule(FieldArchive::Texts)) {
				toExport.insert(FieldArchive::Texts, massExportDialog->moduleFormat(FieldArchive::Texts));
			}
			if (massExportDialog->exportModule(FieldArchive::Chunks)) {
				toExport.insert(FieldArchive::Chunks, massExportDialog->moduleFormat(FieldArchive::Chunks));
			}

			PsfTags tags;

			if (toExport.value(FieldArchive::Akaos) == "minipsf") {
				PsfDialog psfDialog(this);
				psfDialog.setNoTitle(true);
				if (psfDialog.exec() == QDialog::Rejected) {
					return;
				}

				tags = psfDialog.tags();
			}

			if (!fieldArchive->exportation(selectedFields, massExportDialog->directory(),
									  massExportDialog->overwrite(), toExport, &tags)
					&& !observerWasCanceled()) {
				QMessageBox::warning(this, tr("Error"), tr("An error occured when exporting"));
			}

			hideProgression();
		}
	}
}

void Window::massImport()
{
	if (!fieldArchive) return;

	MassImportDialog *massImportDialog = new MassImportDialog(this);
	massImportDialog->fill(fieldArchive, _fieldList->currentMapId());
	if (massImportDialog->exec() == QDialog::Accepted) {
		QList<int> selectedFields = massImportDialog->selectedFields();
		if (!selectedFields.isEmpty()) {
			QMap<Field::FieldSection, QString> toImport;

			showProgression(tr("Import..."), false);

			if (massImportDialog->importModule(MassImportDialog::Texts)) {
				toImport.insert(Field::Scripts, massImportDialog->moduleFormat(MassImportDialog::Texts));
			}

			if (!fieldArchive->importation(selectedFields, massImportDialog->directory(),
									  toImport)
					&& !observerWasCanceled()) {
				QMessageBox::warning(this, tr("Error"), tr("An error occurred when importing"));
			}

			hideProgression();
		}
	}
}

void Window::importToCurrentMap()
{
	int mapId = _fieldList->currentMapId();
	if (mapId < 0) {
		return;
	}

	Field *field = fieldArchive->field(mapId, false);
	if (nullptr == field) {
		return;
	}

	if (!field->isOpen()) {
		field->initEmpty();
	}

	qsizetype index;
	QString name, selectedFilter,
	    pc = tr("PC Field Map (*)"),
	    chunk = tr("Field chunk (*.chunk*.?)"),
	    dat = tr("PS Field Map (*.DAT)");
	QStringList filter;
	filter << dat << pc << chunk;

	name = _fieldList->selectedItems().first()->text(0);
	if (fieldArchive->io()->isPS()) {
		name = name.toUpper();
	}

	QString path = Config::value("importPath").toString().isEmpty() ? fieldArchive->io()->directory() : Config::value("importPath").toString()+"/";
	path = QFileDialog::getOpenFileName(this, tr("Import a file"), path+name, filter.join(";;"), &selectedFilter);
	if (path.isNull()) {
		return;
	}

	bool isChunk = selectedFilter == chunk;
	
	if (isChunk) {
		if (!field->importChunk(path)) {
			QMessageBox::warning(this, tr("Error"), field->errorString());
			return;
		}
	} else {
		bool isDat = selectedFilter == dat;

		ImportDialog dialog((isDat && fieldArchive->io()->isPS())
		                    || (!isDat && fieldArchive->io()->isPC()),
		                    isDat, path, this);
		if (dialog.exec() != QDialog::Accepted) {
			return;
		}
	
		Field::FieldSections parts = dialog.parts();
		if (parts == 0) {
			return;
		}
	
		QFile bsxDevice(dialog.bsxPath()), mimDevice(dialog.mimPath());
	
		if (!field->importer(path, isDat, dialog.isCompressed(), parts, &bsxDevice, &mimDevice)) {
			QMessageBox::warning(this, tr("Error"), field->errorString());
			return;
		}
	}

	setModified(true);
	index = path.lastIndexOf('/');
	Config::setValue("importPath", index == -1 ? path : path.left(index));
	openField(true);
}

void Window::varManager()
{
	if (!varDialog) {
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

	if (!QProcess::startDetached(FF7Exe, args, FF7ExeDir)) {
		QMessageBox::warning(this, tr("Error"), tr("Final Fantasy VII couldn't be launched\n%1")
							 .arg(QDir::toNativeSeparators(FF7Exe)));
	}
}

void Window::searchManager()
{
	if (_textDialog && _textDialog->isVisible()) {
		QString selectedText = _textDialog->selectedText();
		if (!selectedText.isEmpty()) {
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

void Window::searchOpcode(int opcodeID)
{
	searchManager();
	searchDialog->setOpcode(opcodeID, true);
}

void Window::createCurrentMap()
{
	int mapId = _fieldList->currentMapId();

	if (mapId < 0 || fieldArchive == nullptr) {
		return;
	}

	Field *field = fieldArchive->field(mapId, false);
	if (field != nullptr) {
		field->initEmpty();
		openField(true);
		setModified(true);
	}
}

void Window::textManager(int textID, int from, int size, bool activate)
{
	if (!_textDialog) {
		_textDialog = new TextManager(this);
		connect(_textDialog, &TextManager::modified, this, [&] {setModified(true);});
		connect(_textDialog, &TextManager::opcodeModified, _scriptManager, &ScriptManager::refreshOpcode);
		connect(_scriptManager, &ScriptManager::changed, _textDialog, &TextManager::updateFromScripts);
		connect(actionJp_txt, &QAction::triggered, _textDialog, &TextManager::updateNames);
	}

	if (field && field->scriptsAndTexts()->isOpen()) {
		_textDialog->setField(field);
		_textDialog->setEnabled(true);
	} else {
		_textDialog->clear();
		_textDialog->setEnabled(false);
	}
	_textDialog->show();
	if (activate) {
		_textDialog->activateWindow();
	} else {
		searchDialog->raise();
	}
	if (textID >= 0) {
		_textDialog->gotoText(textID, from, size);
	}
}

void Window::modelManager()
{
	if (!_modelManager) {
		if (!field)	return;
		if (field->isPC()) {
			_modelManager = new ModelManagerPC(this);
		} else {
			_modelManager = new ModelManagerPS(this);
		}
		connect(_modelManager, &ModelManager::modified, this, [&] {setModified(true);});
	}

	if (field) {
//		zonePreview->setCurrentIndex(0);
//		if (fieldModel) {
//			fieldModel->clear();
//		}

		_modelManager->fill(field);
		_modelManager->setEnabled(true);

//		bool modelLoaded = false;
//		if (fieldModel && Data::currentModelID != -1) {
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
	if (field) {
		EncounterFile *encounter = field->encounter();
		if (encounter->isOpen()) {
			EncounterWidget dialog(encounter, this);
			if (dialog.exec()==QDialog::Accepted) {
				if (encounter->isModified()) {
					setModified(true);
				}
			}
		} else {
			QMessageBox::warning(this, tr("Opening error"), tr("Can not open encounters!"));
		}
	}
}

void Window::tutManager()
{
	if (!_tutManager) {
		_tutManager = new TutWidget(this);
		connect(_tutManager, &TutWidget::modified, this, [&] {setModified(true);});
	}

	if (field && field->tutosAndSounds()->isOpen()) {
		TutFilePC *tutPC = nullptr;
		if (fieldArchive->isPC()) {
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
	if (!_walkmeshManager) {
		_walkmeshManager = new WalkmeshManager(this);
		connect(_walkmeshManager, &WalkmeshManager::modified, this, [&] {setModified(true);});
	}

	if (field) {
		_walkmeshManager->fill(fieldArchive, field);
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
	if (!_backgroundManager) {
		_backgroundManager = new BGDialog(this);
		connect(_backgroundManager, &BGDialog::modified, this, [&] {setModified(true);});
		connect(_backgroundManager, &BGDialog::modified, zoneImage, &ApercuBG::drawBackground);
	}

	if (field) {
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
	if (field) {
		InfFile *inf = field->inf();
		if (inf->isOpen()) {
			MiscWidget dialog(inf, field, this);
			if (dialog.exec()==QDialog::Accepted)
			{
				if (inf->isModified() || field->isModified()) {
					setModified(true);
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
	if (_lgpWidget != nullptr && _mainStackedWidget->currentWidget() == _lgpWidget) {
		_mainStackedWidget->setCurrentIndex(0); // Back to standard view
	} else if (fieldArchive && fieldArchive->io()->type() == FieldArchiveIO::Lgp) {
		if (_lgpWidget == nullptr) {
			_lgpWidget = new LgpWidget(static_cast<Lgp *>(fieldArchive->io()->device()), this);
			connect(_lgpWidget, &LgpWidget::modified, this, [&] {setModified(true);});
			_mainStackedWidget->addWidget(_lgpWidget);
		}
		_mainStackedWidget->setCurrentWidget(_lgpWidget);
	}
}

void Window::miscOperations()
{
	if (!fieldArchive) {
		return;
	}

	OperationsManager dialog(fieldArchive->isPC(), this);
	if (dialog.exec() == QDialog::Accepted) {
		OperationsManager::Operations operations = dialog.selectedOperations();

		showProgression(tr("Applying..."), false);

		if (operations.testFlag(OperationsManager::CleanUnusedTexts)) {
			fieldArchive->cleanTexts();
		}
		if (!observerWasCanceled()) {
			if (operations.testFlag(OperationsManager::AutosizeTextWindows)) {
				fieldArchive->autosizeTextWindows();
			}
			if (operations.testFlag(OperationsManager::RemoveTexts)) {
				fieldArchive->removeTexts();
			}
			if (operations.testFlag(OperationsManager::RemoveBattles)) {
				fieldArchive->removeBattles();
			}
			if (fieldArchive->isPC()) {
				if (operations.testFlag(OperationsManager::CleanModelLoaderPC)) {
					static_cast<FieldArchivePC *>(fieldArchive)->cleanModelLoader();
				}
				if (operations.testFlag(OperationsManager::RemoveUnusedSectionPC)) {
					static_cast<FieldArchivePC *>(fieldArchive)->removeUnusedSections();
				}
				if (operations.testFlag(OperationsManager::RepairBackgroundsPC)) {
					static_cast<FieldArchivePC *>(fieldArchive)->repairBackgroundsPC();
				}
				if (operations.testFlag(OperationsManager::ResizeBackgrounds)) {
					static_cast<FieldArchivePC *>(fieldArchive)->resizeBackgrounds(QSize(448, 1));
				}
			}
		}

		hideProgression();

		if (fieldArchive->isModified()) {
			setModified(true);
		}
	}
}

void Window::config()
{
	ConfigWindow configWindow(this);
	if (configWindow.exec() == QDialog::Accepted) {
		actionJp_txt->setChecked(Config::value("jp_txt", false).toBool());
		if (_textDialog) {
			_textDialog->updateText();
		}
		actionRun->setEnabled(!Data::ff7AppPath().isEmpty());
		_scriptManager->fillOpcodes();
	}
}

void Window::about()
{
	AboutDialog about(this);
	about.exec();
}
