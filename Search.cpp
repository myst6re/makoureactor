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

Search::Search(QWidget *parent)
	: QDialog(parent, Qt::Tool), fieldID(0), grpScriptID(0), scriptID(0), opcodeID(0), clef(0), text(QString()), bank(0), adress(0), e_script(0), e_group(0)
{
	setWindowTitle(tr("Rechercher"));
	activateWindow();
	
	liste = new QComboBox(this);
	liste->addItem(tr("Texte"));
	liste->addItem(tr("Variable"));
	liste->addItem(tr("Opcode"));
	liste->addItem(tr("Exec"));

	QWidget *texte = new QWidget(this);

	champ = new QComboBox(texte);
	champ->setEditable(true);
	champ->setMaximumWidth(400);
	champ->addItems(Config::value("recentSearch").toStringList());

	caseSens = new QCheckBox(tr("Sensible à la casse"), texte);
	caseSens->setChecked(Config::value("findWithCaseSensitive").toBool());
	useRegexp = new QCheckBox(tr("Utiliser les expressions régulières"), texte);
	useRegexp->setChecked(Config::value("findWithRegExp").toBool());

	QGridLayout *textLayout = new QGridLayout(texte);
	textLayout->setContentsMargins(QMargins());
	textLayout->addWidget(champ, 0, 0, 1, 2);
	textLayout->addWidget(caseSens, 1, 0);
	textLayout->addWidget(useRegexp, 1, 1);
	
	QWidget *opc = new QWidget(this);
	opcode = new QComboBox(opc);
	for(quint16 i=0 ; i<256 ; ++i)
		opcode->addItem(QString("%1 - %2").arg(i,2,16,QChar('0')).arg(Opcode::names[i]));
	opcode->addItem(QString("100 - %1").arg(Opcode::names[0x100]));
	QVBoxLayout *opcodeLayout = new QVBoxLayout(opc);
	opcodeLayout->setContentsMargins(QMargins());
	opcodeLayout->addWidget(opcode);
	opcodeLayout->addStretch();
	
	QWidget *variable = new QWidget(this);
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

	QWidget *execution = new QWidget(this);
	executionGroup = new QComboBox(execution);
	executionScript = new QSpinBox(execution);
	executionScript->setRange(0,31);

	QGridLayout *executionLayout = new QGridLayout(execution);
	executionLayout->setContentsMargins(QMargins());
	executionLayout->addWidget(new QLabel(tr("Script")), 0, 0);
	executionLayout->addWidget(executionScript, 0, 1);
	executionLayout->addWidget(new QLabel(tr("Groupe")), 1, 0);
	executionLayout->addWidget(executionGroup, 1, 1);
	executionLayout->setColumnStretch(1, 1);
	
	QStackedWidget *stack = new QStackedWidget(this);
	stack->addWidget(texte);
	stack->addWidget(variable);
	stack->addWidget(opc);
	stack->addWidget(execution);
	
	buttonSuiv = new QPushButton(tr("Chercher le suivant"), this);
	buttonPrec = new QPushButton(tr("Chercher le précédent"), this);
	buttonSuiv->setFixedSize(buttonPrec->sizeHint());
	buttonPrec->setAutoDefault(false);
	buttonSuiv->setAutoDefault(false);
	buttonSuiv->setDefault(true);

	returnToBegin = new QLabel();
	returnToBegin->setWordWrap(true);
	returnToBegin->setAlignment(Qt::AlignRight);
	QPalette pal = returnToBegin->palette();
	pal.setColor(QPalette::Active, QPalette::WindowText, Qt::red);
	returnToBegin->setPalette(pal);
	returnToBegin->hide();

	QHBoxLayout *topLayout = new QHBoxLayout();
	topLayout->addWidget(liste, 0, Qt::AlignTop);
	topLayout->addWidget(stack, 1, Qt::AlignTop);
	topLayout->addStretch();

	grid = new QGridLayout(this);
	grid->addLayout(topLayout, 0, 0, 1, 3, Qt::AlignTop);
	grid->addWidget(buttonSuiv, 1, 1, Qt::AlignHCenter);
	grid->addWidget(buttonPrec, 2, 1, Qt::AlignHCenter);
	grid->addWidget(returnToBegin, 1, 2, 2, 1, Qt::AlignRight | Qt::AlignBottom);
	
	connect(buttonSuiv, SIGNAL(released()), SLOT(chercherSuivant()));
	connect(buttonPrec, SIGNAL(released()), SLOT(chercherPrecedent()));
	connect(liste, SIGNAL(currentIndexChanged(int)), stack, SLOT(setCurrentIndex(int)));
	connect(champBank, SIGNAL(valueChanged(int)), SLOT(updateComboVarName()));
	connect(champAdress, SIGNAL(valueChanged(int)), comboVarName, SLOT(setCurrentIndex(int)));
	connect(comboVarName, SIGNAL(currentIndexChanged(int)), SLOT(updateChampAdress()));
}

void Search::setFieldArchive(FieldArchive *fieldArchive)
{
	this->fieldArchive = fieldArchive;
}

