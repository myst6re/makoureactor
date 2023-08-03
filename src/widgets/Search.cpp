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
#include "Search.h"
#include "Window.h"
#include "core/Config.h"
#include "core/Var.h"
#include "Data.h"

Search::Search(Window *mainWindow) :
    QDialog(mainWindow, Qt::Tool),
    fieldArchive(nullptr),
    text(QString()), clef(0),
    address(0), e_script(0), e_group(0), bank(0),
    atTheEnd(false), atTheBeginning(false)
{
	setWindowTitle(tr("Find"));
	
	tabWidget = new QTabWidget(this);
	tabWidget->addTab(scriptPageWidget(), tr("Scripts"));
	tabWidget->addTab(textPageWidget(), tr("Texts"));
	tabWidget->setCurrentIndex(Config::value("searchDialogCurrentModule").toInt());

	QList<QPushButton *> buttons;
	buttons.append(buttonNext = new QPushButton(tr("Find next"), this));
	buttons.append(buttonPrev = new QPushButton(tr("Find previous"), this));
	buttons.append(buttonAll = new QPushButton(tr("Find all"), this));

	QMap<int, QPushButton *> buttonWidths;
	for (QPushButton *button : buttons) {
		button->setAutoDefault(false);
		buttonWidths[button->sizeHint().width()] = button;
	}

	buttonNext->setDefault(true);
	// Button Shortcuts
	findNextShortcut = new QShortcut(this);
	findNextShortcut->setKey(QKeySequence::FindNext);
	findNextShortcut->setContext(Qt::ApplicationShortcut);
	connect(findNextShortcut, &QShortcut::activated, this, [&]{emit buttonNext->clicked();});

	findPreviousShortcut = new QShortcut(this);
	findPreviousShortcut->setKey(QKeySequence::FindPrevious);
	findPreviousShortcut->setContext(Qt::ApplicationShortcut);
	connect(findPreviousShortcut, &QShortcut::activated, this, [&]{emit buttonPrev->clicked();});

	// buttonNext.width == buttonPrev.width
	QPushButton *largerButton = buttonWidths.last();
	for (QPushButton *button : buttons) {
		button->setFixedSize(largerButton->sizeHint());
	}

	returnToBegin = new QLabel();
	returnToBegin->setWordWrap(true);
	returnToBegin->setAlignment(Qt::AlignRight);
	QPalette pal = returnToBegin->palette();
	pal.setColor(QPalette::Active, QPalette::WindowText, Qt::red);
	returnToBegin->setPalette(pal);
	returnToBegin->hide();

	QGridLayout *grid = new QGridLayout(this);
	grid->addWidget(tabWidget, 0, 0, 1, 3);
	grid->addWidget(buttonNext, 1, 1, Qt::AlignHCenter);
	grid->addWidget(buttonPrev, 2, 1, Qt::AlignHCenter);
	grid->addWidget(buttonAll, 3, 1, Qt::AlignHCenter);
	grid->addWidget(returnToBegin, 1, 2, 3, 1, Qt::AlignRight | Qt::AlignBottom);
	grid->setRowStretch(0, 1);

	searchAllDialog = new SearchAll(mainWindow);

	connect(buttonNext, &QPushButton::clicked, this, &Search::findNext);
	connect(buttonPrev, &QPushButton::clicked, this, &Search::findPrev);
	connect(buttonAll, &QPushButton::clicked, this, &Search::findAll);

	connect(champ->lineEdit(), &QLineEdit::textEdited, champ2->lineEdit(), &QLineEdit::setText);
	connect(caseSens, &QCheckBox::clicked, this, &Search::updateCaseSensitivity);
	connect(useRegexp, &QCheckBox::clicked, useRegexp2, &QCheckBox::setChecked);

	connect(champ2->lineEdit(), &QLineEdit::textEdited, champ->lineEdit(), &QLineEdit::setText);
	connect(caseSens2, &QCheckBox::clicked, this, &Search::updateCaseSensitivity);
	connect(useRegexp2, &QCheckBox::clicked, useRegexp, &QCheckBox::setChecked);

	connect(tabWidget, &QTabWidget::currentChanged, this, &Search::saveCurrentTab);

	setActionsEnabled(false);
	updateCaseSensitivity(Config::value("findWithCaseSensitive").toBool());
}

void Search::saveCurrentTab(int tab)
{
	Config::setValue("searchDialogCurrentModule", tab);
}

