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

ScriptEditor::ScriptEditor(Script *script, int commandeID, bool modify, bool isInit, QWidget *parent)
	: QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint), isInit(isInit)
{
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
	
	model = new QStandardItemModel(this);
	tableView = new QTableView(this);
	tableView->setModel(model);
	tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	tableView->setSelectionMode(QAbstractItemView::SingleSelection);
	tableView->horizontalHeader()->setVisible(false);
	tableView->setEditTriggers(QAbstractItemView::CurrentChanged | QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
	tableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	
	SpinBoxDelegate *delegate = new SpinBoxDelegate(this);
	tableView->setItemDelegate(delegate);
	tableView->horizontalHeader()->setStretchLastSection(true);
	
/*	nouvelEditeur = new QStackedWidget;
	nouvelEditeur->setFixedHeight(300);
    nouvelEditeur->addWidget(new GotoPage);
    nouvelEditeur->addWidget(new ReturnPage);
    nouvelEditeur->addWidget(new ExecPage);
    nouvelEditeur->addWidget(new ExecCharPage);
    nouvelEditeur->addWidget(new IfPage);
    nouvelEditeur->addWidget(new IfKeyPage);
    nouvelEditeur->addWidget(new IfCharPage);
    nouvelEditeur->addWidget(new WaitPage);
    nouvelEditeur->addWidget(new NopPage);
    nouvelEditeur->addWidget(new OpPage);
    nouvelEditeur->addWidget(new Op1Page);
	nouvelEditeur->addWidget(new BgParamStatePage);
*/	
	addButton = new QPushButton(tr("Ajouter une ligne"),this);
	addButton->hide();
	delButton = new QPushButton(tr("Effacer une ligne"),this);
	delButton->hide();
	ok = new QPushButton(tr("OK"),this);
	ok->setDefault(true);
	cancel = new QPushButton(tr("Annuler"),this);
	
	buttonLayout = new QHBoxLayout();
	buttonLayout->addWidget(addButton);
	buttonLayout->addWidget(delButton);
	buttonLayout->addStretch();
	buttonLayout->addWidget(ok);
	buttonLayout->addWidget(cancel);
	
	layout = new QVBoxLayout(this);
	layout->addWidget(comboBox0);
	layout->addWidget(comboBox);
	layout->addWidget(textEdit);
	layout->addWidget(new QLabel(tr("Paramètres :"), this));
	layout->addWidget(tableView);
	//layout->addWidget(nouvelEditeur);
	layout->addStretch();
	layout->addLayout(buttonLayout);
	
	//Rouage
	connect(cancel, SIGNAL(released()), SLOT(close()));
	
	//Remplissage
	this->script = script;
	this->change = false;
	
	if(modify)
	{
		this->commandeID = commandeID;
		this->commande = Script::createOpcode(script->getCommande(commandeID)->toByteArray());
		int index, i;
		for(i=0 ; i<comboBox0->count() ; ++i)
		{
			buildList(i);
			index = commande->id();
			if(index == 0x0F)
				index = (((OpcodeSPECIAL *)commande)->opcode->id() << 8) | index;
			else if(index == 0x28)
				index = (((OpcodeKAWAI *)commande)->opcode->id() << 8) | index;
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
		textEdit->setPlainText(commande->toString());
		fillModel(commande->id());
		
		connect(ok, SIGNAL(released()), SLOT(modify()));
	}
	else
	{
		this->commandeID = commandeID < 0 ? script->size() : commandeID + 1;
		this->commande = new OpcodeRET();
		comboBox->setCurrentIndex(0);
		changeCurrentOpcode(0);
		fillModel(0);

		connect(ok, SIGNAL(released()), SLOT(add()));
	}

	crashIfInit << 0x08 << 0x09 << 0x0E << 0x20 << 0x21 << 0x2A << 0x35 << 0x40 << 0x48 << 0x49
			<< 0x60 << 0x70 << 0xA2 << 0xA3 << 0xA8 << 0xA9 << 0xAD << 0xAE << 0xAF << 0xB0 << 0xB1
			<< 0xB4 << 0xB5 << 0xBA << 0xBB << 0xBC << 0xC0 << 0xC2;

	connect(comboBox0, SIGNAL(currentIndexChanged(int)), SLOT(buildList(int)));
	connect(model, SIGNAL(itemChanged(QStandardItem *)), SLOT(refreshTextEdit()));
	connect(comboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeCurrentOpcode(int)));
	connect(addButton, SIGNAL(released()), SLOT(addParam()));
	connect(delButton, SIGNAL(released()), SLOT(delLastRow()));
	/* for(int i=0 ; i<256 ; i++) {

		QList<int> paramTypes = this->paramTypes(i);
		int tailleParam = 0;
		for(int j=0 ; j<paramTypes.size() ; j++)
			tailleParam += paramSize(paramTypes.at(j));

		if(tailleParam/8 != Opcode::length[i]-1) {
			qDebug() << QString("Erreur taille (0x%1) : %2 et %3").arg(i,0,16).arg(tailleParam/8).arg(Opcode::length[i]-1);
		}
		if(tailleParam%8!=0) {
			qDebug() << QString("Pas multiple de 8 (0x%1) : %2").arg(i,0,16).arg(tailleParam);
		}
	} */
}

ScriptEditor::~ScriptEditor()
{
	delete commande;
}

QByteArray ScriptEditor::parseModel()
{
	/* quint8 opcode = comboBox->itemData(comboBox->currentIndex()).toInt();

	if(opcode>=0x10 && opcode<=0x13)
	{
		nouvelEditeur->setCurrentIndex(0);
		return ((GotoPage *)nouvelEditeur->currentWidget())->save();
	}
	else if(opcode==0x00 || opcode==0x07)
	{
		nouvelEditeur->setCurrentIndex(1);
		return ((ReturnPage *)nouvelEditeur->currentWidget())->save();
	}
	else if(opcode>=0x01 && opcode<=0x03)
	{
		nouvelEditeur->setCurrentIndex(2);
		return ((ExecPage *)nouvelEditeur->currentWidget())->save();
	}
	else if(opcode>=0x04 && opcode<=0x06)
	{
		nouvelEditeur->setCurrentIndex(3);
		return ((ExecCharPage *)nouvelEditeur->currentWidget())->save();
	}
	else if(opcode>=0x14 && opcode<=0x19)
	{
		nouvelEditeur->setCurrentIndex(4);
		return ((IfPage *)nouvelEditeur->currentWidget())->save();
	}
	else if(opcode>=0x30 && opcode<=0x32)
	{
		nouvelEditeur->setCurrentIndex(5);
		return ((IfKeyPage *)nouvelEditeur->currentWidget())->save();
	}
	else if(opcode==0xCB || opcode==0xCC)
	{
		nouvelEditeur->setCurrentIndex(6);
		return ((IfCharPage *)nouvelEditeur->currentWidget())->save();
	}
	else if(opcode==0x24)
	{
		nouvelEditeur->setCurrentIndex(7);
		return ((WaitPage *)nouvelEditeur->currentWidget())->save();
	}
	else if(opcode==0x5F)
	{
		nouvelEditeur->setCurrentIndex(8);
		return ((NopPage *)nouvelEditeur->currentWidget())->save();
	}
	else if((opcode>=0x76 && opcode<=0x79) || opcode==0x80 || opcode==0x81 || (opcode>=0x85 && opcode<=0x94))
	{
		nouvelEditeur->setCurrentIndex(9);
		return ((OpPage *)nouvelEditeur->currentWidget())->save();
	}
	else if((opcode>=0x7A && opcode<=0x7D) || (opcode>=0x95 && opcode<=0x98))
	{
		nouvelEditeur->setCurrentIndex(10);
		return ((Op1Page *)nouvelEditeur->currentWidget())->save();
	}
	else if(opcode==0xE0 || opcode==0xE1)
	{
		nouvelEditeur->setCurrentIndex(11);
		return ((BgParamStatePage *)nouvelEditeur->currentWidget())->save();
	}*/

	QByteArray nouvelleCommande;
	quint8 byte, length, start;
	int itemData;
	
	itemData = comboBox->itemData(comboBox->currentIndex()).toInt();
	byte = itemData & 0xFF;
	// qDebug() << "byte= " << byte;
	nouvelleCommande.append((char)byte);
	//Calcul longueur commande
	length = 0;
	start = 1;
	
	if(byte == 0xF)
	{
		quint8 byte2 = (itemData >> 8) & 0xFF;
		nouvelleCommande.append((char)byte2);
		switch(byte2)
		{
		case 0xF5:case 0xF6:case 0xF7:case 0xFB:case 0xFC:
					length = 1;
			break;
		case 0xF8:case 0xFD:
					length = 2;
			break;
		}
		start = 2;
	}
	else if(byte == 0x28)//KAWAI
	{
		quint8 byte3 = (itemData >> 8) & 0xFF;
		// qDebug() << "byte3= " << byte3;
		length = model->rowCount()+3;
		// qDebug() << "length = " << length;
		nouvelleCommande.append((char)length);
		nouvelleCommande.append((char)byte3);
		// qDebug() << "nouvelleCommande= " << nouvelleCommande.toHex();
		for(quint8 i=0 ; i<length-3 ; ++i)
			nouvelleCommande.append(model->item(i, 1)->text().toUInt());
		// qDebug() << "nouvelleCommande= " << nouvelleCommande.toHex();
		return nouvelleCommande;
	}
	
	// qDebug() << "byte= " << byte;
	length += Opcode::length[byte];
	// qDebug() << "length= " << length;
	nouvelleCommande.append(QByteArray(length-start, '\x0'));
	// qDebug() << "nouvelleCommande= " << nouvelleCommande.toHex();
	int paramSize, paramType, cur = 8, departBA, tailleBA, departLocal;
	QList<int> paramTypes = this->paramTypes(byte);
	int value;
	
	if(!paramTypes.isEmpty())
	{
		for(quint8 i=0 ; i<paramTypes.size() ; ++i)
		{
			// qDebug() << "i= " << i;
			paramType = paramTypes.at(i);
			// qDebug() << "paramType= " << paramName(paramType);
			value = model->data(model->index(i, 1), Qt::EditRole).toInt();
			// qDebug() << "value= " << value;
			// qDebug() << "data(Qt::DisplayRole)= " << model->data(model->index(i, 1), Qt::DisplayRole);
			// qDebug() << "data(Qt::EditRole)= " << model->data(model->index(i, 1), Qt::EditRole);
			// qDebug() << "data(Qt::UserRole)= " << model->data(model->index(i, 1), Qt::UserRole);
			// qDebug() << "data(Qt::UserRole+1)= " << model->data(model->index(i, 1), Qt::UserRole+1);
			// qDebug() << "data(Qt::UserRole+2)= " << model->data(model->index(i, 1), Qt::UserRole+2);
			
			paramSize = this->paramSize(paramType);
			// qDebug() << "paramSize= " << paramSize;
			departBA = cur/8;
			// qDebug() << "departBA= " << departBA;
			if(paramSize%8 !=0)
				tailleBA = paramSize/8+1;
			else
				tailleBA = paramSize/8;
			// qDebug() << "tailleBA= " << tailleBA;
			
			if(paramSize < 8)
			{
				departLocal = cur%8;
				// qDebug() << "departLocal= " << departLocal;
				nouvelleCommande[departBA] = (char)((quint8)nouvelleCommande.at(departBA) | (value << (8-paramSize-departLocal)));
				// qDebug() << "nouvelleCommande[" << departBA << "]=" << QString("%1").arg((quint8)nouvelleCommande[departBA],8,2,QChar('0'));
			}
			else if(paramSize == 8)
			{
				nouvelleCommande[departBA] = (char)(value & 0xFF);
				// qDebug() << "nouvelleCommande[" << departBA << "]=" << QString("%1").arg((quint8)nouvelleCommande[departBA],8,2,QChar('0'));
			}
			else
			{
				for(int j=0 ; j<tailleBA ; j++)
				{
					nouvelleCommande[departBA+j] = (char)((value>>(j*8)) & 0xFF);
					// qDebug() << "nouvelleCommande[" << departBA << "]=" << QString("%1").arg((quint8)nouvelleCommande[departBA],8,2,QChar('0'));
				}
			}
			// qDebug() << "nouvelleCommande= " << nouvelleCommande.toHex();
			
			cur += paramSize;
			// qDebug() << "cur= " << cur;
		}
	}
	else
	{
		for(quint8 i=start ; i<length ; ++i)
			nouvelleCommande[i] = model->item(i-start, 1)->text().toUInt();
	}
	
	return nouvelleCommande;
}

void ScriptEditor::modify()
{
	/* if(!this->change) {
		close();
		return;
	} */
	QByteArray nouvelleCommande = parseModel();
	if(nouvelleCommande.isEmpty())	return;
	script->setCommande(this->commandeID, Script::createOpcode(nouvelleCommande));
	accept();
}

void ScriptEditor::add()
{
	/* if(!this->change) {
		close();
		return;
	} */
	QByteArray nouvelleCommande = parseModel();
	if(nouvelleCommande.isEmpty())	return;
	script->insertCommande(this->commandeID, Script::createOpcode(nouvelleCommande));
	accept();
}

void ScriptEditor::refreshTextEdit()
{
	this->change = true;
	
//	commande->setCommande(parseModel());//TODO
	textEdit->setPlainText(commande->toString());
}

void ScriptEditor::addParam()
{
	if(model->rowCount()<252)
	{
		addRow(0, 0, 255, inconnu);
		refreshTextEdit();
	}
}

void ScriptEditor::delLastRow()
{
	QList<QStandardItem *> items = model->takeRow(model->rowCount()-1);
	foreach(QStandardItem *item, items)	delete item;
	refreshTextEdit();
}

void ScriptEditor::addRow(int value, int minValue, int maxValue, int type)
{
	QList<QStandardItem *> items;
	QStandardItem *standardItem;
	standardItem = new QStandardItem(paramName(type));
	standardItem->setEditable(false);
	items << standardItem;
	
	standardItem = new QStandardItem(QString("%1").arg(value));
	standardItem->setData(minValue, Qt::UserRole);
	standardItem->setData(maxValue, Qt::UserRole+1);
	standardItem->setData(type, Qt::UserRole+2);
	items << standardItem;
	model->appendRow(items);
}

void ScriptEditor::fillModel(int opcode)
{
	addButton->hide();
	delButton->hide();
	model->clear();
	/* tableView->hide();
	nouvelEditeur->show();
	
	if(opcode>=0x10 && opcode<=0x13)
	{
		nouvelEditeur->setCurrentIndex(0);
		((GotoPage *)nouvelEditeur->currentWidget())->fill(opcode, commande->getParams());
		return;
	}
	else if(opcode==0x00 || opcode==0x07)
	{
		nouvelEditeur->setCurrentIndex(1);
		((ReturnPage *)nouvelEditeur->currentWidget())->fill(opcode, commande->getParams());
		return;
	}
	else if(opcode>=0x01 && opcode<=0x03)
	{
		nouvelEditeur->setCurrentIndex(2);
		((ExecPage *)nouvelEditeur->currentWidget())->fill(opcode, commande->getParams());
		return;
	}
	else if(opcode>=0x04 && opcode<=0x06)
	{
		nouvelEditeur->setCurrentIndex(3);
		((ExecCharPage *)nouvelEditeur->currentWidget())->fill(opcode, commande->getParams());
		return;
	}
	else if(opcode>=0x14 && opcode<=0x19)
	{
		nouvelEditeur->setCurrentIndex(4);
		((IfPage *)nouvelEditeur->currentWidget())->fill(opcode, commande->getParams());
		return;
	}
	else if(opcode>=0x30 && opcode<=0x32)
	{
		nouvelEditeur->setCurrentIndex(5);
		((IfKeyPage *)nouvelEditeur->currentWidget())->fill(opcode, commande->getParams());
		return;
	}
	else if(opcode==0xCB || opcode==0xCC)
	{
		nouvelEditeur->setCurrentIndex(6);
		((IfCharPage *)nouvelEditeur->currentWidget())->fill(opcode, commande->getParams());
		return;
	}
	else if(opcode==0x24)
	{
		nouvelEditeur->setCurrentIndex(7);
		((WaitPage *)nouvelEditeur->currentWidget())->fill(opcode, commande->getParams());
		return;
	}
	else if(opcode==0x5F)
	{
		nouvelEditeur->setCurrentIndex(8);
		((NopPage *)nouvelEditeur->currentWidget())->fill(opcode, commande->getParams());
		return;
	}
	else if((opcode>=0x76 && opcode<=0x79) || opcode==0x80 || opcode==0x81 || (opcode>=0x85 && opcode<=0x94))
	{
		nouvelEditeur->setCurrentIndex(9);
		((OpPage *)nouvelEditeur->currentWidget())->fill(opcode, commande->getParams());
		return;
	}
	else if((opcode>=0x7A && opcode<=0x7D) || (opcode>=0x95 && opcode<=0x98))
	{
		nouvelEditeur->setCurrentIndex(10);
		((Op1Page *)nouvelEditeur->currentWidget())->fill(opcode, commande->getParams());
		return;
	}
	else if(opcode==0xE0 || opcode==0xE1)
	{
		nouvelEditeur->setCurrentIndex(11);
		((BgParamStatePage *)nouvelEditeur->currentWidget())->fill(opcode, commande->getParams());
		return;
	}
	
	tableView->show();
	nouvelEditeur->hide();*/
	
	int paramSize, paramType, cur = 0, maxValue, minValue, value=0;
	QList<int> paramTypes = this->paramTypes(opcode);
	
	if(paramTypes.isEmpty())
	{
		int start = 0;
		if(opcode == 0x0F)//SPECIAL
			start = 1;
		if(opcode == 0x28)//KAWAI
		{
			start = 2;
			addButton->show();
			delButton->show();
		}
		const QByteArray params = commande->params();
		for(int i=start ; i<params.size() ; ++i)
			addRow((quint8)params.at(i), 0, 255, inconnu);
	}
	else
	{
		for(quint8 i=0 ; i<paramTypes.size() ; ++i)
		{
			paramType = paramTypes.at(i);
			paramSize = this->paramSize(paramType);
//			value = commande->subParam(cur, paramSize);//TODO
			// qDebug() << value;
			if(paramIsSigned(paramType)) {
				maxValue = (int)pow(2, paramSize-1)-1;
				minValue = -maxValue-1;
				if(value>maxValue)
					value -= (int)pow(2, paramSize);
			}
			else
			{
				maxValue = (int)pow(2, paramSize)-1;
				minValue = 0;
			}			
			addRow(value, minValue, maxValue, paramType);
			cur += paramSize;
		}
	}
}

void ScriptEditor::changeCurrentOpcode(int index)
{
	this->change = true;
	QByteArray nouvelleCommande;
	int itemData = comboBox->itemData(index).toInt();
	const quint8 opcode = itemData & 0xFF;
	nouvelleCommande.append((char)opcode);
	
	if(opcode == 0x28)//KAWAI
	{
		nouvelleCommande.append('\x03');
		nouvelleCommande.append((char)((itemData >> 8) & 0xFF));
	}
	else if(opcode == 0x0F)//SPECIAL
	{
		quint8 byte2 = (itemData >> 8) & 0xFF;
		nouvelleCommande.append((char)byte2);
		switch(byte2)
		{
		case 0xF5:case 0xF6:case 0xF7:case 0xFB:case 0xFC:
					nouvelleCommande.append('\x00');
			break;
		case 0xF8:case 0xFD:
					nouvelleCommande.append('\x00');
			nouvelleCommande.append('\x00');
			break;
		}
	}
	else
	{
		for(quint8 pos=1 ; pos<Opcode::length[opcode] ; ++pos)
			nouvelleCommande.append('\x00');
	}
//	commande->setCommande(nouvelleCommande);//TODO
	
	fillModel(opcode);
	
	textEdit->setPlainText(commande->toString());
	if(isInit && crashIfInit.contains(opcode))
	{
		textEdit->setDisabled(true);
		tableView->setDisabled(true);
		addButton->setDisabled(true);
		delButton->setDisabled(true);
		ok->setDisabled(true);
	}
	else
	{
		textEdit->setEnabled(true);
		tableView->setEnabled(true);
		addButton->setEnabled(true);
		delButton->setEnabled(true);
		ok->setEnabled(true);
	}
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

QList<int> ScriptEditor::paramTypes(int id)
{
	QList<int> paramTypes;
	switch(id)
	{
	//case 0x00:return paramTypes;
	case 0x01:case 0x02:case 0x03:
		paramTypes<<group_id<<priorite<<script_id;return paramTypes;
	case 0x04:case 0x05:case 0x06:
		paramTypes<<party_id<<priorite<<script_id;return paramTypes;
	case 0x07:
		paramTypes<<priorite<<script_id;return paramTypes;
	case 0x08:
		paramTypes<<vitesse;return paramTypes;
	case 0x09:
		paramTypes<<bank<<bank<<bank<<bank<<bank<<bank<<coord_x<<coord_y<<direction<<coord_x<<coord_y<<direction<<vitesse;return paramTypes;
	case 0x0A:case 0x0B:
		paramTypes<<bank<<bank<<bank<<bank<<party_id<<party_id<<party_id;return paramTypes;
	//case 0x0C:case 0x0D:return paramTypes;
	case 0x0E:
		paramTypes<<cd_id;return paramTypes;
	//case 0x0F:return;//SPECIAL
	case 0x10:case 0x12:
		paramTypes<<jump;return paramTypes;
	case 0x11:case 0x13:
		paramTypes<<jump_l;return paramTypes;
	case 0x14:
		paramTypes<<bank<<bank<<byte<<byte<<operateur<<jump;return paramTypes;
	case 0x15:
		paramTypes<<bank<<bank<<byte<<byte<<operateur<<jump_l;return paramTypes;
	case 0x16:
		paramTypes<<bank<<bank<<sword<<sword<<operateur<<jump;return paramTypes;
	case 0x17:
		paramTypes<<bank<<bank<<sword<<sword<<operateur<<jump_l;return paramTypes;
	case 0x18:
		paramTypes<<bank<<bank<<word<<word<<operateur<<jump;return paramTypes;
	case 0x19:
		paramTypes<<bank<<bank<<word<<word<<operateur<<jump_l;return paramTypes;
	//case 0x1A:case 0x1B:case 0x1C:case 0x1D:case 0x1E:case 0x1F:return paramTypes;
	case 0x20:
		paramTypes<<field_id<<coord_x<<coord_y<<polygone_id<<byte<<minijeu_id;return paramTypes;
	case 0x21:
		paramTypes<<tuto_id;return paramTypes;
	case 0x22:
		paramTypes<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit;return paramTypes;
	case 0x23:
		paramTypes<<bank<<bank<<byte;return paramTypes;
	case 0x24:
		paramTypes<<word;return paramTypes;
	case 0x25://TODO
		paramTypes<<inconnu<<inconnu<<inconnu<<color<<inconnu<<inconnu;return paramTypes;
	case 0x26:
		paramTypes<<boolean;return paramTypes;
	case 0x27:
		paramTypes<<boolean;return paramTypes;
	//case 0x28://KAWAI
	//case 0x29:return paramTypes;
	case 0x2A:
		paramTypes<<party_id;return paramTypes;
	case 0x2B:
		paramTypes<<boolean;return paramTypes;
	case 0x2C:
		paramTypes<<bank<<bank<<layer_id<<coord_z;return paramTypes;
	case 0x2D:
		paramTypes<<bank<<bank<<layer_id<<sword<<sword;return paramTypes;
	case 0x2E:
		paramTypes<<window_id;return paramTypes;
	case 0x2F:
		paramTypes<<window_id<<coord_x<<coord_y<<window_w<<window_h;return paramTypes;
	case 0x30:case 0x31:case 0x32:
		paramTypes<<keys<<jump;return paramTypes;
	case 0x33:
		paramTypes<<boolean;return paramTypes;
	case 0x34:
		paramTypes<<party_id;return paramTypes;
	case 0x35:
		paramTypes<<party_id<<vitesse<<rotation;return paramTypes;
	case 0x36:
		paramTypes<<window_id<<window_num<<byte<<byte;return paramTypes;
	case 0x37:
		paramTypes<<bank<<bank<<window_id<<word<<word<<byte;return paramTypes;
	case 0x38:
		paramTypes<<bank<<bank<<bank<<bank<<byte<<byte<<byte;return paramTypes;
	case 0x39:case 0x3A:
		paramTypes<<bank<<bank<<word<<word;return paramTypes;
	case 0x3B:
		paramTypes<<bank<<bank<<adress<<adress;return paramTypes;
	//case 0x3C:case 0x3D:case 0x3E:case 0x3F:return paramTypes;
	case 0x40:
		paramTypes<<window_id<<text_id;return paramTypes;
	case 0x41:
		paramTypes<<bank<<bank<<window_id<<window_var<<byte;return paramTypes;
	case 0x42:
		paramTypes<<bank<<bank<<window_id<<window_var<<word;return paramTypes;
	case 0x43:
		paramTypes<<text_id;return paramTypes;
	//case 0x44:case 0x46:return paramTypes;
	case 0x45:case 0x47:
		paramTypes<<bank<<bank<<party_id<<word;return paramTypes;
	case 0x48:
		paramTypes<<bank<<bank<<window_id<<text_id<<byte<<byte<<adress;return paramTypes;
	case 0x49:
		paramTypes<<bank<<bank<<menu<<byte;return paramTypes;
	case 0x4A:
		paramTypes<<boolean;return paramTypes;
	case 0x4B:
		paramTypes<<byte;return paramTypes;
	//case 0x4C:case 0x4E:return paramTypes;
	case 0x4D:case 0x4F:
		paramTypes<<bank<<bank<<party_id<<word;return paramTypes;
	case 0x50:
		paramTypes<<window_id<<coord_x<<coord_y<<window_w<<window_h;return paramTypes;
	case 0x51:
		paramTypes<<window_id<<coord_x<<coord_y;return paramTypes;
	case 0x52:
		paramTypes<<window_id<<window_type<<boolean;return paramTypes;
	case 0x53:case 0x54:
		paramTypes<<window_id;return paramTypes;
	case 0x55:
		paramTypes<<window_id<<byte;return paramTypes;
	case 0x56:
		paramTypes<<bank<<bank<<bank<<bank<<adress<<adress<<adress<<adress;return paramTypes;
	case 0x57:
		paramTypes<<bank<<bank<<bank<<bank<<byte<<color;return paramTypes;
	case 0x58:case 0x59:
		paramTypes<<bank<<bank<<item_id<<quantity;return paramTypes;
	case 0x5A:
		paramTypes<<bank<<bank<<item_id<<adress;return paramTypes;
	case 0x5B:
		paramTypes<<bank<<bank<<bank<<bank<<materia_id<<byte<<byte<<byte;return paramTypes;
	case 0x5C:
		paramTypes<<bank<<bank<<bank<<bank<<materia_id<<byte<<byte<<byte<<quantity;return paramTypes;
	case 0x5D:
		paramTypes<<bank<<bank<<bank<<bank<<bank<<bank<<materia_id<<byte<<byte<<byte<<inconnu<<adress;return paramTypes;
	//case 0x5E://TODO
	//case 0x5F:return paramTypes;
	case 0x60:
		paramTypes<<field_id<<coord_x<<coord_y<<polygone_id<<direction;return paramTypes;
	//case 0x61:case 0x62://TODO
	case 0x63:
		paramTypes<<bank<<bank<<vitesse2<<group_id<<byte;return paramTypes;
	case 0x64:
		paramTypes<<bank<<bank<<coord_x<<coord_y;return paramTypes;
	//case 0x65:case 0x67:return paramTypes;
	case 0x66:case 0x68:
		paramTypes<<bank<<bank<<bank<<bank<<coord_x<<coord_y<<vitesse2;return paramTypes;
	case 0x69:
		paramTypes<<boolean;return paramTypes;
	//case 0x6A://TODO
	case 0x6B:
		paramTypes<<bank<<bank<<bank<<bank<<color<<vitesse<<byte<<byte;return paramTypes;
	//case 0x6C:return paramTypes;
	case 0x6D:
		paramTypes<<boolean<<polygone_id;return paramTypes;
	case 0x6E:
		paramTypes<<bank<<bank<<adress;return paramTypes;
	case 0x6F:
		paramTypes<<bank<<bank<<vitesse2<<party_id<<byte;return paramTypes;
	case 0x70:
		paramTypes<<bank<<bank<<word;return paramTypes;
	case 0x71:
		paramTypes<<boolean;return paramTypes;
	//case 0x72://TODO
	case 0x73:case 0x74:
		paramTypes<<bank<<bank<<party_id<<adress;return paramTypes;
	case 0x75:
		paramTypes<<bank<<bank<<bank<<bank<<party_id<<adress<<adress<<adress<<adress;return paramTypes;
	case 0x76:case 0x78:case 0x80:case 0x82:case 0x83:case 0x84:case 0x85:case 0x87:case 0x89:case 0x8B:case 0x8D:case 0x8F:case 0x91:case 0x93:case 0x9A:
		paramTypes<<bank<<bank<<adress<<byte;return paramTypes;
	case 0x77:case 0x79:case 0x81:case 0x86:case 0x88:case 0x8A:case 0x8C:case 0x8E:case 0x90:case 0x92:case 0x94:case 0x9B:
		paramTypes<<bank<<bank<<adress<<word;return paramTypes;
	case 0x7A:case 0x7B:case 0x7C:case 0x7D:case 0x95:case 0x96:case 0x97:case 0x98:case 0x99:
		paramTypes<<bank<<bank<<adress;return paramTypes;
	case 0x7E:
		paramTypes<<boolean;return paramTypes;
	case 0x7F:
		paramTypes<<bank<<bank<<byte;return paramTypes;
	case 0x9C:
		paramTypes<<bank<<bank<<bank<<bank<<adress<<byte<<byte;return paramTypes;
	//case 0x9D:case 0x9E:case 0x9F://TODO
	case 0xA0:
		paramTypes<<personnage_id;return paramTypes;
	case 0xA1:
		paramTypes<<byte;return paramTypes;
	case 0xA2:case 0xA3:case 0xAE:case 0xAF:
		paramTypes<<animation_id<<vitesse;return paramTypes;
	case 0xA4:
		paramTypes<<boolean;return paramTypes;
	case 0xA5:
		paramTypes<<bank<<bank<<bank<<bank<<coord_x<<coord_y<<coord_z<<polygone_id;return paramTypes;
	case 0xA6:
		paramTypes<<bank<<bank<<bank<<bank<<coord_x<<coord_y<<polygone_id;return paramTypes;
	case 0xA7:
		paramTypes<<bank<<bank<<bank<<bank<<coord_x<<coord_y<<coord_z;return paramTypes;
	case 0xA8:case 0xA9:case 0xAD:
		paramTypes<<bank<<bank<<coord_x<<coord_y;return paramTypes;
	case 0xAA:
		paramTypes<<group_id;return paramTypes;
	case 0xAB:
		paramTypes<<group_id<<rotation<<vitesse;return paramTypes;
	//case 0xAC:return paramTypes;
	case 0xB0:case 0xB1:case 0xBB:case 0xBC:
		paramTypes<<animation_id<<byte<<byte<<vitesse;return paramTypes;
	case 0xB2:case 0xBD:
		paramTypes<<bank<<bank<<vitesse2;return paramTypes;
	case 0xB3:
		paramTypes<<bank<<bank<<direction;return paramTypes;
	case 0xB4:case 0xB5:
		paramTypes<<bank<<bank<<direction<<byte<<vitesse<<inconnu;return paramTypes;
	case 0xB6:
		paramTypes<<group_id;return paramTypes;
	case 0xB7:case 0xB9:
		paramTypes<<bank<<bank<<group_id<<adress;return paramTypes;
	case 0xB8:
		paramTypes<<bank<<bank<<group_id<<adress<<adress;return paramTypes;
	case 0xBA:
		paramTypes<<animation_id<<vitesse;return paramTypes;
	//case 0xBE:return paramTypes;
	case 0xBF:
		paramTypes<<group_id;return paramTypes;
	case 0xC0:
		paramTypes<<bank<<bank<<bank<<bank<<coord_x<<coord_y<<polygone_id<<word;return paramTypes;
	case 0xC1:
		paramTypes<<bank<<bank<<bank<<bank<<group_id<<adress<<adress<<adress<<adress;return paramTypes;
	case 0xC2:
		paramTypes<<bank<<bank<<bank<<bank<<coord_x<<coord_y<<coord_z<<polygone_id<<byte<<animation_id<<direction<<vitesse;return paramTypes;
	case 0xC3:
		paramTypes<<bank<<bank<<bank<<bank<<byte<<coord_x<<coord_y<<coord_z<<vitesse2;return paramTypes;
	//case 0xC4:return paramTypes;
	case 0xC5:case 0xC6:
		paramTypes<<bank<<bank<<byte;return paramTypes;
	case 0xC7:
		paramTypes<<boolean;return paramTypes;
	case 0xC8:case 0xC9:case 0xCE:case 0xCF:
		paramTypes<<personnage_id;return paramTypes;
	case 0xCA:
		paramTypes<<personnage_id<<personnage_id<<personnage_id;return paramTypes;
	case 0xCB:case 0xCC:
		paramTypes<<personnage_id<<jump;return paramTypes;
	case 0xCD:
		paramTypes<<boolean<<personnage_id;return paramTypes;
	case 0xD0:
		paramTypes<<coord_x<<coord_y<<coord_z<<coord_x<<coord_y<<coord_z;return paramTypes;
	case 0xD1:case 0xD2:
		paramTypes<<boolean;return paramTypes;
	case 0xD3:
		paramTypes<<bank<<bank<<bank<<bank<<bank<<bank<<coord_x<<coord_y<<coord_z<<coord_x<<coord_y<<coord_z;return paramTypes;
	case 0xD4:case 0xD5:
		paramTypes<<bank<<bank<<bank<<bank<<word<<word<<word<<byte;return paramTypes;
	case 0xD6:case 0xD7:
		paramTypes<<bank<<bank<<word;return paramTypes;
	case 0xD8:
		paramTypes<<field_id;return paramTypes;
	//case 0xD9:return paramTypes;
	//case 0xDA://TODO
	case 0xDB:
		paramTypes<<boolean;return paramTypes;
	case 0xDC:
		paramTypes<<animation_id<<vitesse<<byte;return paramTypes;
	//case 0xDD:case 0xDE:return paramTypes;
	//case 0xDF://TODO
	case 0xE0:case 0xE1:
		paramTypes<<bank<<bank<<parametre_id<<state_id;return paramTypes;
	case 0xE2:case 0xE3:case 0xE4:
		paramTypes<<bank<<bank<<parametre_id;return paramTypes;
	//case 0xE5:case 0xE6:case 0xE7:case 0xE8:case 0xE9:case 0xEA:case 0xEB:case 0xEC:case 0xED:case 0xEE:case 0xEF://TODO
	case 0xF0:case 0xF3:case 0xF4:case 0xF6:
		paramTypes<<music_id;return paramTypes;
	case 0xF1:
		paramTypes<<bank<<bank<<sound_id<<byte;return paramTypes;
	//case 0xF2:case 0xF7://TODO
	case 0xF5:
		paramTypes<<boolean;return paramTypes;
	case 0xF8:
		paramTypes<<movie_id;return paramTypes;
	//case 0xF9:return paramTypes;
	case 0xFA:
		paramTypes<<bank<<bank<<adress;return paramTypes;
	case 0xFB:
		paramTypes<<boolean;return paramTypes;
	//case 0xFC:case 0xFD://TODO
	case 0xFE:
		paramTypes<<bank<<bank<<adress;return paramTypes;
	//case 0xFF:return paramTypes;
	}
	return paramTypes;
}

int ScriptEditor::paramSize(int type)
{
	switch(type)
	{
	case color:				return 24;
	case word:
	case sword:
	case jump_l:
	case coord_x:
	case coord_y:
	case coord_z:
	case window_w:
	case window_h:
	case item_id:
	case vitesse2:
	case polygone_id:
	case sound_id:
	case keys:
	case field_id:			return 16;
	case script_id:			return 5;
	case bank:				return 4;
	case priorite:			return 3;
	case bit: 				return 1;
	}
	return 8;
}

bool ScriptEditor::paramIsSigned(int type)
{
	switch(type)
	{
	case sword:
	case coord_x:
	case coord_y:
	case coord_z:	return true;
	default:		return false;
	}
}

QString ScriptEditor::paramName(int type)
{
	switch(type)
	{
	case word:				return tr("Entier long");
	case sword:				return tr("Entier long signé");
	case coord_x:			return tr("Coordonnée X");
	case coord_y:			return tr("Coordonnée Y");
	case coord_z:			return tr("Coordonnée Z");
	case field_id:			return tr("Écran");
	case tuto_id:			return tr("Tutoriel");
	case personnage_id:		return tr("Personnage");
	case cd_id:				return tr("Disque");
	case minijeu_id:		return tr("Mini-jeu");
	case byte:				return tr("Entier court");
	case vitesse:			return tr("Vitesse (8 bits)");
	case vitesse2:			return tr("Vitesse (16 bits)");
	case direction:			return tr("Direction");
	case polygone_id:		return tr("Polygone");
	case group_id:			return tr("Groupe");
	case script_id:			return tr("Script");
	case party_id:			return tr("Équipier");
	case bank:				return tr("Bank");
	case adress:			return tr("Adresse");
	case priorite:			return tr("Priorité");
	case bit: 				return tr("Flag");
	case jump: 				return tr("Saut court");
	case jump_l: 			return tr("Saut long");
	case operateur: 		return tr("Opérateur");
	case boolean: 			return tr("Booléen");
	case layer_id: 			return tr("Couche");
	case parametre_id: 		return tr("Paramètre");
	case state_id: 			return tr("État");
	case window_id: 		return tr("Fenêtre");
	case window_w: 			return tr("Largeur");
	case window_h: 			return tr("Hauteur");
	case window_var: 		return tr("Variable");
	case keys: 				return tr("Touche(s)");
	case rotation: 			return tr("Sens de rotation");
	case window_num: 		return tr("Type d'affichage");
	case text_id: 			return tr("Texte");
	case menu: 				return tr("Menu");
	case window_type: 		return tr("Type de fenêtre");
	case item_id: 			return tr("Objet");
	case materia_id: 		return tr("Matéria");
	case quantity: 			return tr("Quantité");
	case color: 			return tr("Couleur");
	case animation_id: 		return tr("Animation");
	case music_id: 			return tr("Musique");
	case sound_id: 			return tr("Son");
	case movie_id: 			return tr("Vidéo");
	}
	return tr("???");
}
