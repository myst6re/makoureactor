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

QList<quint8> ScriptEditor::crashIfInit;

ScriptEditor::ScriptEditor(Script *script, int opcodeID, bool modify, bool isInit, QWidget *parent)
	: QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint), opcodeID(opcodeID), isInit(isInit)
{
	if(crashIfInit.isEmpty()) {
		crashIfInit << 0x08 << 0x09 << 0x0E << 0x20 << 0x21 << 0x2A << 0x35 << 0x40 << 0x48 << 0x49
					<< 0x60 << 0x70 << 0xA2 << 0xA3 << 0xA8 << 0xA9 << 0xAD << 0xAE << 0xAF << 0xB0 << 0xB1
					<< 0xB4 << 0xB5 << 0xBA << 0xBB << 0xBC << 0xC0 << 0xC2;
	}

	//Affichage
	setWindowTitle(tr("Éditeur%1").arg(isInit ? tr(" (init mode)") : ""));
	setFixedSize(500, 318);
	//	setFixedSize(500, 618);
	
	QStringList liste0;
	liste0 <<
			  tr("Structures de contrôle") <<
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
	
	editorLayout = new QStackedLayout;
	editorLayout->addWidget(editorWidget = new ScriptEditorGenericList(script, opcodeID, this));
	editorLayout->addWidget(new ScriptEditorLabelPage(script, opcodeID, this));
	editorLayout->addWidget(new ScriptEditorJumpPage(script, opcodeID, this));
	editorLayout->addWidget(new ScriptEditorIfPage(script, opcodeID, this));
	editorLayout->addWidget(new ScriptEditorIfKeyPage(script, opcodeID, this));
	editorLayout->addWidget(new ScriptEditorIfQPage(script, opcodeID, this));

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
	layout->addLayout(editorLayout);
	layout->addStretch();
	layout->addLayout(buttonLayout);
	
	//Rouage
	connect(cancel, SIGNAL(released()), SLOT(close()));
	
	//Remplissage
	this->script = script;
	this->change = false;

	if(modify)
	{
		this->opcode = Script::copyOpcode(script->getOpcode(opcodeID));

		int index, i;
		for(i=0 ; i<comboBox0->count() ; ++i)
		{
			buildList(i);
			index = opcode->id();
			if(index == 0x0F)
				index = (((OpcodeSPECIAL *)opcode)->opcode->id() << 8) | index;
			else if(index == 0x28)
				index = (((OpcodeKAWAI *)opcode)->opcode->id() << 8) | index;
			if((index = comboBox->findData(index)) != -1)
			{
				comboBox0->setCurrentIndex(i);
				comboBox->setCurrentIndex(index);
				break;
			}
		}
		if(i==comboBox0->count())
		{
			setEnabled(false);
			return;
		}
		fillView();
		
		connect(ok, SIGNAL(released()), SLOT(modify()));
	}
	else
	{
		this->opcode = new OpcodeRET();
		comboBox->setCurrentIndex(0);
		changeCurrentOpcode(0);

		connect(ok, SIGNAL(released()), SLOT(add()));
	}

	connect(comboBox0, SIGNAL(currentIndexChanged(int)), SLOT(buildList(int)));
	for(int i=0 ; i<editorLayout->count() ; ++i)
		connect((ScriptEditorView *)editorLayout->widget(i), SIGNAL(opcodeChanged()), SLOT(refreshTextEdit()));
	connect(comboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeCurrentOpcode(int)));
}

ScriptEditor::~ScriptEditor()
{
	delete opcode;
}

void ScriptEditor::fillEditor()
{
	int index;

	// Change current editor widget
	switch((Opcode::Keys)opcode->id()) {
	case Opcode::LABEL:
		index = 1;
		break;
	case Opcode::JMPF:case Opcode::JMPFL:
	case Opcode::JMPB:case Opcode::JMPBL:
		index = 2;
		break;
	case Opcode::IFUB:case Opcode::IFUBL:
	case Opcode::IFSW:case Opcode::IFSWL:
	case Opcode::IFUW:case Opcode::IFUWL:
		index = 3;
		break;
	case Opcode::IFKEY:case Opcode::IFKEYON:
	case Opcode::IFKEYOFF:
		index = 4;
		break;
	case Opcode::IFMEMBQ:case Opcode::IFPRTYQ:
		index = 5;
		break;
	default:
		index = 0;
	}

	editorWidget = (ScriptEditorView *)editorLayout->widget(index);
	bool hasParams = opcode->hasParams() || opcode->isLabel();
	editorWidget->setVisible(hasParams);
	editorWidget->setOpcode(opcode);
	editorLayout->setCurrentWidget(editorWidget);
}