QWidget *Search::scriptPageWidget()
{
	QWidget *ret = new QWidget(this);

	liste = new QComboBox(ret);
	liste->addItem(tr("Text"));
	liste->addItem(tr("Variable"));
	liste->addItem(tr("Opcode"));
	liste->addItem(tr("Run"));
	liste->addItem(tr("Map jump"));

	QWidget *text = new QWidget(ret);

	champ = new QComboBox(text);
	champ->setEditable(true);
	champ->setMaximumWidth(400);
	champ->addItems(Config::value("recentSearch").toStringList());
	champ->lineEdit()->completer()->setCompletionMode(QCompleter::PopupCompletion);
	champ->lineEdit()->setPlaceholderText(tr("Find"));

	caseSens = new QCheckBox(tr("Match case"), text);
	useRegexp = new QCheckBox(tr("Regular expression"), text);
	useRegexp->setChecked(Config::value("findWithRegExp").toBool());

	QGridLayout *textLayout = new QGridLayout(text);
	textLayout->setContentsMargins(QMargins());
	textLayout->addWidget(champ, 0, 0, 1, 2);
	textLayout->addWidget(caseSens, 1, 0);
	textLayout->addWidget(useRegexp, 1, 1);

	QWidget *opc = new QWidget(ret);
	opcode = new QComboBox(opc);
	for (quint16 i = 0; i < 256; ++i) {
		opcode->addItem(QString("%1 - %2").arg(i, 2, 16, QChar('0')).arg(Opcode::names[i]).toUpper());
	}
	opcode->addItem(QString("100 - %1").arg(Opcode::names[0x100]));
	// set config values
	opcode->setCurrentIndex(Config::value("SearchedOpcode").toInt() & 0xFFFF);
	opcode->setEditable(true);
	opcode->setInsertPolicy(QComboBox::NoInsert);
	opcode->completer()->setCompletionMode(QCompleter::PopupCompletion);
	opcode->completer()->setFilterMode(Qt::MatchContains);

	opcode2 = new QComboBox(opc);
	opcode2->setEditable(true);
	opcode2->setInsertPolicy(QComboBox::NoInsert);
	opcode2->completer()->setCompletionMode(QCompleter::PopupCompletion);
	opcode2->completer()->setFilterMode(Qt::MatchContains);
	updateOpcode2(opcode->currentIndex());

	QVBoxLayout *opcodeLayout = new QVBoxLayout(opc);
	opcodeLayout->setContentsMargins(QMargins());
	opcodeLayout->addWidget(opcode);
	opcodeLayout->addWidget(opcode2);
	opcodeLayout->addStretch();

	QWidget *variable = new QWidget(ret);
	QGridLayout *variableLayout = new QGridLayout(variable);
	variableLayout->setContentsMargins(QMargins());
	variableLayout->addWidget(new QLabel(tr("Var")), 0, 0);
	variableLayout->addWidget(champBank = new QSpinBox(variable), 0, 1);
	variableLayout->addWidget(champAddress = new QSpinBox(variable), 0, 2);
	variableLayout->addWidget(champOp = new QComboBox(variable), 0, 3);
	variableLayout->addWidget(champValue = new QLineEdit(variable), 0, 4);
	variableLayout->addWidget(comboVarName = new QComboBox(variable), 1, 0, 1, 5);
	variableLayout->setRowStretch(1, 1);
	champBank->setRange(1,15);
	champAddress->setRange(-1, 255); // -1 is an extra value to keep this field empty and search for every addresses
	for (int i = 0; i < 256; ++i) {
		comboVarName->addItem(QString());
	}
	comboVarName->setEditable(true);
	comboVarName->setInsertPolicy(QComboBox::NoInsert);
	comboVarName->completer()->setCompletionMode(QCompleter::PopupCompletion);
	comboVarName->completer()->setFilterMode(Qt::MatchContains);
	champValue->setPlaceholderText(tr("Value"));
	champOp->addItem(tr("All"));
	champOp->addItem(tr("Assignment"));
	champOp->addItem(tr("Bit Assignment"));
	champOp->addItem(tr("Test"));
	champOp->addItem(tr("Bit test"));
	champOp->addItem(tr("Assignment ≠"));
	champOp->addItem(tr("Assignment <"));
	champOp->addItem(tr("Assignment ≤"));
	champOp->addItem(tr("Assignment >"));
	champOp->addItem(tr("Assignment ≥"));

	// set config values
	champBank->setValue(Config::value("SearchedVarBank").toInt());
	champAddress->setValue(Config::value("SearchedVarAdress").toInt());
	comboVarName->setCurrentIndex(Config::value("SearchedVarAdress").toInt());
	champOp->setCurrentIndex(Config::value("SearchedVarOperation").toInt());
	int searchedVarValue = Config::value("SearchedVarValue", 65536).toInt();
	if (searchedVarValue != 65536) {
		champValue->setText(QString::number(searchedVarValue));
	}
	updateSearchVarPlaceholder(champOp->currentIndex());
	updateComboVarName();

	QWidget *execution = new QWidget(ret);
	executionGroup = new QComboBox(execution);
	executionGroup->setEditable(true);
	executionGroup->setInsertPolicy(QComboBox::NoInsert);
	executionGroup->completer()->setCompletionMode(QCompleter::PopupCompletion);
	executionGroup->completer()->setFilterMode(Qt::MatchContains);
	executionScript = new QSpinBox(execution);
	executionScript->setRange(0,31);

	// set config values
	executionGroup->setCurrentIndex(Config::value("SearchedGroupScript").toInt());
	executionScript->setValue(Config::value("SearchedScript").toInt());

	QGridLayout *executionLayout = new QGridLayout(execution);
	executionLayout->setContentsMargins(QMargins());
	executionLayout->addWidget(new QLabel(tr("Script")), 0, 0);
	executionLayout->addWidget(executionScript, 0, 1);
	executionLayout->addWidget(new QLabel(tr("Group")), 1, 0);
	executionLayout->addWidget(executionGroup, 1, 1);
	executionLayout->setColumnStretch(1, 1);

	QWidget *jump = new QWidget(ret);
	mapJump = new QComboBox(jump);
	mapJump->setEditable(true);
	mapJump->setInsertPolicy(QComboBox::NoInsert);
	mapJump->completer()->setCompletionMode(QCompleter::PopupCompletion);
	mapJump->completer()->setFilterMode(Qt::MatchContains);

	QHBoxLayout *jumpLayout = new QHBoxLayout(jump);
	jumpLayout->setContentsMargins(QMargins());
	jumpLayout->addWidget(mapJump, 1, Qt::AlignTop);
	jumpLayout->addStretch();

	QStackedWidget *stack = new QStackedWidget(ret);
	stack->addWidget(text);
	stack->addWidget(variable);
	stack->addWidget(opc);
	stack->addWidget(execution);
	stack->addWidget(jump);

	QGroupBox *contextGroupBox = new QGroupBox(tr("Scope"), this);
	QRadioButton *globalCheckBox = new QRadioButton(tr("All maps"));
	currentFieldCheckBox = new QRadioButton(tr("Current map"));
	currentGrpScriptCheckBox = new QRadioButton(tr("Current group script"));
	currentScriptCheckBox = new QRadioButton(tr("Current script"));

	FieldArchive::SearchScope searchScope = FieldArchive::SearchScope(Config::value("searchScope").toInt());
	switch (searchScope) {
	case FieldArchive::FieldScope:		currentFieldCheckBox->setChecked(true);		break;
//	case FieldArchive::GrpScriptScope:	currentGrpScriptCheckBox->setChecked(true);	break;
//	case FieldArchive::ScriptScope:		currentScriptCheckBox->setChecked(true);	break;
	default:				globalCheckBox->setChecked(true);			break;
	}

	currentGrpScriptCheckBox->hide();
	currentScriptCheckBox->hide();

	QVBoxLayout *contextLayout = new QVBoxLayout(contextGroupBox);
	contextLayout->addWidget(globalCheckBox);
	contextLayout->addWidget(currentFieldCheckBox);
	contextLayout->addWidget(currentGrpScriptCheckBox);
	contextLayout->addWidget(currentScriptCheckBox);
	contextLayout->addStretch(1);

	QGridLayout *topLayout = new QGridLayout(ret);
	topLayout->addWidget(liste, 0, 0, Qt::AlignTop);
	topLayout->addWidget(stack, 0, 1, Qt::AlignTop);
	topLayout->addWidget(contextGroupBox, 2, 0, 1, 2, Qt::AlignBottom);
	topLayout->setRowStretch(1, 1);

	connect(liste, &QComboBox::currentIndexChanged, stack, &QStackedWidget::setCurrentIndex);
	connect(liste, &QComboBox::currentIndexChanged, this, &Search::saveCurrentScriptTab);
	connect(champBank, &QSpinBox::valueChanged, this, &Search::updateComboVarName);
	connect(champAddress, &QSpinBox::valueChanged, comboVarName, &QComboBox::setCurrentIndex);
	connect(comboVarName, &QComboBox::currentIndexChanged, this, &Search::updateChampAdress);
	connect(champOp, &QComboBox::currentIndexChanged, this, &Search::updateSearchVarPlaceholder);
	connect(opcode, &QComboBox::currentIndexChanged, this, &Search::updateOpcode2);

	liste->setCurrentIndex(Config::value("searchDialogScriptCurrentModule").toInt());

	return ret;
}

