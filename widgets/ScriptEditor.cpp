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
#include "ScriptEditor.h"
#include "ScriptEditorWidgets/ScriptEditorGenericList.h"
#include "ScriptEditorWidgets/ScriptEditorStructPage.h"
#include "ScriptEditorWidgets/ScriptEditorMathPage.h"
#include "ScriptEditorWidgets/ScriptEditorWindowPage.h"

#define JUMP_PAGE	5
#define IF_PAGE		6
#define IFKEY_PAGE	7
#define IFQ_PAGE	8

QList<quint8> ScriptEditor::crashIfInit;

ScriptEditor::ScriptEditor(Field *field, GrpScript *grpScript, Script *script, int opcodeID, bool modify, bool isInit, QWidget *parent) :
	QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint),
	field(field), script(script), opcodeID(opcodeID), isInit(isInit), modify(modify), change(false)
{
	if(crashIfInit.isEmpty()) {
		crashIfInit << 0x08 << 0x09 << 0x0E << 0x20 << 0x21 << 0x2A << 0x35 << 0x40 << 0x48 << 0x49
					<< 0x60 << 0x70 << 0xA2 << 0xA3 << 0xA8 << 0xA9 << 0xAD << 0xAE << 0xAF << 0xB0 << 0xB1
					<< 0xB4 << 0xB5 << 0xBA << 0xBB << 0xBC << 0xC0 << 0xC2;
	}

	setWindowTitle(tr("Éditeur de script%1").arg(isInit ? tr(" (init mode)") : ""));
	setMinimumSize(500, 400);
	
	QStringList liste0;
	liste0 << tr("Structures de contrôle") <<
			  tr("Opérations mathématiques") <<
			  tr("Fenêtres et messages") <<
			  tr("Équipe et inventaire") <<
			  tr("Objets 3D et animations") <<
			  tr("Zones") <<
			  tr("Background") <<
			  tr("Transitions et caméra") <<
			  tr("Audio et vidéo") <<
			  tr("Modules") <<
			  tr("Inconnu");
	
	comboBox0 = new QComboBox(this);
	comboBox0->addItems(liste0);
	comboBox = new QComboBox(this);
	buildList(0);
	
	textEdit = new QPlainTextEdit(this);
	textEdit->setReadOnly(true);
	textEdit->setFixedHeight(38);
	
	editorLayout = new QStackedWidget;
	editorLayout->addWidget(editorWidget = new ScriptEditorGenericList(field, grpScript, script, opcodeID, this));
	editorLayout->addWidget(new ScriptEditorReturnToPage(field, grpScript, script, opcodeID, this));
	editorLayout->addWidget(new ScriptEditorExecPage(field, grpScript, script, opcodeID, this));
	editorLayout->addWidget(new ScriptEditorExecCharPage(field, grpScript, script, opcodeID, this));
	editorLayout->addWidget(new ScriptEditorLabelPage(field, grpScript, script, opcodeID, this));
	editorLayout->addWidget(new ScriptEditorJumpPage(field, grpScript, script, opcodeID, this));
	editorLayout->addWidget(new ScriptEditorIfPage(field, grpScript, script, opcodeID, this));
	editorLayout->addWidget(new ScriptEditorIfKeyPage(field, grpScript, script, opcodeID, this));
	editorLayout->addWidget(new ScriptEditorIfQPage(field, grpScript, script, opcodeID, this));
	editorLayout->addWidget(new ScriptEditorWaitPage(field, grpScript, script, opcodeID, this));
	editorLayout->addWidget(new ScriptEditorBinaryOpPage(field, grpScript, script, opcodeID, this));
	editorLayout->addWidget(new ScriptEditorUnaryOpPage(field, grpScript, script, opcodeID, this));
	editorLayout->addWidget(new ScriptEditorBitOpPage(field, grpScript, script, opcodeID, this));
	editorLayout->addWidget(new ScriptEditorWindowPage(field, grpScript, script, opcodeID, this));
	editorLayout->addWidget(new ScriptEditorWindowModePage(field, grpScript, script, opcodeID, this));
	editorLayout->addWidget(new ScriptEditorWindowMovePage(field, grpScript, script, opcodeID, this));

	ok = new QPushButton(tr("OK"),this);
	ok->setDefault(true);
	QPushButton *cancel = new QPushButton(tr("Annuler"),this);
	
	QHBoxLayout *buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch();
	buttonLayout->addWidget(ok);
	buttonLayout->addWidget(cancel);
	
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(comboBox0);
	layout->addWidget(comboBox);
	layout->addWidget(textEdit);
	layout->addWidget(editorLayout);
	layout->addStretch();
	layout->addLayout(buttonLayout);

	if(modify) {
		this->opcode = Script::copyOpcode(script->opcode(opcodeID));
		int id = opcode->id();

		if(id == Opcode::SPECIAL)
			id = (((OpcodeSPECIAL *)opcode)->opcode->id() << 8) | id;
		else if(id == Opcode::KAWAI)
			id = (((OpcodeKAWAI *)opcode)->opcode->id() << 8) | id;

		setCurrentMenu(id);
		fillView();
	} else {
		this->opcode = new OpcodeRET();
		comboBox->setCurrentIndex(0);
		changeCurrentOpcode(0);
	}

	connect(ok, SIGNAL(released()), SLOT(accept()));
	connect(cancel, SIGNAL(released()), SLOT(close()));
	connect(comboBox0, SIGNAL(currentIndexChanged(int)), SLOT(buildList(int)));
	for(int i=0 ; i<editorLayout->count() ; ++i)
		connect((ScriptEditorView *)editorLayout->widget(i), SIGNAL(opcodeChanged()), SLOT(refreshTextEdit()));
	connect(comboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeCurrentOpcode(int)));
}

