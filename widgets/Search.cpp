/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2012 Arzel Jérôme <myst6re@gmail.com>
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
#include "Data.h"

Search::Search(Window *mainWindow) :
	QDialog(mainWindow, Qt::Tool),
	atTheEnd(false), atTheBeginning(false),
	fieldArchive(0),
	clef(0), text(QString()),
	bank(0), adress(0), e_script(0), e_group(0)
{
	setWindowTitle(tr("Rechercher"));
	
	tabWidget = new QTabWidget(this);
	tabWidget->addTab(scriptPageWidget(), tr("Scripts"));
	tabWidget->addTab(textPageWidget(), tr("Textes"));

	buttonNext = new QPushButton(tr("Chercher le suivant"), this);
	buttonPrev = new QPushButton(tr("Chercher le précédent"), this);
	buttonPrev->setAutoDefault(false);
	buttonNext->setAutoDefault(false);
	buttonNext->setDefault(true);

	new QShortcut(QKeySequence::FindNext, this, SLOT(findNext()), 0, Qt::ApplicationShortcut);
	new QShortcut(QKeySequence::FindPrevious, this, SLOT(findPrev()), 0, Qt::ApplicationShortcut);

	// buttonNext.width == buttonPrev.width
	if(buttonPrev->sizeHint().width() > buttonNext->sizeHint().width())
		buttonNext->setFixedSize(buttonPrev->sizeHint());
	else
		buttonPrev->setFixedSize(buttonNext->sizeHint());

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
	grid->addWidget(returnToBegin, 1, 2, 2, 1, Qt::AlignRight | Qt::AlignBottom);
	QMargins margins = grid->contentsMargins();
	margins.setTop(0);
	margins.setLeft(0);
	margins.setRight(0);
	grid->setContentsMargins(margins);
	grid->setRowStretch(0, 1);

	connect(buttonNext, SIGNAL(released()), SLOT(findNext()));
	connect(buttonPrev, SIGNAL(released()), SLOT(findPrev()));

	connect(champ->lineEdit(), SIGNAL(textEdited(QString)), champ2->lineEdit(), SLOT(setText(QString)));
	connect(caseSens, SIGNAL(clicked(bool)), SLOT(updateCaseSensitivity(bool)));
	connect(useRegexp, SIGNAL(clicked(bool)), useRegexp2, SLOT(setChecked(bool)));

	connect(champ2->lineEdit(), SIGNAL(textEdited(QString)), champ->lineEdit(), SLOT(setText(QString)));
	connect(caseSens2, SIGNAL(clicked(bool)), SLOT(updateCaseSensitivity(bool)));
	connect(useRegexp2, SIGNAL(clicked(bool)), useRegexp, SLOT(setChecked(bool)));

	setActionsEnabled(false);
	updateCaseSensitivity(Config::value("findWithCaseSensitive").toBool());
}