void Search::setOpcode(int opcode)
{
	this->opcode->setCurrentIndex(opcode);
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
	int i=0;
	foreach(const QString &name, Data::currentGrpScriptNames)
		executionGroup->addItem(QString("%1 - %2").arg(i++).arg(name));
}

void Search::changeFileID(int fieldID)
{
	this->fieldID = fieldID;
	grpScriptID = scriptID = opcodeID = -1;
}

void Search::changeGrpScriptID(int grpScriptID)
{
	this->grpScriptID = grpScriptID;
	scriptID = opcodeID = -1;
}

void Search::changeScriptID(int scriptID)
{
	this->scriptID = scriptID;
	opcodeID = -1;
}

void Search::changeOpcodeID(int opcodeID)
{
	this->opcodeID = opcodeID;
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

void Search::chercherSuivant()
{
	parentWidget()->setEnabled(false);
	buttonSuiv->setDefault(true);
	returnToBegin->hide();

	bool localSearch=false;
	const QList<FF7Text *> *currentTextesSav = Data::currentTextes;
	FieldArchive::Sorting sorting = ((Window *)parentWidget())->getFieldSorting();

	++opcodeID;
	
	setSearchValues();

	switch(liste->currentIndex())
	{
	case 0:
		if(fieldArchive->searchText(text, fieldID, grpScriptID, scriptID, opcodeID, sorting))
		{
			emit found(fieldID, grpScriptID, scriptID, opcodeID);
			goto after;
		}
		break;
	case 1:
		if(fieldArchive->searchVar(bank, adress, value, fieldID, grpScriptID, scriptID, opcodeID, sorting))
		{
			emit found(fieldID, grpScriptID, scriptID, opcodeID);
			goto after;
		}
		break;
	case 2:
		if(fieldArchive->searchOpcode(clef, fieldID, grpScriptID, scriptID, opcodeID, sorting))
		{
			emit found(fieldID, grpScriptID, scriptID, opcodeID);
			goto after;
		}
		break;
	case 3:
		if(fieldID < 0)							fieldID = 0;
		if(fieldID > fieldArchive->size())		fieldID = fieldArchive->size()-1;
		Field *currentField = fieldArchive->field(fieldID);
		if(currentField && currentField->searchExec(e_group, e_script, grpScriptID, scriptID, opcodeID))
		{
			emit found(fieldID, grpScriptID, scriptID, opcodeID);
			goto after;
		}
		localSearch = true;
		break;
	}

	returnToBegin->setText(tr("Dernier %1,\npoursuite au début.")
						   .arg(localSearch ? tr("groupe") : tr("écran")));
	returnToBegin->show();

	Data::currentTextes = currentTextesSav;
	if(!localSearch)	fieldID = -1;
	grpScriptID = scriptID = opcodeID = -1;

after:
	parentWidget()->setEnabled(true);
}

void Search::chercherPrecedent()
{
	parentWidget()->setEnabled(false);
	buttonPrec->setDefault(true);
	returnToBegin->hide();

	bool localSearch=false;
	const QList<FF7Text *> *currentTextesSav = Data::currentTextes;
	FieldArchive::Sorting sorting = ((Window *)parentWidget())->getFieldSorting();

	--opcodeID;

	setSearchValues();

	switch(liste->currentIndex())
	{
	case 0:
		if(fieldArchive->searchTextP(text, fieldID, grpScriptID, scriptID, opcodeID, sorting))
		{
			emit found(fieldID, grpScriptID, scriptID, opcodeID);
			goto after;
		}
		break;
	case 1:
		if(fieldArchive->searchVarP(bank, adress, value, fieldID, grpScriptID, scriptID, opcodeID, sorting))
		{
			emit found(fieldID, grpScriptID, scriptID, opcodeID);
			goto after;
		}
		break;
	case 2:
		if(fieldArchive->searchOpcodeP(clef, fieldID, grpScriptID, scriptID, opcodeID, sorting))
		{
			emit found(fieldID, grpScriptID, scriptID, opcodeID);
			goto after;
		}
		break;
	case 3:
		if(fieldID < 0)							fieldID = 0;
		if(fieldID > fieldArchive->size())		fieldID = fieldArchive->size()-1;
		Field *currentField = fieldArchive->field(fieldID);
		if(currentField && currentField->searchExecP(e_group, e_script, grpScriptID, scriptID, opcodeID))
		{
			emit found(fieldID, grpScriptID, scriptID, opcodeID);
			goto after;
		}
		localSearch = true;
		break;
	}

	returnToBegin->setText(tr("Premier %1,\npoursuite à la fin.")
						   .arg(localSearch ? tr("groupe") : tr("écran")));
	returnToBegin->show();

	Data::currentTextes = currentTextesSav;
	if(!localSearch)	fieldID = 2147483647;
	grpScriptID = scriptID = opcodeID = 2147483647;

after:
	parentWidget()->setEnabled(true);
}

void Search::setSearchValues()
{
	bool ok;
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
		break;
	case 2:
		clef = opcode->currentIndex();
		break;
	case 3:
		e_group = executionGroup->currentIndex();
		e_script = executionScript->value();
		break;
	default:return;
	}
}