ScriptEditor::~ScriptEditor()
{
	Opcode *op = editorWidget->opcode();
	if(op) {
		delete op;
	} else {
		delete this->opcode;
	}
}

void ScriptEditor::fillEditor()
{
	int index;

	editorWidget->clear();

	// Change current editor widget
	switch((Opcode::Keys)opcode->id()) {
	case Opcode::RETTO:
		index = 1;
		break;
	case Opcode::REQEW:case Opcode::REQ:
	case Opcode::REQSW:
		index = 2;
		break;
	case Opcode::PRQEW:case Opcode::PREQ:
	case Opcode::PRQSW:
		index = 3;
		break;
	case Opcode::LABEL:
		index = 4;
		break;
	case Opcode::JMPF:case Opcode::JMPFL:
	case Opcode::JMPB:case Opcode::JMPBL:
		index = JUMP_PAGE;
		break;
	case Opcode::IFUB:case Opcode::IFUBL:
	case Opcode::IFSW:case Opcode::IFSWL:
	case Opcode::IFUW:case Opcode::IFUWL:
		index = IF_PAGE;
		break;
	case Opcode::IFKEY:case Opcode::IFKEYON:
	case Opcode::IFKEYOFF:
		index = IFKEY_PAGE;
		break;
	case Opcode::IFMEMBQ:case Opcode::IFPRTYQ:
		index = IFQ_PAGE;
		break;
	case Opcode::WAIT:
		index = 9;
		break;
	case Opcode::PLUSX:case Opcode::PLUS2X:
	case Opcode::MINUSX:case Opcode::MINUS2X:
	case Opcode::SETBYTE:case Opcode::SETWORD:
	case Opcode::PLUS:case Opcode::PLUS2:
	case Opcode::MINUS:case Opcode::MINUS2:
	case Opcode::MUL:case Opcode::MUL2:
	case Opcode::DIV:case Opcode::DIV2:
	case Opcode::MOD:case Opcode::MOD2:
	case Opcode::AND:case Opcode::AND2:
	case Opcode::OR:case Opcode::OR2:
	case Opcode::XOR:case Opcode::XOR2:
	case Opcode::LBYTE:case Opcode::HBYTE:
		index = 10;
		break;
	case Opcode::INCX:case Opcode::INC2X:
	case Opcode::INC:case Opcode::INC2:
	case Opcode::DECX:case Opcode::DEC2X:
	case Opcode::DEC:case Opcode::DEC2:
	case Opcode::RANDOM:
		index = 11;
		break;
	case Opcode::BITOFF:case Opcode::BITON:
	case Opcode::BITXOR:
		index = 12;
		break;
	case Opcode::WINDOW:case Opcode::WSIZW:
	case Opcode::WROW:
		index = 13;
		break;
	case Opcode::WMODE:
		index = 14;
		break;
	case Opcode::WMOVE:
		index = 15;
		break;
	default:
		index = 0;
	}

	editorWidget = (ScriptEditorView *)editorLayout->widget(index);
	bool hasParams = opcode->hasParams() || opcode->isLabel();
	editorLayout->setVisible(hasParams);
	editorLayout->setCurrentWidget(editorWidget);
	editorWidget->setOpcode(opcode);
}

void ScriptEditor::fillView()
{
	textEdit->setPlainText(opcode->toString(field)); // text part
	fillEditor(); // editor part

	// disable if necessary
	bool disableEditor = (isInit && crashIfInit.contains(opcode->id())) || !editorWidget->isValid();

	textEdit->setDisabled(disableEditor);
	editorWidget->setDisabled(disableEditor);
	ok->setDisabled(disableEditor);
}