void ScriptEditor::fillView()
{
	textEdit->setPlainText(opcode->toString()); // text part
	fillEditor(); // editor part

	// disable if necessary
	bool disableEditor = (isInit && crashIfInit.contains(opcode->id())) || !editorWidget->isValid();

	textEdit->setDisabled(disableEditor);
	editorWidget->setDisabled(disableEditor);
	ok->setDisabled(disableEditor);
}

void ScriptEditor::modify()
{
	/* if(!this->change) {
		close();
		return;
	} */
	script->setOpcode(this->opcodeID, Script::copyOpcode(editorWidget->opcode()));
	accept();
}

void ScriptEditor::add()
{
	/* if(!this->change) {
		close();
		return;
	} */
	script->insertOpcode(this->opcodeID, Script::copyOpcode(editorWidget->opcode()));
	accept();
}

void ScriptEditor::refreshTextEdit()
{
	this->change = true;

	textEdit->setPlainText(editorWidget->opcode()->toString());
}

void ScriptEditor::changeCurrentOpcode(int index)
{
	this->change = true;
	QByteArray newOpcode;
	int itemData = comboBox->itemData(index).toInt();
	const quint8 id = itemData & 0xFF;
	bool isLabel = itemData == 0x100;
	newOpcode.append((char)id);
	
	if(id == 0x28)//KAWAI
	{
		newOpcode.append('\x03'); // size
		newOpcode.append((char)((itemData >> 8) & 0xFF)); // KAWAI ID
	}
	else if(id == 0x0F)//SPECIAL
	{
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
	}
	else if(!isLabel)
	{
		for(quint8 pos=1 ; pos<Opcode::length[id] ; ++pos)
			newOpcode.append('\x00');
	}

	if(isLabel) {
		if(0x100 == opcode->id()) {
			((OpcodeLabel *)opcode)->setLabel(0);
		} else {
			opcode = new OpcodeLabel(0);
		}
	} else {
		if(id == opcode->id()) {
			opcode->setParams(newOpcode.mid(1)); // same opcode, just change params
		} else { // change all
			delete opcode;
			opcode = Script::createOpcode(newOpcode);
		}
	}
	
	fillView();
}