void Search::saveCurrentScriptTab(int tab)
{
	Config::setValue("searchDialogScriptCurrentModule", tab);
}

QWidget *Search::textPageWidget()
{
	QWidget *ret = new QWidget(this);

	champ2 = new QComboBox(ret);
	champ2->setEditable(true);
	champ2->addItems(Config::value("recentSearch").toStringList());
	champ2->lineEdit()->completer()->setCompletionMode(QCompleter::PopupCompletion);
	champ2->lineEdit()->setPlaceholderText(tr("Find"));

	replace2 = new QComboBox(ret);
	replace2->setEditable(true);
	replace2->addItems(Config::value("recentReplace").toStringList());
	replace2->lineEdit()->setPlaceholderText(tr("Replace"));
	replaceCurrentButton = new QPushButton(tr("Replace"), this);
	replaceAllButton = new QPushButton(tr("Replace all"), this);

	caseSens2 = new QCheckBox(tr("Match case"), ret);
	useRegexp2 = new QCheckBox(tr("Regular expression"), ret);
	useRegexp2->setChecked(Config::value("findWithRegExp").toBool());

	QGroupBox *contextGroupBox = new QGroupBox(tr("Scope"), this);
	QRadioButton *globalCheckBox = new QRadioButton(tr("All maps"));
	currentFieldCheckBox2 = new QRadioButton(tr("Current map"));
	currentTextCheckBox = new QRadioButton(tr("Current text"));

	FieldArchive::SearchScope searchScope = FieldArchive::SearchScope(Config::value("searchScope").toInt());
	switch (searchScope) {
	case FieldArchive::FieldScope:		currentFieldCheckBox2->setChecked(true);		break;
//	case FieldArchive::TextScope:		currentTextCheckBox->setChecked(true);	break;
	default:							globalCheckBox->setChecked(true);			break;
	}

	currentTextCheckBox->hide();

	QVBoxLayout *contextLayout = new QVBoxLayout(contextGroupBox);
	contextLayout->addWidget(globalCheckBox);
	contextLayout->addWidget(currentFieldCheckBox2);
	contextLayout->addWidget(currentTextCheckBox);
	contextLayout->addStretch(1);

	QGridLayout *textLayout = new QGridLayout(ret);
	textLayout->addWidget(champ2, 0, 0, 1, 6);
	textLayout->addWidget(replace2, 1, 0, 1, 2);
	textLayout->addWidget(replaceCurrentButton, 1, 2, 1, 2);
	textLayout->addWidget(replaceAllButton, 1, 4, 1, 2);
	textLayout->addWidget(caseSens2, 2, 0, 1, 3);
	textLayout->addWidget(useRegexp2, 2, 3, 1, 3);
	textLayout->addWidget(contextGroupBox, 4, 0, 1, 6);
	textLayout->setRowStretch(3, 1);

	connect(replaceCurrentButton, &QPushButton::clicked, this, &Search::replaceCurrent);
	connect(replaceAllButton, &QPushButton::clicked, this, &Search::replaceAll);

	return ret;
}