void ScriptEditor::accept()
{
	/* if(!this->change) {
		close();
		return;
	} */
	if(needslabel()) {
		script->insertOpcode(this->opcodeID, new OpcodeLabel(((OpcodeJump *)editorWidget->opcode())->label()));
	}

	if(modify) {
		script->setOpcode(this->opcodeID, Script::copyOpcode(editorWidget->opcode()));
	} else {
		script->insertOpcode(this->opcodeID, Script::copyOpcode(editorWidget->opcode()));
	}
	QDialog::accept();
}

bool ScriptEditor::needslabel() const
{
	return (editorLayout->currentIndex() == JUMP_PAGE
			|| editorLayout->currentIndex() == IF_PAGE
			|| editorLayout->currentIndex() == IFKEY_PAGE
			|| editorLayout->currentIndex() == IFQ_PAGE)
			&& ((ScriptEditorJumpPageInterface *)editorWidget)->needsLabel();
}

void ScriptEditor::refreshTextEdit()
{
	this->change = true;

	opcode = editorWidget->opcode();

	textEdit->setPlainText(opcode->toString(field));
}

void ScriptEditor::changeCurrentOpcode(int index)
{
	this->change = true;

	const QList<QVariant> itemDataList = comboBox->itemData(index).toList();
	const int itemData = !itemDataList.isEmpty() ? itemDataList.first().toInt() : 0;

	// Create opcode

	if(Opcode::LABEL == itemData) { // LABEL exception
		if(Opcode::LABEL == opcode->id()) {
			((OpcodeLabel *)opcode)->setLabel(0);
		} else {
			delete opcode;
			opcode = new OpcodeLabel(0);
		}
	} else {
		const quint8 id = itemData & 0xFF;
		QByteArray newOpcode;
		newOpcode.append((char)id);

		// Fill opcode with \x00

		if(Opcode::KAWAI == id) { //KAWAI
			newOpcode.append('\x03'); // size
			newOpcode.append((char)((itemData >> 8) & 0xFF)); // KAWAI ID
		} else if(Opcode::SPECIAL == id) { //SPECIAL
			quint8 byte2 = (itemData >> 8) & 0xFF;
			newOpcode.append((char)byte2); // SPECIAL ID
			switch(byte2)
			{
			case 0xF5:case 0xF6:case 0xF7:case 0xFB:case 0xFC:
				newOpcode.append('\x00');
				break;
			case 0xF8:case 0xFD:
				newOpcode.append('\x00');
				newOpcode.append('\x00');
				break;
			}
		} else {
			for(quint8 pos=1 ; pos<Opcode::length[id] ; ++pos) {
				newOpcode.append('\x00');
			}
		}

		if(id == opcode->id()) {
			opcode->setParams(newOpcode.constData() + 1, newOpcode.size() - 1); // same opcode, just change params
		} else { // change all
			delete opcode;
			opcode = Script::createOpcode(newOpcode);
		}
	}

	fillView();
}

void ScriptEditor::setCurrentMenu(int id)
{
	int index, i, j;
	for(i=0 ; i<comboBox0->count() ; ++i) {
		buildList(i);
		index = -1;
		for(j=0 ; j<comboBox->count() && index==-1 ; ++j) {
			QList<QVariant> dataList = comboBox->itemData(j).toList();
			foreach(const QVariant &v, dataList) {
				if(v.toInt() == id) {
					index = j;
					break;
				}
			}
		}

		if(index != -1) {
			comboBox0->setCurrentIndex(i);
			comboBox->setCurrentIndex(index);
			setEnabled(true);
			return;
		}
	}

	setEnabled(false);
}

