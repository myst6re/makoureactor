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
	: QDialog(parent, Qt::Tool), fileID(0), grpScriptID(0), scriptID(0), opcodeID(0), clef(0), text(QString()), bank(0), adress(0), e_script(0), e_group(0)
{
	setWindowTitle(tr("Rechercher"));
	activateWindow();
	
	grid = new QGridLayout(this);
	
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
	variableLayout->setRowStretch(1, 1);
	champBank->setRange(1,15);
	champAdress->setRange(0,255);

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
	
	grid->addWidget(liste, 0, 0, Qt::AlignTop);
	grid->addWidget(stack, 0, 1, Qt::AlignTop);
	grid->addWidget(buttonSuiv, 1, 0, 1, 2, Qt::AlignHCenter);
	grid->addWidget(buttonPrec, 2, 0, 1, 2, Qt::AlignHCenter);
	
	connect(buttonSuiv, SIGNAL(released()), SLOT(chercherSuivant()));
	connect(buttonPrec, SIGNAL(released()), SLOT(chercherPrecedent()));
	connect(liste, SIGNAL(currentIndexChanged(int)), stack, SLOT(setCurrentIndex(int)));
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

void Search::changeFileID(int fileID)
{
	this->fileID = fileID;
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

void Search::cancelSearching()
{
	cancel = true;
}

void Search::chercherSuivant()
{
	parentWidget()->setEnabled(false);
	buttonSuiv->setDefault(true);

	const QList<FF7Text *> *currentTextesSav = Data::currentTextes;
	FieldArchive::Sorting sorting = ((Window *)parentWidget())->getFieldSorting();

	++opcodeID;
	
	setSearchValues();

	switch(liste->currentIndex())
	{
	case 0:
		if(fieldArchive->rechercherTexte(text, fileID, grpScriptID, scriptID, opcodeID, sorting))
		{
			emit found(fileID, grpScriptID, scriptID, opcodeID);
			goto after;
		}
		break;
	case 1:
		if(fieldArchive->rechercherVar(bank, adress, value, fileID, grpScriptID, scriptID, opcodeID, sorting))
		{
			emit found(fileID, grpScriptID, scriptID, opcodeID);
			goto after;
		}
		break;
	case 2:
		if(fieldArchive->rechercherOpcode(clef, fileID, grpScriptID, scriptID, opcodeID, sorting))
		{
			emit found(fileID, grpScriptID, scriptID, opcodeID);
			goto after;
		}
		break;
	case 3:
		if(fieldArchive->rechercherExec(e_group, e_script, fileID, grpScriptID, scriptID, opcodeID, sorting))
		{
			emit found(fileID, grpScriptID, scriptID, opcodeID);
			goto after;
		}
		break;
	}
	
	QMessageBox::information(this, windowTitle(), tr("Dernier fichier,\npoursuite de la recherche dans le premier fichier."));

	Data::currentTextes = currentTextesSav;
	this->fileID = grpScriptID = scriptID = opcodeID = -1;

after:
	parentWidget()->setEnabled(true);
}

void Search::chercherPrecedent()
{
	parentWidget()->setEnabled(false);
	buttonPrec->setDefault(true);

	const QList<FF7Text *> *currentTextesSav = Data::currentTextes;
	FieldArchive::Sorting sorting = ((Window *)parentWidget())->getFieldSorting();

	--opcodeID;

	setSearchValues();

	switch(liste->currentIndex())
	{
	case 0:
		if(fieldArchive->rechercherTexteP(text, fileID, grpScriptID, scriptID, opcodeID, sorting))
		{
			emit found(fileID, grpScriptID, scriptID, opcodeID);
			goto after;
		}
		break;
	case 1:
		if(fieldArchive->rechercherVarP(bank, adress, value, fileID, grpScriptID, scriptID, opcodeID, sorting))
		{
			emit found(fileID, grpScriptID, scriptID, opcodeID);
			goto after;
		}
		break;
	case 2:
		if(fieldArchive->rechercherOpcodeP(clef, fileID, grpScriptID, scriptID, opcodeID, sorting))
		{
			emit found(fileID, grpScriptID, scriptID, opcodeID);
			goto after;
		}
		break;
	case 3:
		if(fieldArchive->rechercherExecP(e_group, e_script, fileID, grpScriptID, scriptID, opcodeID, sorting))
		{
			emit found(fileID, grpScriptID, scriptID, opcodeID);
			goto after;
		}
		break;
	}

	QMessageBox::information(this, windowTitle(), tr("Premier fichier,\npoursuite de la recherche dans le dernier fichier."));

	Data::currentTextes = currentTextesSav;
	fileID = fieldArchive->size()-1;
	if(fileID >= 0) {
		Field *field = fieldArchive->field(fileID);
		if(field != NULL) {
			grpScriptID = field->grpScripts.size()-1;
			if(grpScriptID >= 0) {
				scriptID = field->grpScripts.at(grpScriptID)->size()-1;
				if(scriptID >= 0) {
					opcodeID = field->grpScripts.at(grpScriptID)->getScript(scriptID)->size();
				}
			}
		} else {
			grpScriptID = scriptID = opcodeID = 65535;
		}
	}

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