void Search::showEvent(QShowEvent *event)
{
	activateWindow();
	champ->setFocus();
	champ->lineEdit()->selectAll();
	QDialog::showEvent(event);
}

void Search::setActionsEnabled(bool enable)
{
	buttonNext->setEnabled(enable);
	buttonPrev->setEnabled(enable);
	buttonAll->setEnabled(enable);
	replaceCurrentButton->setEnabled(enable);
	replaceAllButton->setEnabled(enable);
}

void Search::updateCaseSensitivity(bool cs)
{
	caseSens->setChecked(cs);
	caseSens2->setChecked(cs);
	Qt::CaseSensitivity caseSensitivity = cs
			? Qt::CaseSensitive
			: Qt::CaseInsensitive;
	champ->lineEdit()->completer()
			->setCaseSensitivity(caseSensitivity);
	champ2->lineEdit()->completer()
			->setCaseSensitivity(caseSensitivity);
}

void Search::setFieldArchive(FieldArchive *fieldArchive)
{
	this->fieldArchive = fieldArchive;
	searchAllDialog->setFieldArchive(fieldArchive);
	updateRunSearch();
	setActionsEnabled(fieldArchive != nullptr);
	if (mapJump->count() <= 0 && fieldArchive != nullptr) {
		int mapID = 0;
		const QStringList &mapList = fieldArchive->mapList().mapNames();
		for (const QString &fieldName : mapList) {
			mapJump->addItem(QString("%1 - %2").arg(mapID++, 3, 10, QChar('0')).arg(fieldName));
		}
		// set config values
		mapJump->setCurrentIndex(Config::value("SearchedMapJump").toInt());
	}
}

void Search::setOpcode(int opcode, bool show)
{
	if (opcode >= 0 && opcode < this->opcode->count()) {
		this->opcode->setCurrentIndex(opcode);
	}

	if (show) {
		tabWidget->setCurrentIndex(0); // Script
		liste->setCurrentIndex(2); // Opcode
	}
}

void Search::setText(const QString &text)
{
	champ->setEditText(text);
	champ2->setEditText(text);
}

void Search::setScriptExec(int groupID, int scriptID)
{
	if (groupID >= 0) {
		executionGroup->setCurrentIndex(groupID);
		executionScript->setValue(scriptID);
	}
}