QWidget *Search::scriptPageWidget()
{
	QWidget *ret = new QWidget(this);

	liste = new QComboBox(ret);
	liste->addItem(tr("Texte"));
	liste->addItem(tr("Variable"));
	liste->addItem(tr("Opcode"));
	liste->addItem(tr("Exec"));
	liste->addItem(tr("Saut d'écran"));

	QWidget *text = new QWidget(ret);

	champ = new QComboBox(text);
	champ->setEditable(true);
	champ->setMaximumWidth(400);
	champ->addItems(Config::value("recentSearch").toStringList());
	champ->lineEdit()->completer()->setCompletionMode(QCompleter::PopupCompletion);

	caseSens = new QCheckBox(tr("Sensible à la casse"), text);
	useRegexp = new QCheckBox(tr("Utiliser les expressions régulières"), text);
	useRegexp->setChecked(Config::value("findWithRegExp").toBool());

	QGridLayout *textLayout = new QGridLayout(text);
	textLayout->setContentsMargins(QMargins());
	textLayout->addWidget(champ, 0, 0, 1, 2);
	textLayout->addWidget(caseSens, 1, 0);
	textLayout->addWidget(useRegexp, 1, 1);

	QWidget *opc = new QWidget(ret);
	opcode = new QComboBox(opc);
	for(quint16 i=0 ; i<256 ; ++i)
		opcode->addItem(QString("%1 - %2").arg(i,2,16,QChar('0')).arg(Opcode::names[i]));
	opcode->addItem(QString("100 - %1").arg(Opcode::names[0x100]));
	// set config values
	opcode->setCurrentIndex(Config::value("SearchedOpcode").toInt());

	QVBoxLayout *opcodeLayout = new QVBoxLayout(opc);
	opcodeLayout->setContentsMargins(QMargins());
	opcodeLayout->addWidget(opcode);
	opcodeLayout->addStretch();

	QWidget *variable = new QWidget(ret);
	QGridLayout *variableLayout = new QGridLayout(variable);
	variableLayout->setContentsMargins(QMargins());
	variableLayout->addWidget(new QLabel(tr("Var")), 0, 0);
	variableLayout->addWidget(champBank = new QSpinBox(variable), 0, 1);
	variableLayout->addWidget(champAdress = new QSpinBox(variable), 0, 2);
	variableLayout->addWidget(new QLabel("="), 0, 3);
	variableLayout->addWidget(champValue = new QLineEdit(variable), 0, 4);
	variableLayout->addWidget(comboVarName = new QComboBox(variable), 1, 0, 1, 5);
	variableLayout->setRowStretch(1, 1);
	champBank->setRange(1,15);
	champAdress->setRange(0,255);
	for(int i=0 ; i<256 ; ++i) {
		comboVarName->addItem(QString());
	}
	updateComboVarName();

	// set config values
	champBank->setValue(Config::value("SearchedVarBank").toInt());
	champAdress->setValue(Config::value("SearchedVarAdress").toInt());
	comboVarName->setCurrentIndex(Config::value("SearchedVarAdress").toInt());
	int searchedVarValue = Config::value("SearchedVarValue", 65536).toInt();
	if(searchedVarValue != 65536) {
		champValue->setText(QString::number(searchedVarValue));
	}

	QWidget *execution = new QWidget(ret);
	executionGroup = new QComboBox(execution);
	executionScript = new QSpinBox(execution);
	executionScript->setRange(0,31);

	// set config values
	executionGroup->setCurrentIndex(Config::value("SearchedGroupScript").toInt());
	executionScript->setValue(Config::value("SearchedScript").toInt());

	QGridLayout *executionLayout = new QGridLayout(execution);
	executionLayout->setContentsMargins(QMargins());
	executionLayout->addWidget(new QLabel(tr("Script")), 0, 0);
	executionLayout->addWidget(executionScript, 0, 1);
	executionLayout->addWidget(new QLabel(tr("Groupe")), 1, 0);
	executionLayout->addWidget(executionGroup, 1, 1);
	executionLayout->setColumnStretch(1, 1);

	QWidget *jump = new QWidget(ret);
	mapJump = new QComboBox(jump);

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

	QGroupBox *contextGroupBox = new QGroupBox(tr("Contexte"), this);
	QRadioButton *globalCheckBox = new QRadioButton(tr("Sur tous les écrans"));
	currentFieldCheckBox = new QRadioButton(tr("Uniquement l'écran courant"));
	currentGrpScriptCheckBox = new QRadioButton(tr("Uniquement le groupe courant"));
	currentScriptCheckBox = new QRadioButton(tr("Uniquement le script courant"));

	FieldArchive::SearchScope searchScope = FieldArchive::SearchScope(Config::value("searchScope").toInt());
	switch(searchScope) {
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

	connect(liste, SIGNAL(currentIndexChanged(int)), stack, SLOT(setCurrentIndex(int)));
	connect(champBank, SIGNAL(valueChanged(int)), SLOT(updateComboVarName()));
	connect(champAdress, SIGNAL(valueChanged(int)), comboVarName, SLOT(setCurrentIndex(int)));
	connect(comboVarName, SIGNAL(currentIndexChanged(int)), SLOT(updateChampAdress()));

	return ret;
}

QWidget *Search::textPageWidget()
{
	QWidget *ret = new QWidget(this);

	champ2 = new QComboBox(ret);
	champ2->setEditable(true);
	champ2->setMaximumWidth(400);
	champ2->addItems(Config::value("recentSearch").toStringList());
	champ2->lineEdit()->completer()->setCompletionMode(QCompleter::PopupCompletion);

	replace2 = new QComboBox(ret);
	replace2->setEditable(true);
	replace2->setMaximumWidth(400);
	replace2->addItems(Config::value("recentReplace").toStringList());
	replaceCurrentButton = new QPushButton(tr("Remplacer"), this);
	replaceAllButton = new QPushButton(tr("Remplacer tout"), this);

	caseSens2 = new QCheckBox(tr("Sensible à la casse"), ret);
	useRegexp2 = new QCheckBox(tr("Utiliser les expressions régulières"), ret);
	useRegexp2->setChecked(Config::value("findWithRegExp").toBool());

	QGroupBox *contextGroupBox = new QGroupBox(tr("Contexte"), this);
	QRadioButton *globalCheckBox = new QRadioButton(tr("Sur tous les écrans"));
	currentFieldCheckBox2 = new QRadioButton(tr("Uniquement l'écran courant"));
	currentTextCheckBox = new QRadioButton(tr("Uniquement le texte courant"));

	FieldArchive::SearchScope searchScope = FieldArchive::SearchScope(Config::value("searchScope").toInt());
	switch(searchScope) {
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

	connect(replaceCurrentButton, SIGNAL(clicked()), SLOT(replaceCurrent()));
	connect(replaceAllButton, SIGNAL(clicked()), SLOT(replaceAll()));

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
	updateRunSearch();
	setActionsEnabled(fieldArchive != NULL);
	if(mapJump->count() <= 0) {
		int mapID=0;
		foreach(const QString &fieldName, Data::field_names) {
			mapJump->addItem(QString("%1 - %2").arg(mapID++, 3, 10, QChar('0')).arg(fieldName));
		}
		// set config values
		mapJump->setCurrentIndex(Config::value("SearchedMapJump").toInt());
	}
}

void Search::setOpcode(int opcode)
{
	this->opcode->setCurrentIndex(opcode);
}

void Search::setText(const QString &text)
{
	champ->setEditText(text);
	champ2->setEditText(text);
}

void Search::setScriptExec(int groupID, int scriptID)
{
	if(groupID >= 0) {
		executionGroup->setCurrentIndex(groupID);
		executionScript->setValue(scriptID);
	}
}

void Search::updateRunSearch()
{
	executionGroup->clear();
	if(fieldArchive) {
		Field *f = fieldArchive->field(mainWindow()->currentFieldId());
		if(f) {
			int i=0;
			foreach(const GrpScript *grp, f->scriptsAndTexts()->grpScripts())
				executionGroup->addItem(QString("%1 - %2").arg(i++).arg(grp->name()));
		}
	}
}

void Search::updateComboVarName()
{
	for(int i=0 ; i<256 ; ++i) {
		comboVarName->setItemText(i, QString("%1 - %2").arg(i, 3, 10, QChar('0')).arg(Var::name(champBank->value(), i)));
	}
}

void Search::updateChampAdress()
{
	champAdress->blockSignals(true);
	champAdress->setValue(comboVarName->currentIndex());
	champAdress->blockSignals(false);
}

void Search::cancelSearching()
{
	cancel = true;
}

FieldArchive::SearchScope Search::searchScope() const
{
	if(tabWidget->currentIndex() == 0) {
		if(currentFieldCheckBox->isChecked() || liste->currentIndex() == 3) { // Search exec
			return FieldArchive::FieldScope;
		}
		if(currentGrpScriptCheckBox->isChecked()) {
			return FieldArchive::GrpScriptScope;
		}
		if(currentScriptCheckBox->isChecked()) {
			return FieldArchive::ScriptScope;
		}
	} else {
		if(currentFieldCheckBox2->isChecked()) {
			return FieldArchive::FieldScope;
		}
		if(currentTextCheckBox->isChecked()) {
			return FieldArchive::TextScope;
		}
	}

	return FieldArchive::GlobalScope;
}

QString Search::lastMessage() const
{
	switch(searchScope()) {
	case FieldArchive::GlobalScope:		return tr("Dernier écran");
	case FieldArchive::FieldScope:		return tr("Dernier groupe");
	case FieldArchive::GrpScriptScope:	return tr("Dernier script");
	case FieldArchive::ScriptScope:		return tr("Dernière instruction");
	case FieldArchive::TextScope:		return tr("Dernier texte");
	}
	return QString();
}

QString Search::firstMessage() const
{
	switch(searchScope()) {
	case FieldArchive::GlobalScope:		return tr("Premier écran");
	case FieldArchive::FieldScope:		return tr("Premier groupe");
	case FieldArchive::GrpScriptScope:	return tr("Premier script");
	case FieldArchive::ScriptScope:		return tr("Première instruction");
	case FieldArchive::TextScope:		return tr("Premier texte");
	}
	return QString();
}

void Search::findNext()
{
	if(!fieldArchive) {
		return;
	}

	mainWindow()->setEnabled(false);
	buttonNext->setDefault(true);
	returnToBegin->hide();

	int fieldID, grpScriptID, scriptID, opcodeID,
			textID, from;
	FieldArchive::Sorting sorting = mainWindow()->getFieldSorting();
	FieldArchive::SearchScope scope = searchScope();
	
	setSearchValues();

	atTheBeginning = false;

	if(atTheEnd) {
		fieldID = scope == FieldArchive::FieldScope ? mainWindow()->currentFieldId() : -1;
		grpScriptID = scope == FieldArchive::GrpScriptScope ? mainWindow()->currentGrpScriptId() : -1;
		scriptID = scope == FieldArchive::ScriptScope ? mainWindow()->currentScriptId() : -1;
		opcodeID = -1;
		textID = scope == FieldArchive::TextScope && mainWindow()->textDialog() ? mainWindow()->textDialog()->currentTextId() : -1;
		from = 0;
		atTheEnd = false;
	} else {
		fieldID = mainWindow()->currentFieldId();
		grpScriptID = mainWindow()->currentGrpScriptId();
		scriptID = mainWindow()->currentScriptId();
		opcodeID = mainWindow()->currentOpcodeId();
		textID = mainWindow()->textDialog() ? mainWindow()->textDialog()->currentTextId() : -1;
		from = mainWindow()->textDialog() ? qMax(mainWindow()->textDialog()->currentTextPosition(),
												 mainWindow()->textDialog()->currentAnchorPosition())
										  : 0;
	}

	if(tabWidget->currentIndex() == 0) { // scripts page
		++opcodeID;

		switch(liste->currentIndex())
		{
		case 0:
			if(fieldArchive->searchTextInScripts(text, fieldID, grpScriptID,
												 scriptID, opcodeID,
												 sorting, scope)) {
				emit found(fieldID, grpScriptID, scriptID, opcodeID);
				goto after;
			}
			break;
		case 1:
			if(fieldArchive->searchVar(bank, adress, value,
									   fieldID, grpScriptID,
									   scriptID, opcodeID,
									   sorting, scope)) {
				emit found(fieldID, grpScriptID, scriptID, opcodeID);
				goto after;
			}
			break;
		case 2:
			if(fieldArchive->searchOpcode(clef, fieldID, grpScriptID,
										  scriptID, opcodeID,
										  sorting, scope)) {
				emit found(fieldID, grpScriptID, scriptID, opcodeID);
				goto after;
			}
			break;
		case 4:
			if(fieldArchive->searchMapJump(field, fieldID, grpScriptID,
										   scriptID, opcodeID,
										   sorting, scope)) {
				emit found(fieldID, grpScriptID, scriptID, opcodeID);
				goto after;
			}
			break;
		case 3:
			if(fieldArchive->searchExec(e_group, e_script, fieldID,
										grpScriptID, scriptID, opcodeID,
										sorting, scope)) {
				emit found(fieldID, grpScriptID, scriptID, opcodeID);
				goto after;
			}
			break;
		}
	} else { // texts page
		int size;
		if(fieldArchive->searchText(text, fieldID, textID, from,
									size, sorting, scope)) {
			emit foundText(fieldID, textID, from, size);
			goto after;
		}
	}

	returnToBegin->setText(tr("%1,\npoursuite au début.")
						   .arg(lastMessage()));
	returnToBegin->show();

	atTheEnd = true;

after:
	mainWindow()->setEnabled(true);
}

void Search::findPrev()
{
	if(!fieldArchive) {
		return;
	}

	mainWindow()->setEnabled(false);
	buttonPrev->setDefault(true);
	returnToBegin->hide();

	int fieldID, grpScriptID, scriptID, opcodeID,
			textID, from;
	FieldArchive::Sorting sorting = mainWindow()->getFieldSorting();
	FieldArchive::SearchScope scope = searchScope();

	setSearchValues();

	atTheEnd = false;

	if(atTheBeginning) {
		fieldID = scope == FieldArchive::FieldScope ? mainWindow()->currentFieldId() : 2147483647;
		grpScriptID = scope == FieldArchive::GrpScriptScope ? mainWindow()->currentGrpScriptId() : 2147483647;
		scriptID = scope == FieldArchive::ScriptScope ? mainWindow()->currentScriptId() : 2147483647;
		opcodeID = 2147483647;
		textID = scope == FieldArchive::TextScope && mainWindow()->textDialog() ? mainWindow()->textDialog()->currentTextId() : 2147483647;
		from = -1;
		atTheBeginning = false;
	} else {
		fieldID = mainWindow()->currentFieldId();
		grpScriptID = mainWindow()->currentGrpScriptId();
		scriptID = mainWindow()->currentScriptId();
		opcodeID = mainWindow()->currentOpcodeId();
		textID = mainWindow()->textDialog() ? mainWindow()->textDialog()->currentTextId() : -1;
		from = mainWindow()->textDialog() ? qMin(mainWindow()->textDialog()->currentTextPosition(),
												 mainWindow()->textDialog()->currentAnchorPosition())
										  : -1;
		if(from <= 0) {
			textID--;
		}
	}

	if(tabWidget->currentIndex() == 0) { // scripts page
		--opcodeID;

		switch(liste->currentIndex())
		{
		case 0:
			if(fieldArchive->searchTextInScriptsP(text, fieldID, grpScriptID,
												  scriptID, opcodeID,
												  sorting, scope)) {
				emit found(fieldID, grpScriptID, scriptID, opcodeID);
				goto after;
			}
			break;
		case 1:
			if(fieldArchive->searchVarP(bank, adress, value,
										fieldID, grpScriptID,
										scriptID, opcodeID,
										sorting, scope)) {
				emit found(fieldID, grpScriptID, scriptID, opcodeID);
				goto after;
			}
			break;
		case 2:
			if(fieldArchive->searchOpcodeP(clef, fieldID, grpScriptID,
										   scriptID, opcodeID,
										   sorting, scope)) {
				emit found(fieldID, grpScriptID, scriptID, opcodeID);
				goto after;
			}
			break;
		case 4:
			if(fieldArchive->searchMapJumpP(field, fieldID, grpScriptID,
											scriptID, opcodeID,
											sorting, scope)) {
				emit found(fieldID, grpScriptID, scriptID, opcodeID);
				goto after;
			}
			break;
		case 3:
			if(fieldArchive->searchExecP(e_group, e_script, fieldID,
										 grpScriptID, scriptID,
										 opcodeID, sorting, scope)) {
				emit found(fieldID, grpScriptID, scriptID, opcodeID);
				goto after;
			}
			break;
		}
	} else { // texts page
		--from;
		int index, size;
		if(fieldArchive->searchTextP(text, fieldID, textID,
									 from, index, size,
									 sorting, scope)) {
			emit foundText(fieldID, textID, index, size);
			goto after;
		}
	}

	returnToBegin->setText(tr("%1,\npoursuite à la fin.")
						   .arg(firstMessage()));
	returnToBegin->show();

	atTheBeginning = true;

after:
	mainWindow()->setEnabled(true);
}

void Search::setSearchValues()
{
	bool ok;
	if(tabWidget->currentIndex() == 0) { // scripts page
		switch(liste->currentIndex())
		{
		case 0:
		{
			QString lineEditText = champ->lineEdit()->text();
			text = QRegExp(useRegexp->isChecked() ? lineEditText : QRegExp::escape(lineEditText), caseSens->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
			QStringList recentSearch = Config::value("recentSearch").toStringList();
			int index;
			if((index = recentSearch.indexOf(lineEditText)) != -1) {
				recentSearch.removeAt(index);
			}
			recentSearch.prepend(lineEditText);
			champ->clear();
			champ->addItems(recentSearch);
			champ->setCurrentIndex(0);
			if(recentSearch.size() > 20) {
				recentSearch.removeLast();
			}
			Config::setValue("recentSearch", recentSearch);
			Config::setValue("findWithCaseSensitive", caseSens->isChecked());
			Config::setValue("findWithRegExp", useRegexp->isChecked());
		}
			break;
		case 1:
			bank = champBank->value();
			adress = champAdress->value();
			value = champValue->text().toInt(&ok);
			if(!ok)	value = 65536;
			Config::setValue("SearchedVarBank", bank);
			Config::setValue("SearchedVarAdress", adress);
			Config::setValue("SearchedVarValue", value);
			break;
		case 2:
			clef = opcode->currentIndex();
			Config::setValue("SearchedOpcode", clef);
			break;
		case 3:
			e_group = executionGroup->currentIndex();
			e_script = executionScript->value();
			Config::setValue("SearchedGroupScript", e_group);
			Config::setValue("SearchedScript", e_script);
			break;
		case 4:
			field = mapJump->currentIndex();
			Config::setValue("SearchedMapJump", field);
			break;
		default:return;
		}
	} else { // texts page
		QString lineEditText = champ2->lineEdit()->text();
		text = QRegExp(useRegexp2->isChecked() ? lineEditText : QRegExp::escape(lineEditText), caseSens2->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
		QStringList recentSearch = Config::value("recentSearch").toStringList();
		int index;
		if((index = recentSearch.indexOf(lineEditText)) != -1) {
			recentSearch.removeAt(index);
		}
		recentSearch.prepend(lineEditText);
		champ2->clear();
		champ2->addItems(recentSearch);
		champ2->setCurrentIndex(0);
		if(recentSearch.size() > 20) {
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
	if(!fieldArchive || !mainWindow()->textDialog()) {
		return;
	}

	setSearchValues();

	if(fieldArchive->replaceText(text, replace2->lineEdit()->text(),
								 mainWindow()->currentFieldId(),
								 mainWindow()->textDialog()->currentTextId(),
								 qMin(mainWindow()->textDialog()->currentTextPosition(),
									  mainWindow()->textDialog()->currentAnchorPosition()))) {
		// Update view
		mainWindow()->textDialog()->updateText();
		mainWindow()->setModified();
	}
}

void Search::replaceAll()
{
	if(!fieldArchive) {
		return;
	}

	mainWindow()->setEnabled(false);
	FieldArchive::Sorting sorting = mainWindow()->getFieldSorting();
	FieldArchive::SearchScope scope = searchScope();
	setSearchValues();

	QString after = replace2->lineEdit()->text();
	int fieldID = 0, textID = 0, from = 0, size;

	if(scope == FieldArchive::FieldScope) {
		fieldID = mainWindow()->currentFieldId();
	} else if(scope == FieldArchive::TextScope) {
		if(mainWindow()->textDialog()) {
			textID = mainWindow()->textDialog()->currentTextId();
		}
	}

	bool modified = false;
	while(fieldArchive->searchText(text, fieldID, textID, from, size, sorting, scope)) {
		if(fieldArchive->replaceText(text, after, fieldID, textID, from)) {
			modified = true;
		}
		from += after.size();
	}

	mainWindow()->setEnabled(true);

	if(modified) {

		// Update view
		if(mainWindow()->textDialog()) {
			mainWindow()->textDialog()->updateText();
		}
		mainWindow()->setModified();
	}
}