void ScriptEditor::buildList(int id)
{
	comboBox->clear();
	switch(id) {
	case 0:
		comboBox->addItem(tr("Retourner"), QList<QVariant>() << 0x00);
		comboBox->addItem(tr("Retourner à"), QList<QVariant>() << 0x07);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Exécuter un script"), QList<QVariant>()
						  << 0x01 << 0x02 << 0x03);
		comboBox->addItem(tr("Exécuter un script d'un équipier"), QList<QVariant>()
						  << 0x04 << 0x05 << 0x06);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Label"), QList<QVariant>() << 0x100);
		comboBox->addItem(tr("Aller au label"), QList<QVariant>() << 0x10 << 0x11 << 0x12 << 0x13);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Si...alors"), QList<QVariant>()
						  << 0x14 << 0x15 << 0x16 << 0x17 << 0x18 << 0x19);
		comboBox->addItem(tr("Si touche pressée"), QList<QVariant>() << 0x30 << 0x31 << 0x32);
		comboBox->addItem(tr("Si personnage membre de l'équipe"), QList<QVariant>() << 0xCB);
		comboBox->addItem(tr("Si personnage existe"), QList<QVariant>() << 0xCC);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Attendre"), QList<QVariant>() << 0x24);
		comboBox->addItem(tr("Ne rien faire"), QList<QVariant>() << 0x5F);
		return;
	case 1:
		comboBox->addItem(tr("Opération binaire"), QList<QVariant>()
						  << 0x80 << 0x81 << 0x76 << 0x85 << 0x77 << 0x86 << 0x78
						  << 0x87 << 0x79 << 0x88 << 0x89 << 0x8A << 0x8B << 0x8C
						  << 0x8D << 0x8E << 0x8F << 0x90 << 0x91 << 0x92 << 0x93
						  << 0x94 << 0x9A << 0x9B);
		comboBox->addItem(tr("Opération unaire"), QList<QVariant>()
						  << 0x95 << 0x96 << 0x7A << 0x7B << 0x97 << 0x98
						  << 0x7C << 0x7D << 0x99);
		comboBox->addItem(tr("Opération bit à bit"), QList<QVariant>()
						  << 0x82 << 0x83 << 0x84);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Seed Random Generator"), QList<QVariant>() << 0x7F);
		comboBox->addItem(tr("Créer un mot de deux octets"), QList<QVariant>() << 0x9C);
		comboBox->addItem(tr("Sinus"), QList<QVariant>() << 0xD4);
		comboBox->addItem(tr("Cosinus"), QList<QVariant>() << 0xD5);
		return;
	case 2:
		comboBox->addItem(tr("Créer une fenêtre"), QList<QVariant>() << 0x50);
		comboBox->addItem(tr("Redimensionner une fenêtre"), QList<QVariant>() << 0x2F);
		comboBox->addItem(tr("Modifier le nombre de lignes dans fenêtre"), QList<QVariant>() << 0x55);
		comboBox->addItem(tr("Déplacer une fenêtre"), QList<QVariant>() << 0x51);
		comboBox->addItem(tr("Modifier type de fenêtre"), QList<QVariant>() << 0x52);
		comboBox->addItem(tr("Remettre une fenêtre à zéro"), QList<QVariant>() << 0x53);
		comboBox->addItem(tr("Fermer une fenêtre (1)"), QList<QVariant>() << 0x2E);
		comboBox->addItem(tr("Fermer une fenêtre (2)"), QList<QVariant>() << 0x54);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Mettre un compteur dans fenêtre"), QList<QVariant>() << 0x36);
		comboBox->addItem(tr("Modifier valeur du compteur dans fenêtre"), QList<QVariant>() << 0x37);
		comboBox->addItem(tr("Compte à rebours"), QList<QVariant>() << 0x38);
		comboBox->addItem(tr("Modifier variable dans fenêtre (8 bits)"), QList<QVariant>() << 0x41);
		comboBox->addItem(tr("Modifier variable dans fenêtre (16 bits)"), QList<QVariant>() << 0x42);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Afficher texte"), QList<QVariant>() << 0x40);
		comboBox->addItem(tr("Poser une question"), QList<QVariant>() << 0x48);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Modifier le nom du lieu"), QList<QVariant>() << 0x43);
		comboBox->addItem(tr("Activer/Désactiver menu"), QList<QVariant>() << 0x4A);
		comboBox->addItem(tr("Obtenir la couleur d'une fenêtre"), QList<QVariant>() << 0x56);
		comboBox->addItem(tr("Changer la couleur d'une fenêtre"), QList<QVariant>() << 0x57);
		return;
	case 3:
		comboBox->addItem(tr("HP/MP restaurés (1)"), QList<QVariant>() << 0x3C);
		comboBox->addItem(tr("HP/MP restaurés (2)"), QList<QVariant>() << 0x3D);
		comboBox->addItem(tr("HP/MP/statut restaurés"), QList<QVariant>() << 0x3E);
		comboBox->addItem(tr("HP/MP restaurés (3)"), QList<QVariant>() << 0x3F);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Augmenter les MPs"), QList<QVariant>() << 0x45);
		comboBox->addItem(tr("Diminuer les MPs"), QList<QVariant>() << 0x47);
		comboBox->addItem(tr("Augmenter les HPs"), QList<QVariant>() << 0x4D);
		comboBox->addItem(tr("Diminuer les HPs"), QList<QVariant>() << 0x4F);
		comboBox->addItem(tr("Ajouter des gils"), QList<QVariant>() << 0x39);
		comboBox->addItem(tr("Retirer des gils"), QList<QVariant>() << 0x3A);
		comboBox->addItem(tr("Quantité d'argent"), QList<QVariant>() << 0x3B);
		comboBox->addItem(tr("Ajouter objet"), QList<QVariant>() << 0x58);
		comboBox->addItem(tr("Retirer objet"), QList<QVariant>() << 0x59);
		comboBox->addItem(tr("Quantité d'objets"), QList<QVariant>() << 0x5A);
		comboBox->addItem(tr("Ajouter Matéria"), QList<QVariant>() << 0x5B);
		comboBox->addItem(tr("Retirer Matéria"), QList<QVariant>() << 0x5C);
		comboBox->addItem(tr("Quantité de Matérias"), QList<QVariant>() << 0x5D);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Nouvelle équipe"), QList<QVariant>() << 0xCA);
		comboBox->addItem(tr("Ajouter un personnage"), QList<QVariant>() << 0xC8);
		comboBox->addItem(tr("Retirer un personnage"), QList<QVariant>() << 0xC9);
		comboBox->addItem(tr("Sauvegarder l'équipe"), QList<QVariant>() << 0x0A);
		comboBox->addItem(tr("Restaurer l'équipe"), QList<QVariant>() << 0x0B);
		comboBox->addItem(tr("Créer/effacer personnage"), QList<QVariant>() << 0xCD);
		comboBox->addItem(tr("Bloquer personnage"), QList<QVariant>() << 0xCE);
		comboBox->addItem(tr("Débloquer personnage"), QList<QVariant>() << 0xCF);
		return;
	case 4:
		comboBox->addItem(tr("Définir objet 3D"), QList<QVariant>() << 0xA1);
		comboBox->addItem(tr("Affecter personnage à objet 3D"), QList<QVariant>() << 0xA0);
		comboBox->addItem(tr("Prendre le contrôle d'un groupe"), QList<QVariant>() << 0xBF);
		comboBox->insertSeparator(comboBox->count());//Get
		comboBox->addItem(tr("Obtenir la direction d'un groupe (Dir)"), QList<QVariant>() << 0xB7);
		comboBox->addItem(tr("Obtenir la direction d'un équipier (Dir)"), QList<QVariant>() << 0x73);
		comboBox->addItem(tr("Obtenir le triangle id d'un groupe (I)"), QList<QVariant>() << 0xB9);
		comboBox->addItem(tr("Obtenir le triangle id d'un équipier (I)"), QList<QVariant>() << 0x74);
		comboBox->addItem(tr("Obtenir les coordonnées d'un groupe (X,Y)"), QList<QVariant>() << 0xB8);
		comboBox->addItem(tr("Obtenir les coordonnées d'un groupe (X,Y,Z,I)"), QList<QVariant>() << 0xC1);
		comboBox->addItem(tr("Obtenir les coordonnées d'un équipier (X,Y,Z,I)"), QList<QVariant>() << 0x75);
		comboBox->insertSeparator(comboBox->count());//placer
		comboBox->addItem(tr("Placer (X,Y,Z)"), QList<QVariant>() << 0xA7);
		comboBox->addItem(tr("Placer (X,Y,I)"), QList<QVariant>() << 0xA6);
		comboBox->addItem(tr("Placer (X,Y,Z,I)"), QList<QVariant>() << 0xA5);
		comboBox->addItem(tr("Placer (Dir)"), QList<QVariant>() << 0xB3);
		comboBox->insertSeparator(comboBox->count());//déplacer
		comboBox->addItem(tr("Déplacer"), QList<QVariant>() << 0xA8);
		comboBox->addItem(tr("Déplacer sans animation"), QList<QVariant>() << 0xAD);
		comboBox->addItem(tr("Déplacer sans animation ni rotation"), QList<QVariant>() << 0xA9);
		comboBox->addItem(tr("Déplacer vers un groupe"), QList<QVariant>() << 0xAA);
		comboBox->addItem(tr("Déplacer vers un équipier"), QList<QVariant>() << 0x2A);
		comboBox->insertSeparator(comboBox->count());//rotation
		comboBox->addItem(tr("Rotation"), QList<QVariant>() << 0xB4);
		comboBox->addItem(tr("Rotation vers un groupe"), QList<QVariant>() << 0xB6);
		comboBox->addItem(tr("Rotation vers un équipier"), QList<QVariant>() << 0x34);
		comboBox->addItem(tr("Rotation inversée"), QList<QVariant>() << 0xB5);
		comboBox->addItem(tr("Rotation inversée vers un groupe"), QList<QVariant>() << 0xAB);
		comboBox->addItem(tr("Rotation inversée vers un équipier"), QList<QVariant>() << 0x35);
		comboBox->addItem(tr("Attendre fin rotation"), QList<QVariant>() << 0xDE);
		comboBox->insertSeparator(comboBox->count());//Animation
		comboBox->addItem(tr("Jouer animation en boucle"), QList<QVariant>() << 0xA2);
		comboBox->addItem(tr("Jouer animation (1)"), QList<QVariant>() << 0xAF);
		comboBox->addItem(tr("Jouer animation (2)"), QList<QVariant>() << 0xBA);
		comboBox->addItem(tr("Jouer animation et revenir (1)"), QList<QVariant>() << 0xA3);
		comboBox->addItem(tr("Jouer animation et revenir (2)"), QList<QVariant>() << 0xAE);
		comboBox->addItem(tr("Jouer partiellement animation (1)"), QList<QVariant>() << 0xB1);
		comboBox->addItem(tr("Jouer partiellement animation (2)"), QList<QVariant>() << 0xBC);
		comboBox->addItem(tr("Jouer partiellement animation et revenir (1)"), QList<QVariant>() << 0xB0);
		comboBox->addItem(tr("Jouer partiellement animation et revenir (2)"), QList<QVariant>() << 0xBB);
		comboBox->addItem(tr("Jouer animation Stand/Walk/Run"), QList<QVariant>() << 0xDC);
		comboBox->addItem(tr("Jouer animation du saut"), QList<QVariant>() << 0xC0);
		comboBox->addItem(tr("Jouer animation de l'échelle"), QList<QVariant>() << 0xC2);
		comboBox->addItem(tr("Rejoindre le leader"), QList<QVariant>() << 0x08);
		comboBox->addItem(tr("Séparer les membres de l'équipe"), QList<QVariant>() << 0x09);
		comboBox->addItem(tr("Arrêter animation"), QList<QVariant>() << 0xDD);
		comboBox->addItem(tr("Attendre fin animation"), QList<QVariant>() << 0xAC);
		comboBox->insertSeparator(comboBox->count());//Filtre graphique KAWAI
		comboBox->addItem(tr("Filtre graphique - OEil ouvert/fermé"), QList<QVariant>() << 0x0028);
		comboBox->addItem(tr("Filtre graphique - Activer/Désactiver transparence"), QList<QVariant>() << 0x0128);
		comboBox->addItem(tr("Filtre graphique - AMBNT"), QList<QVariant>() << 0x0228);
		comboBox->addItem(tr("Filtre graphique - ??? (1)"), QList<QVariant>() << 0x0328);
		comboBox->addItem(tr("Filtre graphique - ??? (2)"), QList<QVariant>() << 0x0428);
		comboBox->addItem(tr("Filtre graphique - ??? (3)"), QList<QVariant>() << 0x0528);
		comboBox->addItem(tr("Filtre graphique - LIGHT"), QList<QVariant>() << 0x0628);
		comboBox->addItem(tr("Filtre graphique - ??? (4)"), QList<QVariant>() << 0x0728);
		comboBox->addItem(tr("Filtre graphique - ??? (5)"), QList<QVariant>() << 0x0828);
		comboBox->addItem(tr("Filtre graphique - ??? (6)"), QList<QVariant>() << 0x0928);
		comboBox->addItem(tr("Filtre graphique - SBOBJ"), QList<QVariant>() << 0x0A28);
		comboBox->addItem(tr("Filtre graphique - ??? (7)"), QList<QVariant>() << 0x0B28);
		comboBox->addItem(tr("Filtre graphique - ??? (8)"), QList<QVariant>() << 0x0C28);
		comboBox->addItem(tr("Filtre graphique - SHINE"), QList<QVariant>() << 0x0D28);
		comboBox->addItem(tr("Filtre graphique - RESET"), QList<QVariant>() << 0xFF28);
		comboBox->addItem(tr("Attendre fin filtre graphique"), QList<QVariant>() << 0x29);
		comboBox->insertSeparator(comboBox->count());//offset Object
		comboBox->addItem(tr("OFST"), QList<QVariant>() << 0xC3);
		comboBox->addItem(tr("OFSTW"), QList<QVariant>() << 0xC4);
		comboBox->insertSeparator(comboBox->count());//paramètres
		comboBox->addItem(tr("Vitesse déplacements"), QList<QVariant>() << 0xB2);
		comboBox->addItem(tr("Vitesse animations"),QList<QVariant>() <<  0xBD);
		comboBox->addItem(tr("Cacher/Afficher objet 3D"), QList<QVariant>() << 0xA4);
		comboBox->addItem(tr("Activer/Désactiver rotation"), QList<QVariant>() << 0xDB);
		comboBox->addItem(tr("Cligner des yeux"), QList<QVariant>() << 0x26);
		comboBox->addItem(tr("Activer/désactiver parler"), QList<QVariant>() << 0x7E);
		comboBox->addItem(tr("Activer/désactiver toucher"), QList<QVariant>() << 0xC7);
		comboBox->addItem(tr("Distance pour parler (8 bits)"), QList<QVariant>() << 0xC5);
		comboBox->addItem(tr("Distance pour toucher (8 bits)"), QList<QVariant>() << 0xC6);
		comboBox->addItem(tr("Distance pour parler (16 bits)"), QList<QVariant>() << 0xD6);
		comboBox->addItem(tr("Distance pour toucher (16 bits)"), QList<QVariant>() << 0xD7);
		return;
	case 5:
		comboBox->addItem(tr("Créer zone"), QList<QVariant>() << 0xD0);
		comboBox->addItem(tr("Modifier zone"), QList<QVariant>() << 0xD3);
		comboBox->addItem(tr("Activer/Désactiver zone"), QList<QVariant>() << 0xD1);
		comboBox->addItem(tr("Activer/Désactiver triangle"), QList<QVariant>() << 0x6D);
		comboBox->insertSeparator(comboBox->count());//???
		comboBox->addItem(tr("SLIP"), QList<QVariant>() << 0x2B);
		return;
	case 6:
		comboBox->addItem(tr("Déplacer une couche du décor (Z)"), QList<QVariant>() << 0x2C);
		comboBox->addItem(tr("Animer une couche du décor"), QList<QVariant>() << 0x2D);
		comboBox->addItem(tr("Afficher un état d'un paramètre"), QList<QVariant>() << 0xE0);
		comboBox->addItem(tr("Cacher un état d'un paramètre"), QList<QVariant>() << 0xE1);
		comboBox->addItem(tr("Afficher l'état suivant d'un paramètre"), QList<QVariant>() << 0xE2);
		comboBox->addItem(tr("Afficher l'état précédent d'un paramètre"), QList<QVariant>() << 0xE3);
		comboBox->addItem(tr("Cacher un paramètre"), QList<QVariant>() << 0xE4);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Store Palette"), QList<QVariant>() << 0xE5);
		comboBox->addItem(tr("Store Palette (S)"), QList<QVariant>() << 0xEB);
		comboBox->addItem(tr("Load Palette"), QList<QVariant>() << 0xE6);
		comboBox->addItem(tr("Load Palette (S)"), QList<QVariant>() << 0xEC);
		comboBox->addItem(tr("Copy Palette"), QList<QVariant>() << 0xE7);
		comboBox->addItem(tr("Copy Palette (2)"), QList<QVariant>() << 0xED);
		comboBox->addItem(tr("Partial Copy Palette"), QList<QVariant>() << 0xE8);
		comboBox->addItem(tr("Partial Copy Palette (2)"), QList<QVariant>() << 0xEE);
		comboBox->addItem(tr("Multiplier Palette"), QList<QVariant>() << 0xDF);
		comboBox->addItem(tr("Multiplier Palette (2)"), QList<QVariant>() << 0xEA);
		comboBox->addItem(tr("Additionner Palette"), QList<QVariant>() << 0xE9);
		comboBox->addItem(tr("Additionner Palette (2)"), QList<QVariant>() << 0xEF);
		return;
	case 7:
		comboBox->addItem(tr("Centrer sur le personnage jouable"), QList<QVariant>() << 0x65);
		comboBox->addItem(tr("Centrer sur personnage"), QList<QVariant>() << 0x6F);
		comboBox->addItem(tr("Centrer sur groupe"), QList<QVariant>() << 0x63);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Centrer (X,Y)"), QList<QVariant>() << 0x64);
		comboBox->addItem(tr("Centrer (X,Y,Vitesse amortie)"), QList<QVariant>() << 0x66);
		comboBox->addItem(tr("Centrer (X,Y,Vitesse linéaire)"), QList<QVariant>() << 0x68);
		comboBox->addItem(tr("SCRLO"), QList<QVariant>() << 0x61);
		comboBox->addItem(tr("SCRLC"), QList<QVariant>() << 0x62);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Secouer l'écran"), QList<QVariant>() << 0x5E);
		comboBox->addItem(tr("Attendre fin centrage"), QList<QVariant>() << 0x67);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("VWOFT"), QList<QVariant>() << 0x6A);
		comboBox->addItem(tr("FADE"), QList<QVariant>() << 0x6B);
		comboBox->addItem(tr("FADEW"), QList<QVariant>() << 0x6C);
		comboBox->addItem(tr("NFADE"), QList<QVariant>() << 0x25);
		return;
	case 8:
		comboBox->addItem(tr("Jouer son"), QList<QVariant>() << 0xF1);
		comboBox->addItem(tr("AKAO"), QList<QVariant>() << 0xF2);
		comboBox->addItem(tr("AKAO2"), QList<QVariant>() << 0xDA);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Jouer musique"), QList<QVariant>() << 0xF0);
		comboBox->addItem(tr("MUSVT"), QList<QVariant>() << 0xF3);
		comboBox->addItem(tr("MUSVM"), QList<QVariant>() << 0xF4);
		comboBox->addItem(tr("Activer/Désactiver musique"), QList<QVariant>() << 0xF5);
		comboBox->addItem(tr("Musique de combat"), QList<QVariant>() << 0xF6);
		comboBox->addItem(tr("CHMPH"), QList<QVariant>() << 0xF7);
		comboBox->addItem(tr("CHMST"), QList<QVariant>() << 0xFE);
		comboBox->addItem(tr("FMUSC"), QList<QVariant>() << 0xFC);
		comboBox->addItem(tr("CMUSC"), QList<QVariant>() << 0xFD);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Prochaine cinématique"), QList<QVariant>() << 0xF8);
		comboBox->addItem(tr("Jouer cinématique"), QList<QVariant>() << 0xF9);
		comboBox->addItem(tr("MVIEF"), QList<QVariant>() << 0xFA);
		comboBox->addItem(tr("MVCAM"), QList<QVariant>() << 0xFB);
		comboBox->addItem(tr("BGMOVIE"), QList<QVariant>() << 0x27);
		return;
	case 9:
		comboBox->addItem(tr("Lancer un tutoriel"), QList<QVariant>() << 0x21);
		comboBox->addItem(tr("Afficher menu"), QList<QVariant>() << 0x49);
		comboBox->addItem(tr("Changer de disque"), QList<QVariant>() << 0x0E);
		comboBox->addItem(tr("Mini-jeu"), QList<QVariant>() << 0x20);
		comboBox->addItem(tr("Résultat du dernier combat"), QList<QVariant>() << 0x23);
		comboBox->addItem(tr("Table de combat"), QList<QVariant>() << 0x4B);
		comboBox->addItem(tr("Changer d'écran"), QList<QVariant>() << 0x60);
		comboBox->addItem(tr("ID de l'écran précédent"), QList<QVariant>() << 0x6E);
		comboBox->addItem(tr("Combattre"), QList<QVariant>() << 0x70);
		comboBox->addItem(tr("Activer/désactiver les combats"), QList<QVariant>() << 0x71);
		comboBox->addItem(tr("Mode de combat"), QList<QVariant>() << 0x72);
		comboBox->addItem(tr("Mode de combat (2)"), QList<QVariant>() << 0x22);
		comboBox->addItem(tr("Activer/désactiver les changements d'écran"), QList<QVariant>() << 0xD2);
		comboBox->addItem(tr("Activer/Désactiver déplacements"), QList<QVariant>() << 0x33);
		comboBox->addItem(tr("Précharger un écran"), QList<QVariant>() << 0xD8);
		comboBox->addItem(tr("PMJMP2"), QList<QVariant>() << 0xD9);
		comboBox->addItem(tr("Game Over"), QList<QVariant>() << 0xFF);
		comboBox->addItem(tr("SPECIAL - Activer/Désactiver curseur"), QList<QVariant>() << 0xF50F);
		comboBox->addItem(tr("SPECIAL - PNAME"), QList<QVariant>() << 0xF60F);
		comboBox->addItem(tr("SPECIAL - Vitesse du jeu"), QList<QVariant>() << 0xF70F);
		comboBox->addItem(tr("SPECIAL - Vitesse des message"), QList<QVariant>() << 0xF80F);
		comboBox->addItem(tr("SPECIAL - Full Materia"), QList<QVariant>() << 0xF90F);
		comboBox->addItem(tr("SPECIAL - Full Item"), QList<QVariant>() << 0xFA0F);
		comboBox->addItem(tr("SPECIAL - Activer/Désactiver combats"), QList<QVariant>() << 0xFB0F);
		comboBox->addItem(tr("SPECIAL - Activer/Désactiver mouvements"), QList<QVariant>() << 0xFC0F);
		comboBox->addItem(tr("SPECIAL - Renommer personnage"), QList<QVariant>() << 0xFD0F);
		comboBox->addItem(tr("SPECIAL - Clear Game"), QList<QVariant>() << 0xFE0F);
		comboBox->addItem(tr("SPECIAL - Clear Items"), QList<QVariant>() << 0xFF0F);
		return;
	case 10:
		comboBox->addItem(tr("MPDSP"), QList<QVariant>() << 0x69);
		comboBox->addItem(tr("SETX"), QList<QVariant>() << 0x9D);
		comboBox->addItem(tr("GETX"), QList<QVariant>() << 0x9E);
		comboBox->addItem(tr("SEARCHX"), QList<QVariant>() << 0x9F);
		return;
	}
}