void Search::updateRunSearch()
{
	executionGroup->clear();
	if (fieldArchive) {
		Field *f = fieldArchive->field(mainWindow()->fieldList()->currentMapId());
		if (f) {
			int i=0;
			for (const GrpScript &grp : f->scriptsAndTexts()->grpScripts()) {
				executionGroup->addItem(QString("%1 - %2").arg(i++).arg(grp.name()));
			}
		}
	}
}

void Search::updateComboVarName()
{
	for (quint16 i=0; i<256; ++i) {
		comboVarName->setItemText(i, QString("%1 - %2").arg(i, 3, 10, QChar('0')).arg(Var::name(quint8(champBank->value()), quint8(i))));
	}
}

void Search::updateChampAdress()
{
	champAddress->blockSignals(true);
	champAddress->setValue(comboVarName->currentIndex());
	champAddress->blockSignals(false);
}

void Search::updateSearchVarPlaceholder(int opIndex)
{
	if (opIndex == Opcode::None) {
		champValue->setEnabled(false);
	} else {
		champValue->setPlaceholderText(opIndex == Opcode::Assign || opIndex == Opcode::Compare ? tr("Value") : tr("Position"));
		champValue->setEnabled(true);
	}
}

void Search::updateOpcode2(int opIndex)
{
	if (opIndex == OpcodeKey::AKAO || opIndex == OpcodeKey::AKAO2) {
		opcode2->clear();
		QList<quint8> unknownItems;
		for (quint16 i=0; i<256; ++i) {
			bool ok;
			QString str = Opcode::akao(quint8(i), &ok);
			if (ok) {
				opcode2->addItem(QString("%1 - %2").arg(i, 2, 16, QChar('0')).toUpper()
				                     .arg(str.remove(QRegularExpression("\\[.*$"))), i);
			} else {
				unknownItems.append(quint8(i));
			}
		}
		for (quint8 i : unknownItems) {
			opcode2->addItem(QString("%1").arg(i, 2, 16, QChar('0')).toUpper(), i);
		}
		opcode2->show();
	} else {
		opcode2->hide();
	}
}

void Search::cancelSearching()
{
	cancel = true;
}

FieldArchive::SearchScope Search::searchScope() const
{
	if (tabWidget->currentIndex() == 0) {
		if (currentFieldCheckBox->isChecked() || liste->currentIndex() == 3) { // Search exec
			return FieldArchive::FieldScope;
		}
		if (currentGrpScriptCheckBox->isChecked()) {
			return FieldArchive::GrpScriptScope;
		}
		if (currentScriptCheckBox->isChecked()) {
			return FieldArchive::ScriptScope;
		}
	} else {
		if (currentFieldCheckBox2->isChecked()) {
			return FieldArchive::FieldScope;
		}
		if (currentTextCheckBox->isChecked()) {
			return FieldArchive::TextScope;
		}
	}

	return FieldArchive::GlobalScope;
}

QString Search::lastMessage() const
{
	switch (searchScope()) {
	case FieldArchive::GlobalScope:		return tr("Last map");
	case FieldArchive::FieldScope:		return tr("Last group");
	case FieldArchive::GrpScriptScope:	return tr("Last script");
	case FieldArchive::ScriptScope:		return tr("Last opcode");
	case FieldArchive::TextScope:		return tr("Last text");
	}
	return QString();
}

QString Search::firstMessage() const
{
	switch (searchScope()) {
	case FieldArchive::GlobalScope:		return tr("First map");
	case FieldArchive::FieldScope:		return tr("First group");
	case FieldArchive::GrpScriptScope:	return tr("First script");
	case FieldArchive::ScriptScope:		return tr("First opcode");
	case FieldArchive::TextScope:		return tr("First text");
	}
	return QString();
}