void ScriptEditor::buildList(int id)
{
	comboBox->clear();
	switch(id)
	{
	case 0:
		comboBox->addItem(tr("Retourner"), 0x00);
		comboBox->addItem(tr("Retourner à"), 0x07);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Exécuter un script"), 0x01);
		comboBox->addItem(tr("Exécuter un script"), 0x02);
		comboBox->addItem(tr("Exécuter un script"), 0x03);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Exécuter un script d'un équipier"), 0x04);
		comboBox->addItem(tr("Exécuter un script d'un équipier"), 0x05);
		comboBox->addItem(tr("Exécuter un script d'un équipier"), 0x06);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Label"), 0x100);
		comboBox->addItem(tr("Saut court en avant"), 0x10);
		comboBox->addItem(tr("Saut long en avant"), 0x11);
		comboBox->addItem(tr("Saut court en arrière"), 0x12);
		comboBox->addItem(tr("Saut long en arrière"), 0x13);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Si...alors (8 bits, saut court)"), 0x14);
		comboBox->addItem(tr("Si...alors (8 bits, saut long)"), 0x15);
		comboBox->addItem(tr("Si...alors (16 bits-signé, saut court)"), 0x16);
		comboBox->addItem(tr("Si...alors (16 bits-signé, saut long)"), 0x17);
		comboBox->addItem(tr("Si...alors (16 bits-non signé, saut court)"), 0x18);
		comboBox->addItem(tr("Si...alors (16 bits-non signé, saut long)"), 0x19);
		comboBox->addItem(tr("Si touche pressée"), 0x30);
		comboBox->addItem(tr("Si touche pressée une fois"), 0x31);
		comboBox->addItem(tr("Si touche lachée une fois"), 0x32);
		comboBox->addItem(tr("Si personnage membre de l'équipe"), 0xCB);
		comboBox->addItem(tr("Si personnage existe"), 0xCC);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Attendre"), 0x24);
		comboBox->addItem(tr("Ne rien faire"), 0x5F);
		return;
	case 1:
		comboBox->addItem(tr("Affectation (8 bits)"), 0x80);
		comboBox->addItem(tr("Affectation (16 bits)"), 0x81);
		comboBox->addItem(tr("Addition (8 bits)"), 0x76);
		comboBox->addItem(tr("Addition (8 bits - mod 256)"), 0x85);
		comboBox->addItem(tr("Addition (16 bits)"), 0x77);
		comboBox->addItem(tr("Addition (16 bits - mod 65536)"), 0x86);
		comboBox->addItem(tr("Soustraction (8 bits)"), 0x78);
		comboBox->addItem(tr("Soustraction (8 bits - mod 256)"), 0x87);
		comboBox->addItem(tr("Soustraction (16 bits)"), 0x79);
		comboBox->addItem(tr("Soustraction (16 bits - mod 65536)"), 0x88);
		comboBox->addItem(tr("Multiplication (8 bits - mod 256)"), 0x89);
		comboBox->addItem(tr("Multiplication (16 bits - mod 65536)"), 0x8A);
		comboBox->addItem(tr("Division (8 bits - mod 256)"), 0x8B);
		comboBox->addItem(tr("Division (16 bits - mod 65536)"), 0x8C);
		comboBox->addItem(tr("Incrémentation (8 bits)"), 0x7A);
		comboBox->addItem(tr("Incrémentation (8 bits - mod 256)"), 0x95);
		comboBox->addItem(tr("Incrémentation (16 bits)"), 0x7B);
		comboBox->addItem(tr("Incrémentation (16 bits - mod 65536)"), 0x96);
		comboBox->addItem(tr("Décrémentation (8 bits)"), 0x7C);
		comboBox->addItem(tr("Décrémentation (8 bits - mod 256)"), 0x97);
		comboBox->addItem(tr("Décrémentation (16 bits)"), 0x7D);
		comboBox->addItem(tr("Décrémentation (16 bits - mod 65536)"), 0x98);
		comboBox->addItem(tr("Modulo (8 bits - mod 256)"), 0x8D);
		comboBox->addItem(tr("Modulo (16 bits - mod 65536)"), 0x8E);
		comboBox->addItem(tr("Et binaire (8 bits - mod 256)"), 0x8F);
		comboBox->addItem(tr("Et binaire (16 bits - mod 65536)"), 0x90);
		comboBox->addItem(tr("Ou binaire (8 bits - mod 256)"), 0x91);
		comboBox->addItem(tr("Ou binaire (16 bits - mod 65536)"), 0x92);
		comboBox->addItem(tr("Ou exclusif binaire (8 bits - mod 256)"), 0x93);
		comboBox->addItem(tr("Ou exclusif binaire (16 bits - mod 65536)"), 0x94);
		comboBox->addItem(tr("Activer un bit"), 0x82);
		comboBox->addItem(tr("Désactiver un bit"), 0x83);
		comboBox->addItem(tr("Inverser un bit"), 0x84);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Seed Random Generator"), 0x7F);
		comboBox->addItem(tr("Valeur aléatoire"), 0x99);
		comboBox->addItem(tr("Octet de poids faible"), 0x9A);
		comboBox->addItem(tr("Octet de poids fort"), 0x9B);
		comboBox->addItem(tr("Créer un mot de deux octets"), 0x9C);
		comboBox->addItem(tr("Sinus"), 0xD4);
		comboBox->addItem(tr("Cosinus"), 0xD5);
		return;
	case 2:
		comboBox->addItem(tr("Créer une fenêtre"), 0x50);
		comboBox->addItem(tr("Fermer une fenêtre (1)"), 0x2E);
		comboBox->addItem(tr("Fermer une fenêtre (2)"), 0x54);
		comboBox->addItem(tr("Redimensionner une fenêtre"), 0x2F);
		comboBox->addItem(tr("Modifier le nombre de lignes dans fenêtre"), 0x55);
		comboBox->addItem(tr("Déplacer une fenêtre"), 0x51);
		comboBox->addItem(tr("Modifier type de fenêtre"), 0x52);
		comboBox->addItem(tr("Remettre une fenêtre à zéro"), 0x53);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Mettre un compteur dans fenêtre"), 0x36);
		comboBox->addItem(tr("Modifier valeur du compteur dans fenêtre"), 0x37);
		comboBox->addItem(tr("Compte à rebours"), 0x38);
		comboBox->addItem(tr("Modifier variable dans fenêtre (8 bits)"), 0x41);
		comboBox->addItem(tr("Modifier variable dans fenêtre (16 bits)"), 0x42);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Afficher texte"), 0x40);
		comboBox->addItem(tr("Poser une question"), 0x48);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Modifier le nom du lieu"), 0x43);
		comboBox->addItem(tr("Activer/Désactiver menu"), 0x4A);
		comboBox->addItem(tr("GWCOL"), 0x56);
		comboBox->addItem(tr("SWCOL"), 0x57);
		return;
	case 3:
		comboBox->addItem(tr("HP/MP restaurés (1)"), 0x3C);
		comboBox->addItem(tr("HP/MP restaurés (2)"), 0x3D);
		comboBox->addItem(tr("HP/MP/statut restaurés"), 0x3E);
		comboBox->addItem(tr("HP/MP restaurés (3)"), 0x3F);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Augmenter les MPs"), 0x45);
		comboBox->addItem(tr("Diminuer les MPs"), 0x47);
		comboBox->addItem(tr("Augmenter les HPs"), 0x4D);
		comboBox->addItem(tr("Diminuer les HPs"), 0x4F);
		comboBox->addItem(tr("Ajouter des gils"), 0x39);
		comboBox->addItem(tr("Retirer des gils"), 0x3A);
		comboBox->addItem(tr("Quantité d'argent"), 0x3B);
		comboBox->addItem(tr("Ajouter objet"), 0x58);
		comboBox->addItem(tr("Retirer objet"), 0x59);
		comboBox->addItem(tr("Quantité d'objets"), 0x5A);
		comboBox->addItem(tr("Ajouter Matéria"), 0x5B);
		comboBox->addItem(tr("Retirer Matéria"), 0x5C);
		comboBox->addItem(tr("Quantité de Matérias"), 0x5D);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Nouvelle équipe"), 0xCA);
		comboBox->addItem(tr("Ajouter un personnage"), 0xC8);
		comboBox->addItem(tr("Retirer un personnage"), 0xC9);
		comboBox->addItem(tr("Sauvegarder l'équipe"), 0x0A);
		comboBox->addItem(tr("Restaurer l'équipe"), 0x0B);
		comboBox->addItem(tr("Créer/effacer personnage"), 0xCD);
		comboBox->addItem(tr("Bloquer personnage"), 0xCE);
		comboBox->addItem(tr("Débloquer personnage"), 0xCF);
		return;
	case 4:
		comboBox->addItem(tr("Définir objet 3D"), 0xA1);
		comboBox->addItem(tr("Affecter personnage à objet 3D"), 0xA0);
		comboBox->addItem(tr("Prendre le contrôle d'un groupe"), 0xBF);
		comboBox->insertSeparator(comboBox->count());//Get
		comboBox->addItem(tr("Obtenir la direction d'un groupe (Dir)"), 0xB7);
		comboBox->addItem(tr("Obtenir la direction d'un équipier (Dir)"), 0x73);
		comboBox->addItem(tr("Obtenir le polygone id d'un groupe (I)"), 0xB9);
		comboBox->addItem(tr("Obtenir le polygone id d'un équipier (I)"), 0x74);
		comboBox->addItem(tr("Obtenir les coordonnées d'un groupe (X,Y)"), 0xB8);
		comboBox->addItem(tr("Obtenir les coordonnées d'un groupe (X,Y,Z,I)"), 0xC1);
		comboBox->addItem(tr("Obtenir les coordonnées d'un équipier (X,Y,Z,I)"), 0x75);
		comboBox->insertSeparator(comboBox->count());//placer
		comboBox->addItem(tr("Placer (X,Y,Z)"), 0xA7);
		comboBox->addItem(tr("Placer (X,Y,I)"), 0xA6);
		comboBox->addItem(tr("Placer (X,Y,Z,I)"), 0xA5);
		comboBox->addItem(tr("Placer (Dir)"), 0xB3);
		comboBox->insertSeparator(comboBox->count());//déplacer
		comboBox->addItem(tr("Déplacer"), 0xA8);
		comboBox->addItem(tr("Déplacer sans animation"), 0xAD);
		comboBox->addItem(tr("Déplacer sans animation ni rotation"), 0xA9);
		comboBox->addItem(tr("Déplacer vers un groupe"), 0xAA);
		comboBox->addItem(tr("Déplacer vers un équipier"), 0x2A);
		comboBox->insertSeparator(comboBox->count());//rotation
		comboBox->addItem(tr("Rotation"), 0xB4);
		comboBox->addItem(tr("Rotation vers un groupe"), 0xB6);
		comboBox->addItem(tr("Rotation vers un équipier"), 0x34);
		comboBox->addItem(tr("Rotation inversée"), 0xB5);
		comboBox->addItem(tr("Rotation inversée vers un groupe"), 0xAB);
		comboBox->addItem(tr("Rotation inversée vers un équipier"), 0x35);
		comboBox->addItem(tr("Attendre fin rotation"), 0xDE);
		comboBox->insertSeparator(comboBox->count());//Animation
		comboBox->addItem(tr("Jouer animation en boucle"), 0xA2);
		comboBox->addItem(tr("Jouer animation (1)"), 0xAF);
		comboBox->addItem(tr("Jouer animation (2)"), 0xBA);
		comboBox->addItem(tr("Jouer animation et revenir (1)"), 0xA3);
		comboBox->addItem(tr("Jouer animation et revenir (2)"), 0xAE);
		comboBox->addItem(tr("Jouer partiellement animation (1)"), 0xB1);
		comboBox->addItem(tr("Jouer partiellement animation (2)"), 0xBC);
		comboBox->addItem(tr("Jouer partiellement animation et revenir (1)"), 0xB0);
		comboBox->addItem(tr("Jouer partiellement animation et revenir (2)"), 0xBB);
		comboBox->addItem(tr("Jouer animation Stand/Walk/Run"), 0xDC);
		comboBox->addItem(tr("Jouer animation du saut"), 0xC0);
		comboBox->addItem(tr("Jouer animation de l'échelle"), 0xC2);
		comboBox->addItem(tr("Rejoindre le leader"), 0x08);
		comboBox->addItem(tr("Séparer les membres de l'équipe"), 0x09);
		comboBox->addItem(tr("Arrêter animation"), 0xDD);
		comboBox->addItem(tr("Attendre fin animation"), 0xAC);
		comboBox->insertSeparator(comboBox->count());//Filtre graphique KAWAI
		comboBox->addItem(tr("Filtre graphique - OEil ouvert/fermé"), 0x0028);
		comboBox->addItem(tr("Filtre graphique - Activer/Désactiver transparence"), 0x0128);
		comboBox->addItem(tr("Filtre graphique - AMBNT"), 0x0228);
		comboBox->addItem(tr("Filtre graphique - ??? (1)"), 0x0328);
		comboBox->addItem(tr("Filtre graphique - ??? (2)"), 0x0428);
		comboBox->addItem(tr("Filtre graphique - ??? (3)"), 0x0528);
		comboBox->addItem(tr("Filtre graphique - LIGHT"), 0x0628);
		comboBox->addItem(tr("Filtre graphique - ??? (4)"), 0x0728);
		comboBox->addItem(tr("Filtre graphique - ??? (5)"), 0x0828);
		comboBox->addItem(tr("Filtre graphique - ??? (6)"), 0x0928);
		comboBox->addItem(tr("Filtre graphique - SBOBJ"), 0x0A28);
		comboBox->addItem(tr("Filtre graphique - ??? (7)"), 0x0B28);
		comboBox->addItem(tr("Filtre graphique - ??? (8)"), 0x0C28);
		comboBox->addItem(tr("Filtre graphique - SHINE"), 0x0D28);
		comboBox->addItem(tr("Filtre graphique - RESET"), 0xFF28);
		comboBox->addItem(tr("Attendre fin filtre graphique"), 0x29);
		comboBox->insertSeparator(comboBox->count());//offset Object
		comboBox->addItem(tr("OFST"), 0xC3);
		comboBox->addItem(tr("OFSTW"), 0xC4);
		comboBox->insertSeparator(comboBox->count());//paramètres
		comboBox->addItem(tr("Vitesse déplacements"), 0xB2);
		comboBox->addItem(tr("Vitesse animations"), 0xBD);
		comboBox->addItem(tr("Cacher/Afficher objet 3D"), 0xA4);
		comboBox->addItem(tr("Activer/Désactiver rotation"), 0xDB);
		comboBox->addItem(tr("Cligner des yeux"), 0x26);
		comboBox->addItem(tr("Activer/désactiver parler"), 0x7E);
		comboBox->addItem(tr("Activer/désactiver toucher"), 0xC7);
		comboBox->addItem(tr("Distance pour parler (8 bits)"), 0xC5);
		comboBox->addItem(tr("Distance pour toucher (8 bits)"), 0xC6);
		comboBox->addItem(tr("Distance pour parler (16 bits)"), 0xD6);
		comboBox->addItem(tr("Distance pour toucher (16 bits)"), 0xD7);
		comboBox->insertSeparator(comboBox->count());//???
		comboBox->addItem(tr("SLIP"), 0x2B);
		return;
	case 5:
		comboBox->addItem(tr("Créer zone"), 0xD0);
		comboBox->addItem(tr("Modifier zone"), 0xD3);
		comboBox->addItem(tr("Activer/Désactiver zone"), 0xD1);
		comboBox->addItem(tr("Activer/Désactiver polygone"), 0x6D);
		return;
	case 6:
		comboBox->addItem(tr("Déplacer une couche du décor (Z)"), 0x2C);
		comboBox->addItem(tr("Animer une couche du décor"), 0x2D);
		comboBox->addItem(tr("Afficher un état d'un paramètre"), 0xE0);
		comboBox->addItem(tr("Cacher un état d'un paramètre"), 0xE1);
		comboBox->addItem(tr("Afficher l'état suivant d'un paramètre"), 0xE2);
		comboBox->addItem(tr("Afficher l'état précédent d'un paramètre"), 0xE3);
		comboBox->addItem(tr("Cacher un paramètre"), 0xE4);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("MP Palette"), 0xDF);
		comboBox->addItem(tr("MP Palette (2)"), 0xEA);
		comboBox->addItem(tr("ST (Store) Palette"), 0xE5);
		comboBox->addItem(tr("ST (Store) PLS"), 0xEB);
		comboBox->addItem(tr("LD (Load) Palette"), 0xE6);
		comboBox->addItem(tr("LD (Load) PLS"), 0xEC);
		comboBox->addItem(tr("CP (Copy) Palette"), 0xE7);
		comboBox->addItem(tr("CP (Copy) Palette (2)"), 0xED);
		comboBox->addItem(tr("RT Palette"), 0xE8);
		comboBox->addItem(tr("RT Palette (2)"), 0xEE);
		comboBox->addItem(tr("AD Palette"), 0xE9);
		comboBox->addItem(tr("AD Palette (2)"), 0xEF);
		return;
	case 7:
		comboBox->addItem(tr("Centrer sur le personnage jouable"), 0x65);
		comboBox->addItem(tr("Centrer sur personnage"), 0x6F);
		comboBox->addItem(tr("Centrer sur groupe"), 0x63);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Centrer (X,Y)"), 0x64);
		comboBox->addItem(tr("Centrer (X,Y,Vitesse amortie)"), 0x66);
		comboBox->addItem(tr("Centrer (X,Y,Vitesse linéaire)"), 0x68);
		comboBox->addItem(tr("SCRLO"), 0x61);
		comboBox->addItem(tr("SCRLC"), 0x62);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Secouer l'écran"), 0x5E);
		comboBox->addItem(tr("Attendre fin centrage"), 0x67);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("VWOFT"), 0x6A);
		comboBox->addItem(tr("FADE"), 0x6B);
		comboBox->addItem(tr("FADEW"), 0x6C);
		comboBox->addItem(tr("NFADE"), 0x25);
		return;
	case 8:
		comboBox->addItem(tr("Jouer son"), 0xF1);
		comboBox->addItem(tr("AKAO"), 0xF2);
		comboBox->addItem(tr("AKAO2"), 0xDA);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Jouer musique"), 0xF0);
		comboBox->addItem(tr("MUSVT"), 0xF3);
		comboBox->addItem(tr("MUSVM"), 0xF4);
		comboBox->addItem(tr("Activer/Désactiver musique"), 0xF5);
		comboBox->addItem(tr("Musique de combat"), 0xF6);
		comboBox->addItem(tr("CHMPH"), 0xF7);
		comboBox->addItem(tr("CHMST"), 0xFE);
		comboBox->addItem(tr("FMUSC"), 0xFC);
		comboBox->addItem(tr("CMUSC"), 0xFD);
		comboBox->insertSeparator(comboBox->count());
		comboBox->addItem(tr("Prochaine cinématique"), 0xF8);
		comboBox->addItem(tr("Jouer cinématique"), 0xF9);
		comboBox->addItem(tr("MVIEF"), 0xFA);
		comboBox->addItem(tr("MVCAM"), 0xFB);
		comboBox->addItem(tr("BGMOVIE"), 0x27);
		return;
	case 9:
		comboBox->addItem(tr("Lancer un tutoriel"), 0x21);
		comboBox->addItem(tr("Afficher menu"), 0x49);
		comboBox->addItem(tr("Changer de disque"), 0x0E);
		comboBox->addItem(tr("Mini-jeu"), 0x20);
		comboBox->addItem(tr("Mode de combat"), 0x22);
		comboBox->addItem(tr("Résultat du dernier combat"), 0x23);
		comboBox->addItem(tr("Table de combat"), 0x4B);
		comboBox->addItem(tr("Changer d'écran"), 0x60);
		comboBox->addItem(tr("ID de l'écran précédent"), 0x6E);
		comboBox->addItem(tr("Combattre"), 0x70);
		comboBox->addItem(tr("Activer/désactiver les combats"), 0x71);
		comboBox->addItem(tr("BTLMD"), 0x72);
		comboBox->addItem(tr("Activer/désactiver les changements d'écran"), 0xD2);
		comboBox->addItem(tr("Activer/Désactiver déplacements"), 0x33);
		comboBox->addItem(tr("Précharger un écran"), 0xD8);
		comboBox->addItem(tr("PMJMP2"), 0xD9);
		comboBox->addItem(tr("Game Over"), 0xFF);
		comboBox->addItem(tr("SPECIAL - Activer/Désactiver curseur"), 0xF50F);
		comboBox->addItem(tr("SPECIAL - PNAME"), 0xF60F);
		comboBox->addItem(tr("SPECIAL - Vitesse du jeu"), 0xF70F);
		comboBox->addItem(tr("SPECIAL - Vitesse des message"), 0xF80F);
		comboBox->addItem(tr("SPECIAL - Full Materia"), 0xF90F);
		comboBox->addItem(tr("SPECIAL - Full Item"), 0xFA0F);
		comboBox->addItem(tr("SPECIAL - Activer/Désactiver combats"), 0xFB0F);
		comboBox->addItem(tr("SPECIAL - Activer/Désactiver mouvements"), 0xFC0F);
		comboBox->addItem(tr("SPECIAL - Renommer personnage"), 0xFD0F);
		comboBox->addItem(tr("SPECIAL - Clear Game"), 0xFE0F);
		comboBox->addItem(tr("SPECIAL - Clear Items"), 0xFF0F);
		return;
	case 10:
		comboBox->addItem(tr("MPDSP"), 0x69);
		comboBox->addItem(tr("SETX"), 0x9D);
		comboBox->addItem(tr("GETX"), 0x9E);
		comboBox->addItem(tr("SEARCHX"), 0x9F);
		return;
	}
}