void Search::findNext()
{
	if (!fieldArchive) {
		return;
	}

	mainWindow()->setEnabled(false);
	buttonNext->setDefault(true);
	returnToBegin->hide();

	QTimer timer(this);
	connect(&timer, &QTimer::timeout, this, &Search::processEvents);
	timer.start(700);

	setSearchValues();

	int mapID, grpScriptID, scriptID, opcodeID, textID;
	qsizetype from;
	FieldArchive::Sorting sorting = mainWindow()->getFieldSorting();
	FieldArchive::SearchScope scope = searchScope();

	atTheBeginning = false;

	if (atTheEnd) {
		mapID = scope == FieldArchive::FieldScope ? mainWindow()->fieldList()->currentMapId() : -1;
		grpScriptID = scope == FieldArchive::GrpScriptScope ? mainWindow()->scriptWidget()->currentGrpScriptId() : -1;
		scriptID = scope == FieldArchive::ScriptScope ? mainWindow()->scriptWidget()->currentScriptId() : -1;
		opcodeID = -1;
		textID = scope == FieldArchive::TextScope && mainWindow()->textWidget() ? mainWindow()->textWidget()->currentTextId() : -1;
		from = 0;
		atTheEnd = false;
	} else {
		mapID = mainWindow()->fieldList()->currentMapId();
		grpScriptID = mainWindow()->scriptWidget()->currentGrpScriptId();
		scriptID = mainWindow()->scriptWidget()->currentScriptId();
		opcodeID = mainWindow()->scriptWidget()->currentOpcodeId();
		textID = mainWindow()->textWidget() ? mainWindow()->textWidget()->currentTextId() : -1;
		from = mainWindow()->textWidget() ? qMax(mainWindow()->textWidget()->currentTextPosition(),
		                                         mainWindow()->textWidget()->currentAnchorPosition())
		                                  : 0;
	}

	bool f = false;
	
	QElapsedTimer t;
	t.start();

	if (tabWidget->currentIndex() == 0) { // scripts page
		if (findNextScript(sorting, scope,
		                   mapID, grpScriptID, scriptID, opcodeID)) {
			emit found(mapID, grpScriptID, scriptID, opcodeID);
			f = true;
		}
	} else { // texts page
		qsizetype size;
		if (findNextText(sorting, scope,
		                 mapID, textID, from, size)) {
			emit foundText(mapID, textID, from, size);
			f = true;
		}
	}

	qDebug() << "elasped search time" << t.elapsed();

	if (!f) {
		returnToBegin->setText(tr("%1,\ncontinued from top.")
		                           .arg(lastMessage()));
		returnToBegin->show();

		atTheEnd = true;
	}

	timer.stop();
	mainWindow()->setEnabled(true);
}

void Search::processEvents() const
{
	QCoreApplication::processEvents();
}

bool Search::findNextScript(FieldArchive::Sorting sorting, FieldArchive::SearchScope scope,
                            int &mapID, int &grpScriptID, int &scriptID, int &opcodeID)
{
	++opcodeID;

	switch (liste->currentIndex()) {
	case 0:
		return fieldArchive->searchTextInScripts(text, mapID, grpScriptID,
		                                         scriptID, opcodeID,
		                                         sorting, scope);
	case 1:
		return fieldArchive->searchVar(bank, address, op, value,
		                               mapID, grpScriptID,
		                               scriptID, opcodeID,
		                               sorting, scope);
	case 2:
		return fieldArchive->searchOpcode(clef, mapID, grpScriptID,
		                                  scriptID, opcodeID,
		                                  sorting, scope);
	case 4:
		return fieldArchive->searchMapJump(map, mapID, grpScriptID,
		                                   scriptID, opcodeID,
		                                   sorting, scope);
	case 3:
		return fieldArchive->searchExec(e_group, e_script, mapID,
		                                grpScriptID, scriptID, opcodeID,
		                                sorting, scope);
	}

	return false;
}

bool Search::findNextText(FieldArchive::Sorting sorting, FieldArchive::SearchScope scope,
                          int &mapID, int &textID, qsizetype &from, qsizetype &size)
{
	return fieldArchive->searchText(text, mapID, textID, from,
	                                size, sorting, scope);
}

void Search::findPrev()
{
	if (!fieldArchive) {
		return;
	}

	mainWindow()->setEnabled(false);
	buttonPrev->setDefault(true);
	returnToBegin->hide();

	int mapID, grpScriptID, scriptID, opcodeID, textID;
	qsizetype from;
	FieldArchive::Sorting sorting = mainWindow()->getFieldSorting();
	FieldArchive::SearchScope scope = searchScope();

	setSearchValues();

	atTheEnd = false;

	if (atTheBeginning) {
		mapID = scope == FieldArchive::FieldScope ? mainWindow()->fieldList()->currentMapId() : 2147483647;
		grpScriptID = scope == FieldArchive::GrpScriptScope ? mainWindow()->scriptWidget()->currentGrpScriptId() : 2147483647;
		scriptID = scope == FieldArchive::ScriptScope ? mainWindow()->scriptWidget()->currentScriptId() : 2147483647;
		opcodeID = 2147483647;
		textID = scope == FieldArchive::TextScope && mainWindow()->textWidget() ? mainWindow()->textWidget()->currentTextId() : 2147483647;
		from = -1;
		atTheBeginning = false;
	} else {
		mapID = mainWindow()->fieldList()->currentMapId();
		grpScriptID = mainWindow()->scriptWidget()->currentGrpScriptId();
		scriptID = mainWindow()->scriptWidget()->currentScriptId();
		opcodeID = mainWindow()->scriptWidget()->currentOpcodeId();
		textID = mainWindow()->textWidget() ? mainWindow()->textWidget()->currentTextId() : -1;
		from = mainWindow()->textWidget() ? qMin(mainWindow()->textWidget()->currentTextPosition(),
		                                         mainWindow()->textWidget()->currentAnchorPosition())
		                                  : -1;
		if (from <= 0) {
			textID--;
		}
	}

	bool f = false;

	QTimer timer(this);
	connect(&timer, &QTimer::timeout, this, &Search::processEvents);
	timer.start(700);

	if (tabWidget->currentIndex() == 0) { // scripts page
		if (findPrevScript(sorting, scope, mapID, grpScriptID,
		                   scriptID, opcodeID)) {
			emit found(mapID, grpScriptID, scriptID, opcodeID);
			f = true;
		}
	} else { // texts page
		qsizetype index = 0, size = 0;
		if (findPrevText(sorting, scope, mapID, textID,
		                 from, index, size)) {
			emit foundText(mapID, textID, index, size);
			f = true;
		}
	}

	timer.stop();

	if (!f) {
		returnToBegin->setText(tr("%1,\nchase at the end.")
		                           .arg(firstMessage()));
		returnToBegin->show();

		atTheBeginning = true;
	}

	mainWindow()->setEnabled(true);
}

bool Search::findPrevScript(FieldArchive::Sorting sorting, FieldArchive::SearchScope scope,
                            int &mapID, int &grpScriptID, int &scriptID, int &opcodeID)
{
	--opcodeID;

	switch (liste->currentIndex()) {
	case 0:
		return fieldArchive->searchTextInScriptsP(text, mapID, grpScriptID,
		                                          scriptID, opcodeID,
		                                          sorting, scope);
	case 1:
		return fieldArchive->searchVarP(bank, address, op, value,
		                                mapID, grpScriptID,
		                                scriptID, opcodeID,
		                                sorting, scope);
	case 2:
		return fieldArchive->searchOpcodeP(clef, mapID, grpScriptID,
		                                   scriptID, opcodeID,
		                                   sorting, scope);
	case 4:
		return fieldArchive->searchMapJumpP(map, mapID, grpScriptID,
		                                    scriptID, opcodeID,
		                                    sorting, scope);
	case 3:
		return fieldArchive->searchExecP(e_group, e_script, mapID,
		                                 grpScriptID, scriptID, opcodeID,
		                                 sorting, scope);
	}

	return false;
}

bool Search::findPrevText(FieldArchive::Sorting sorting, FieldArchive::SearchScope scope,
                          int &mapID, int &textID, qsizetype &index, qsizetype &from, qsizetype &size)
{
	--from;

	return fieldArchive->searchTextP(text, mapID, textID, from,
	                                 index, size, sorting, scope);
}

void Search::findAll()
{
	searchAllDialog->show();
	searchAllDialog->activateWindow();
	searchAllDialog->raise();

	mainWindow()->setEnabled(false);
	setEnabled(false);
	setSearchValues();
	int mapID = -1;
	FieldArchive::Sorting sorting = mainWindow()->getFieldSorting();
	FieldArchive::SearchScope scope = searchScope();

	if (scope == FieldArchive::FieldScope) {
		mapID = mainWindow()->fieldList()->currentMapId();
	}

	if (tabWidget->currentIndex() == 0) { // scripts page
		int grpScriptID = -1, scriptID = -1, opcodeID = -1;

		if (scope == FieldArchive::GrpScriptScope) {
			grpScriptID = mainWindow()->scriptWidget()->currentGrpScriptId();
		}
		if (scope == FieldArchive::ScriptScope) {
			scriptID = mainWindow()->scriptWidget()->currentScriptId();
		}

		searchAllDialog->setScriptSearch();
		while (findNextScript(sorting, scope,
						   mapID, grpScriptID, scriptID, opcodeID)) {
			QCoreApplication::processEvents();
			// Cancelled
			if (searchAllDialog->isHidden()) {
				break;
			}
			searchAllDialog->addResultOpcode(mapID, grpScriptID, scriptID, opcodeID);
		}
	} else { // texts page
		int textID = -1;
		qsizetype from = -1;

		if (scope == FieldArchive::TextScope && mainWindow()->textWidget()) {
			textID = mainWindow()->textWidget()->currentTextId();
		}

		searchAllDialog->setTextSearch();
		qsizetype size = 0;
		while (findNextText(sorting, scope,
						 mapID, textID, ++from, size)) {
			QCoreApplication::processEvents();
			// Cancelled
			if (searchAllDialog->isHidden()) {
				break;
			}
			searchAllDialog->addResultText(mapID, textID, from, size);
		}
	}

	mainWindow()->setEnabled(true);
	setEnabled(true);
}

QRegularExpression Search::buildRegExp(const QString &lineEditText, bool caseSensitive, bool useRegexp)
{
	return QRegularExpression(useRegexp ? lineEditText : QRegularExpression::escape(lineEditText),
	                          caseSensitive ? QRegularExpression::PatternOptions() : QRegularExpression::CaseInsensitiveOption);
}

void Search::setSearchValues()
{
	bool ok;
	if (tabWidget->currentIndex() == 0) { // scripts page
		switch (liste->currentIndex())
		{
		case 0:
		{
			QString lineEditText = champ->lineEdit()->text();
			text = buildRegExp(lineEditText, caseSens->isChecked(), useRegexp->isChecked());
			QStringList recentSearch = Config::value("recentSearch").toStringList();
			qsizetype index = recentSearch.indexOf(lineEditText);
			if (index != -1) {
				recentSearch.removeAt(index);
			}
			recentSearch.prepend(lineEditText);
			champ->clear();
			champ->addItems(recentSearch);
			champ->setCurrentIndex(0);
			if (recentSearch.size() > 20) {
				recentSearch.removeLast();
			}
			Config::setValue("recentSearch", recentSearch);
			Config::setValue("findWithCaseSensitive", caseSens->isChecked());
			Config::setValue("findWithRegExp", useRegexp->isChecked());
		}
			break;
		case 1:
			bank = quint8(champBank->value());
			address = quint8(champAddress->value());
			op = Opcode::Operation(champOp->currentIndex());
			value = champValue->text().toInt(&ok);
			if (!ok)	value = 0x10000;
			Config::setValue("SearchedVarBank", bank);
			Config::setValue("SearchedVarAdress", qint16(address));
			Config::setValue("SearchedVarOperation", champOp->currentIndex());
			Config::setValue("SearchedVarValue", value);
			break;
		case 2:
			clef = opcode->currentIndex();
			if (opcode2->isVisible()) {
				clef |= (opcode2->currentData().toInt() + 1) << 16;
			}
			Config::setValue("SearchedOpcode", clef);
			break;
		case 3:
			e_group = quint8(executionGroup->currentIndex());
			e_script = quint8(executionScript->value());
			Config::setValue("SearchedGroupScript", e_group);
			Config::setValue("SearchedScript", e_script);
			break;
		case 4:
			map = quint16(mapJump->currentIndex());
			Config::setValue("SearchedMapJump", map);
			break;
		default:return;
		}
	} else { // texts page
		QString lineEditText = champ2->lineEdit()->text();
		text = buildRegExp(lineEditText, caseSens2->isChecked(), useRegexp2->isChecked());
		QStringList recentSearch = Config::value("recentSearch").toStringList();
		qsizetype index = recentSearch.indexOf(lineEditText);
		if (index != -1) {
			recentSearch.removeAt(index);
		}
		recentSearch.prepend(lineEditText);
		champ2->clear();
		champ2->addItems(recentSearch);
		champ2->setCurrentIndex(0);
		if (recentSearch.size() > 20) {
			recentSearch.removeLast();
		}
		Config::setValue("recentSearch", recentSearch);
		Config::setValue("findWithCaseSensitive", caseSens2->isChecked());
		Config::setValue("findWithRegExp", useRegexp2->isChecked());
	}
	Config::setValue("searchScope", int(searchScope()));
}

void Search::replaceCurrent()
{
	if (!fieldArchive || !mainWindow()->textWidget()) {
		return;
	}

	returnToBegin->hide();

	setSearchValues();

	if (fieldArchive->replaceText(text, replace2->lineEdit()->text(),
								 mainWindow()->fieldList()->currentMapId(),
								 mainWindow()->textWidget()->currentTextId(),
								 qMin(mainWindow()->textWidget()->currentTextPosition(),
									  mainWindow()->textWidget()->currentAnchorPosition()))) {
		// Update view
		mainWindow()->textWidget()->updateText();
		mainWindow()->setModified();
	}
}

void Search::replaceAll()
{
	if (!fieldArchive) {
		return;
	}

	mainWindow()->setEnabled(false);
	FieldArchive::Sorting sorting = mainWindow()->getFieldSorting();
	FieldArchive::SearchScope scope = searchScope();
	setSearchValues();
	returnToBegin->hide();

	QString after = replace2->lineEdit()->text();
	int mapID = -1, textID = -1;
	qsizetype from = -1, size = 0;

	if (scope == FieldArchive::FieldScope) {
		mapID = mainWindow()->fieldList()->currentMapId();
	} else if (scope == FieldArchive::TextScope) {
		if (mainWindow()->textWidget()) {
			textID = mainWindow()->textWidget()->currentTextId();
		}
	}

	QTimer timer(this);
	connect(&timer, &QTimer::timeout, this, &Search::processEvents);
	timer.start(700);

	int replacedCount = 0;
	while (fieldArchive->searchText(text, mapID, textID, from, size, sorting, scope)) {
		if (fieldArchive->replaceText(text, after, mapID, textID, from)) {
			fieldArchive->field(mapID)->setModified(true);
			replacedCount += 1;
		}
		from += after.size();
	}

	timer.stop();
	mainWindow()->setEnabled(true);

	returnToBegin->setText(tr("%1 occurrences replaced.")
	                           .arg(replacedCount));
	returnToBegin->show();

	if (replacedCount > 0) {

		// Update view
		if (mainWindow()->textWidget()) {
			mainWindow()->textWidget()->updateText();
		}
		mainWindow()->setModified();
	}
}
