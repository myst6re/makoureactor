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
#include "ScriptEditorGenericList.h"
#include "Delegate.h"

ScriptEditorGenericList::ScriptEditorGenericList(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
    ScriptEditorView(field, grpScript, script, opcodeID, parent),
    addButton(0), delButton(0), tableView(0), model(0)
{
}

void ScriptEditorGenericList::build()
{
	model = new QStandardItemModel(this);
	tableView = new QTableView(this);
	tableView->setModel(model);
	tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	tableView->setSelectionMode(QAbstractItemView::SingleSelection);
	tableView->horizontalHeader()->setVisible(false);
	tableView->setEditTriggers(QAbstractItemView::CurrentChanged | QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
	tableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

	SpinBoxDelegate *delegate = new SpinBoxDelegate(this);
	delegate->setField(field());
	tableView->setItemDelegate(delegate);
	tableView->horizontalHeader()->setStretchLastSection(true);

	addButton = new QPushButton(tr("Add a line"),this);
	addButton->hide();
	delButton = new QPushButton(tr("Delete a line"),this);
	delButton->hide();

	QHBoxLayout *buttonLayout = new QHBoxLayout();
	buttonLayout->addWidget(addButton);
	buttonLayout->addWidget(delButton);
	buttonLayout->addStretch();
	buttonLayout->setContentsMargins(QMargins());

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(new QLabel(tr("Arguments:"), this));
	layout->addWidget(tableView);
	layout->addStretch();
	layout->addLayout(buttonLayout);
	layout->setContentsMargins(QMargins());

	connect(model, SIGNAL(itemChanged(QStandardItem *)), SIGNAL(opcodeChanged()));
	connect(addButton, SIGNAL(released()), SLOT(addParam()));
	connect(delButton, SIGNAL(released()), SLOT(delLastRow()));
}

Opcode *ScriptEditorGenericList::opcode()
{
	bool isLabel;
	QByteArray newOpcode = parseModel(&isLabel);
	if (newOpcode.isEmpty()) {
		return opcodePtr();
	}

	if (isLabel) {
		quint32 label;
		memcpy(&label, newOpcode.constData() + 1, 4);
		ScriptEditorView::setOpcode(new OpcodeLabel(label));
	} else {
		ScriptEditorView::setOpcode(Script::createOpcode(newOpcode));
	}

	return opcodePtr();
}

void ScriptEditorGenericList::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);
	fillModel();
}

QByteArray ScriptEditorGenericList::parseModel(bool *isLabel)
{
	*isLabel = false;

	QByteArray newOpcode;
	quint8 byte, length, start;

	*isLabel = opcodePtr()->isLabel();
	byte = opcodePtr()->id() & 0xFF;
	newOpcode.append((char)byte);
	// Compute opcode length
	length = 0;
	start = 1;
	
	if (byte == 0x0F) { // SPECIAL
		quint8 byte2 = ((OpcodeSPECIAL *)opcodePtr())->opcode->id();
		newOpcode.append((char)byte2);
		switch (byte2) {
		case 0xF5:case 0xF6:case 0xF7:case 0xFB:case 0xFC:
				length = 1;
			break;
		case 0xF8:case 0xFD:
				length = 2;
			break;
		}
		start = 2;
	} else if (byte == 0x28) { // KAWAI
		quint8 byte3 = ((OpcodeKAWAI *)opcodePtr())->opcode->id();
		length = model->rowCount()+3;
		newOpcode.append((char)length);
		newOpcode.append((char)byte3);
		for (quint8 i=0; i<length-3; ++i) {
			newOpcode.append(model->item(i, 1)->text().toUInt());
		}
		return newOpcode;
	}

	length += Opcode::length[opcodePtr()->id()];
	newOpcode.append(QByteArray(length-start, '\x0'));
	QList<int> paramTypes = this->paramTypes(opcodePtr()->id());

	if (!paramTypes.isEmpty()) {
		int cur = 8;
		for (quint8 i=0; i<paramTypes.size(); ++i) {
			int paramType = paramTypes.at(i),
			    value = model->data(model->index(i, 1), Qt::EditRole).toInt();

			int paramSize = this->paramSize(paramType);
			int startBA = cur / 8, sizeBA;
			if (paramSize % 8 != 0) {
				sizeBA = paramSize / 8 + 1;
			} else {
				sizeBA = paramSize / 8;
			}

			if (paramSize < 8) {
				int startLocal = cur % 8;
				newOpcode[startBA] = (char)((quint8)newOpcode.at(startBA) | (value << (8-paramSize-startLocal)));
			} else if (paramSize == 8) {
				newOpcode[startBA] = (char)(value & 0xFF);
			} else {
				for (int j=0; j<sizeBA; j++) {
					newOpcode[startBA+j] = (char)((value>>(j*8)) & 0xFF);
				}
			}

			cur += paramSize;
		}
	} else {
		for (quint8 i=start; i<length; ++i) {
			newOpcode[i] = model->item(i-start, 1)->text().toUInt();
		}
	}

	return newOpcode;
}

void ScriptEditorGenericList::addParam()
{
	if (model->rowCount() < 252) {
		addRow(0, 0, 255, inconnu);
		emit opcodeChanged();
	}
}

void ScriptEditorGenericList::delLastRow()
{
	qDeleteAll(model->takeRow(model->rowCount()-1));
	emit opcodeChanged();
}

void ScriptEditorGenericList::addRow(int value, int minValue, int maxValue, int type)
{
	QList<QStandardItem *> items;
	QStandardItem *standardItem;
	standardItem = new QStandardItem(paramName(type));
	standardItem->setEditable(false);
	items.append(standardItem);
	
	standardItem = new QStandardItem(QString("%1").arg(value));
	standardItem->setData(minValue, Qt::UserRole);
	standardItem->setData(maxValue, Qt::UserRole+1);
	standardItem->setData(type, Qt::UserRole+2);
	items.append(standardItem);
	model->appendRow(items);
}

void ScriptEditorGenericList::fillModel()
{
	addButton->hide();
	delButton->hide();
	model->clear();

	int paramSize, paramType, cur = 0, maxValue, minValue, value=0;
	QList<int> paramTypes = this->paramTypes(opcodePtr()->id());
	
	if (opcodePtr()->isLabel()) {
		addRow(((OpcodeLabel *)opcodePtr())->label(), 0, (int)pow(2, 31)-1, label);
	} else if (paramTypes.isEmpty()) {
		int start = 0;
		if (opcodePtr()->id() == 0x0F) { //SPECIAL
			start = 1;
		}
		if (opcodePtr()->id() == 0x28) { //KAWAI
			start = 2;
			addButton->show();
			delButton->show();
		}
		const QByteArray params = opcodePtr()->params();
		for (int i=start; i<params.size(); ++i) {
			addRow((quint8)params.at(i), 0, 255, inconnu);
		}
	} else {
		for (quint8 i=0; i<paramTypes.size(); ++i) {
			paramType = paramTypes.at(i);
			paramSize = this->paramSize(paramType);
			value = opcodePtr()->subParam(cur, paramSize);
			// qDebug() << value;
			if (paramIsSigned(paramType)) {
				maxValue = (int)pow(2, paramSize-1)-1;
				minValue = -maxValue-1;
				if (value>maxValue) {
					value -= (int)pow(2, paramSize);
				}
			} else {
				maxValue = (int)pow(2, paramSize)-1;
				minValue = 0;
			}			
			addRow(value, minValue, maxValue, paramType);
			cur += paramSize;
		}
	}
}

QList<int> ScriptEditorGenericList::paramTypes(int id)
{
	QList<int> paramTypes;
	switch (id) {
	//case 0x00:break;
	case 0x01:case 0x02:case 0x03:
		paramTypes<<group_id<<priorite<<script_id;break;
	case 0x04:case 0x05:case 0x06:
		paramTypes<<party_id<<priorite<<script_id;break;
	case 0x07:
		paramTypes<<priorite<<script_id;break;
	case 0x08:
		paramTypes<<vitesse;break;
	case 0x09:
		paramTypes<<bank<<bank<<bank<<bank<<bank<<bank<<coord_x<<coord_y<<direction<<coord_x<<coord_y<<direction<<vitesse;break;
	case 0x0A:case 0x0B:
		paramTypes<<bank<<bank<<bank<<bank<<party_id<<party_id<<party_id;break;
	//case 0x0C:case 0x0D:break;
	case 0x0E:
		paramTypes<<cd_id;break;
	//case 0x0F:return;//SPECIAL
	case 0x10:case 0x12:
		paramTypes<<jump;break;
	case 0x11:case 0x13:
		paramTypes<<jump_l;break;
	case 0x14:
		paramTypes<<bank<<bank<<byte<<byte<<operateur<<jump;break;
	case 0x15:
		paramTypes<<bank<<bank<<byte<<byte<<operateur<<jump_l;break;
	case 0x16:
		paramTypes<<bank<<bank<<sword<<sword<<operateur<<jump;break;
	case 0x17:
		paramTypes<<bank<<bank<<sword<<sword<<operateur<<jump_l;break;
	case 0x18:
		paramTypes<<bank<<bank<<word<<word<<operateur<<jump;break;
	case 0x19:
		paramTypes<<bank<<bank<<word<<word<<operateur<<jump_l;break;
	case 0x1A:
		paramTypes<<word<<word<<dword<<byte;break;
	case 0x1B:
		paramTypes<<jump_l;break;
	//case 0x1C:case 0x1D:case 0x1E:case 0x1F:break;
	case 0x20:
		paramTypes<<field_id<<coord_x<<coord_y<<polygone_id<<byte<<minijeu_id;break;
	case 0x21:
		paramTypes<<tuto_id;break;
	case 0x22:
		paramTypes<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit;break;
	case 0x23:
		paramTypes<<bank<<bank<<byte;break;
	case 0x24:
		paramTypes<<word;break;
	case 0x25:
		paramTypes<<bank<<bank<<bank<<bank<<byte<<color<<vitesse<<inconnu;break;
	case 0x26:
		paramTypes<<boolean;break;
	case 0x27:
		paramTypes<<boolean;break;
	//case 0x28://KAWAI
	//case 0x29:break;
	case 0x2A:
		paramTypes<<party_id;break;
	case 0x2B:
		paramTypes<<boolean;break;
	case 0x2C:
		paramTypes<<bank<<bank<<layer_id<<coord_z;break;
	case 0x2D:
		paramTypes<<bank<<bank<<layer_id<<sword<<sword;break;
	case 0x2E:
		paramTypes<<window_id;break;
	case 0x2F:
		paramTypes<<window_id<<coord_x<<coord_y<<window_w<<window_h;break;
	case 0x30:case 0x31:case 0x32:
		paramTypes<<keys<<jump;break;
	case 0x33:
		paramTypes<<boolean;break;
	case 0x34:
		paramTypes<<party_id;break;
	case 0x35:
		paramTypes<<party_id<<vitesse<<rotation;break;
	case 0x36:
		paramTypes<<window_id<<window_num<<byte<<byte;break;
	case 0x37:
		paramTypes<<bank<<bank<<window_id<<word<<word<<byte;break;
	case 0x38:
		paramTypes<<bank<<bank<<bank<<bank<<byte<<byte<<byte;break;
	case 0x39:case 0x3A:
		paramTypes<<bank<<bank<<word<<word;break;
	case 0x3B:
		paramTypes<<bank<<bank<<adress<<adress;break;
	//case 0x3C:case 0x3D:case 0x3E:case 0x3F:break;
	case 0x40:
		paramTypes<<window_id<<text_id;break;
	case 0x41:
		paramTypes<<bank<<bank<<window_id<<window_var<<byte;break;
	case 0x42:
		paramTypes<<bank<<bank<<window_id<<window_var<<word;break;
	case 0x43:
		paramTypes<<text_id;break;
	//case 0x44:case 0x46:break;
	case 0x45:case 0x47:
		paramTypes<<bank<<bank<<party_id<<word;break;
	case 0x48:
		paramTypes<<bank<<bank<<window_id<<text_id<<byte<<byte<<adress;break;
	case 0x49:
		paramTypes<<bank<<bank<<menu<<byte;break;
	case 0x4A:
		paramTypes<<boolean;break;
	case 0x4B:
		paramTypes<<byte;break;
	//case 0x4C:case 0x4E:break;
	case 0x4D:case 0x4F:
		paramTypes<<bank<<bank<<party_id<<word;break;
	case 0x50:
		paramTypes<<window_id<<coord_x<<coord_y<<window_w<<window_h;break;
	case 0x51:
		paramTypes<<window_id<<coord_x<<coord_y;break;
	case 0x52:
		paramTypes<<window_id<<window_type<<boolean;break;
	case 0x53:case 0x54:
		paramTypes<<window_id;break;
	case 0x55:
		paramTypes<<window_id<<byte;break;
	case 0x56:
		paramTypes<<bank<<bank<<bank<<bank<<adress<<adress<<adress<<adress;break;
	case 0x57:
		paramTypes<<bank<<bank<<bank<<bank<<byte<<color;break;
	case 0x58:case 0x59:
		paramTypes<<bank<<bank<<item_id<<quantity;break;
	case 0x5A:
		paramTypes<<bank<<bank<<item_id<<adress;break;
	case 0x5B:
		paramTypes<<bank<<bank<<bank<<bank<<materia_id<<byte<<byte<<byte;break;
	case 0x5C:
		paramTypes<<bank<<bank<<bank<<bank<<materia_id<<byte<<byte<<byte<<quantity;break;
	case 0x5D:
		paramTypes<<bank<<bank<<bank<<bank<<bank<<bank<<materia_id<<byte<<byte<<byte<<inconnu<<adress;break;
    case 0x5E:
        paramTypes<<byte<<byte<<shakeType<<xAmplitude<<xFrames<<yAmplitude<<yFrames;break;
	//case 0x5F:break;
	case 0x60:
		paramTypes<<field_id<<coord_x<<coord_y<<polygone_id<<direction;break;
	//case 0x61:case 0x62://TODO
	case 0x63:
		paramTypes<<bank<<bank<<vitesse2<<group_id<<byte;break;
	case 0x64:
		paramTypes<<bank<<bank<<coord_x<<coord_y;break;
	//case 0x65:case 0x67:break;
	case 0x66:case 0x68:
		paramTypes<<bank<<bank<<bank<<bank<<coord_x<<coord_y<<vitesse2;break;
	case 0x69:
		paramTypes<<boolean;break;
	case 0x6A:
		paramTypes<<bank<<bank<<sword<<sword<<byte;break;
	case 0x6B:
		paramTypes<<bank<<bank<<bank<<bank<<color<<vitesse<<byte<<byte;break;
	//case 0x6C:break;
	case 0x6D:
		paramTypes<<polygone_id<<boolean;break;
	case 0x6E:
		paramTypes<<bank<<bank<<adress;break;
	case 0x6F:
		paramTypes<<bank<<bank<<vitesse2<<party_id<<byte;break;
	case 0x70:
		paramTypes<<bank<<bank<<word;break;
	case 0x71:
		paramTypes<<boolean;break;
	case 0x72:
		paramTypes<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit;break;
	case 0x73:case 0x74:
		paramTypes<<bank<<bank<<party_id<<adress;break;
	case 0x75:
		paramTypes<<bank<<bank<<bank<<bank<<party_id<<adress<<adress<<adress<<adress;break;
	case 0x76:case 0x78:case 0x80:case 0x82:case 0x83:case 0x84:case 0x85:case 0x87:case 0x89:case 0x8B:case 0x8D:case 0x8F:case 0x91:case 0x93:case 0x9A:
		paramTypes<<bank<<bank<<adress<<byte;break;
	case 0x77:case 0x79:case 0x81:case 0x86:case 0x88:case 0x8A:case 0x8C:case 0x8E:case 0x90:case 0x92:case 0x94:case 0x9B:
		paramTypes<<bank<<bank<<adress<<word;break;
	case 0x7A:case 0x7B:case 0x7C:case 0x7D:case 0x95:case 0x96:case 0x97:case 0x98:case 0x99:
		paramTypes<<bank<<bank<<adress;break;
	case 0x7E:
		paramTypes<<boolean;break;
	case 0x7F:
		paramTypes<<bank<<bank<<byte;break;
	case 0x9C:
		paramTypes<<bank<<bank<<bank<<bank<<adress<<byte<<byte;break;
	//case 0x9D:case 0x9E:case 0x9F://TODO
	case 0xA0:
		paramTypes<<personnage_id;break;
	case 0xA1:
		paramTypes<<byte;break;
	case 0xA2:case 0xA3:case 0xAE:case 0xAF:
		paramTypes<<animation_id<<vitesse;break;
	case 0xA4:
		paramTypes<<boolean;break;
	case 0xA5:
		paramTypes<<bank<<bank<<bank<<bank<<coord_x<<coord_y<<coord_z<<polygone_id;break;
	case 0xA6:
		paramTypes<<bank<<bank<<bank<<bank<<coord_x<<coord_y<<polygone_id;break;
	case 0xA7:
		paramTypes<<bank<<bank<<bank<<bank<<coord_x<<coord_y<<coord_z;break;
	case 0xA8:case 0xA9:case 0xAD:
		paramTypes<<bank<<bank<<coord_x<<coord_y;break;
	case 0xAA:
		paramTypes<<group_id;break;
	case 0xAB:
		paramTypes<<group_id<<rotation<<vitesse;break;
	//case 0xAC:break;
	case 0xB0:case 0xB1:case 0xBB:case 0xBC:
		paramTypes<<animation_id<<byte<<byte<<vitesse;break;
	case 0xB2:case 0xBD:
		paramTypes<<bank<<bank<<vitesse2;break;
	case 0xB3:
		paramTypes<<bank<<bank<<direction;break;
	case 0xB4:case 0xB5:
		paramTypes<<bank<<bank<<direction<<byte<<vitesse<<inconnu;break;
	case 0xB6:
		paramTypes<<group_id;break;
	case 0xB7:case 0xB9:
		paramTypes<<bank<<bank<<group_id<<adress;break;
	case 0xB8:
		paramTypes<<bank<<bank<<group_id<<adress<<adress;break;
	case 0xBA:
		paramTypes<<animation_id<<vitesse;break;
	//case 0xBE:break;
	case 0xBF:
		paramTypes<<group_id;break;
	case 0xC0:
		paramTypes<<bank<<bank<<bank<<bank<<coord_x<<coord_y<<polygone_id<<word;break;
	case 0xC1:
		paramTypes<<bank<<bank<<bank<<bank<<group_id<<adress<<adress<<adress<<adress;break;
	case 0xC2:
		paramTypes<<bank<<bank<<bank<<bank<<coord_x<<coord_y<<coord_z<<polygone_id<<byte<<animation_id<<direction<<vitesse;break;
	case 0xC3:
		paramTypes<<bank<<bank<<bank<<bank<<byte<<coord_x<<coord_y<<coord_z<<vitesse2;break;
	//case 0xC4:break;
	case 0xC5:case 0xC6:
		paramTypes<<bank<<bank<<byte;break;
	case 0xC7:
		paramTypes<<boolean;break;
	case 0xC8:case 0xC9:case 0xCE:case 0xCF:
		paramTypes<<personnage_id;break;
	case 0xCA:
		paramTypes<<personnage_id<<personnage_id<<personnage_id;break;
	case 0xCB:case 0xCC:
		paramTypes<<personnage_id<<jump;break;
	case 0xCD:
		paramTypes<<boolean<<personnage_id;break;
	case 0xD0:
		paramTypes<<coord_x<<coord_y<<coord_z<<coord_x<<coord_y<<coord_z;break;
	case 0xD1:case 0xD2:
		paramTypes<<boolean;break;
	case 0xD3:
		paramTypes<<bank<<bank<<bank<<bank<<bank<<bank<<coord_x<<coord_y<<coord_z<<coord_x<<coord_y<<coord_z;break;
	case 0xD4:case 0xD5:
		paramTypes<<bank<<bank<<bank<<bank<<word<<word<<word<<byte;break;
	case 0xD6:case 0xD7:
		paramTypes<<bank<<bank<<word;break;
	case 0xD8:
		paramTypes<<field_id;break;
	//case 0xD9:break;
	case 0xDA:
		paramTypes<<bank<<bank<<bank<<bank<<bank<<bank<<akao<<word<<word<<word<<word<<word;break;
	case 0xDB:
		paramTypes<<boolean;break;
	case 0xDC:
		paramTypes<<animation_id<<vitesse<<byte;break;
	//case 0xDD:case 0xDE:break;
	case 0xDF:
		paramTypes<<bank<<bank<<bank<<bank<<bank<<bank<<byte<<byte<<byte<<byte<<byte<<byte<<byte;break;
	case 0xE0:case 0xE1:
		paramTypes<<bank<<bank<<parametre_id<<state_id;break;
	case 0xE2:case 0xE3:case 0xE4:
		paramTypes<<bank<<bank<<parametre_id;break;
	case 0xE5:case 0xE6:case 0xE7:
		paramTypes<<bank<<bank<<byte<<byte<<byte;break;
	case 0xE8:
		paramTypes<<bank<<bank<<bank<<bank<<byte<<byte<<byte<<byte;break;
	case 0xE9:
		paramTypes<<bank<<bank<<bank<<bank<<bank<<bank<<byte<<byte<<byte<<byte<<byte<<byte;break;
	case 0xEA:
		paramTypes<<bank<<bank<<bank<<bank<<bank<<bank<<byte<<byte<<byte<<byte<<byte<<byte;break;
	case 0xEB:case 0xEC:
		paramTypes<<byte<<byte<<byte<<byte;break;
	//case 0xED:case 0xEE:case 0xEF://TODO
	case 0xF0:case 0xF3:case 0xF4:case 0xF6:
		paramTypes<<music_id;break;
	case 0xF1:
		paramTypes<<bank<<bank<<sound_id<<byte;break;
	case 0xF2:
		paramTypes<<bank<<bank<<bank<<bank<<bank<<bank<<akao<<byte<<word<<word<<word<<word;break;
	case 0xF5:
		paramTypes<<boolean;break;
	//case 0xF7://TODO
	case 0xF8:
		paramTypes<<movie_id;break;
	//case 0xF9:break;
	case 0xFA:
		paramTypes<<bank<<bank<<adress;break;
	case 0xFB:
		paramTypes<<boolean;break;
	//case 0xFC://TODO
	case 0xFD:
		paramTypes<<music_id<<inconnu<<inconnu<<inconnu<<inconnu<<inconnu<<inconnu;break;
	case 0xFE:
		paramTypes<<bank<<bank<<adress;break;
	//case 0xFF:break;
	case 0x100:
		paramTypes<<label;break;
	}
	return paramTypes;
}

int ScriptEditorGenericList::paramSize(int type)
{
	switch (type) {
	case dword:
	case label:				return 32;
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

bool ScriptEditorGenericList::paramIsSigned(int type)
{
	switch (type) {
	case sword:
	case coord_x:
	case coord_y:
	case coord_z:	return true;
	default:		return false;
	}
}

QString ScriptEditorGenericList::paramName(int type)
{
	switch (type) {
	case dword:				return tr("Double long");
	case word:				return tr("Long");
	case sword:				return tr("Signed long");
	case coord_x:			return tr("X coordinate");
	case coord_y:			return tr("Y coordinate");
	case coord_z:			return tr("Z coordinate");
	case field_id:			return tr("Map");
	case tuto_id:			return tr("Tutorial");
	case personnage_id:		return tr("Character");
	case cd_id:				return tr("Disc");
	case minijeu_id:		return tr("Minigame");
	case byte:				return tr("Short");
	case vitesse:			return tr("Speed (8-bit)");
	case vitesse2:			return tr("Speed (16-bit)");
	case direction:			return tr("Direction");
	case polygone_id:		return tr("Triangle");
	case group_id:			return tr("Group");
	case script_id:			return tr("Script");
	case party_id:			return tr("Team member");
	case bank:				return tr("Bank");
	case adress:			return tr("Address");
	case priorite:			return tr("Priority");
	case bit: 				return tr("Flag");
	case jump: 				return tr("Jump (short)");
	case jump_l: 			return tr("Jump (long)");
	case operateur: 		return tr("Operator");
	case boolean: 			return tr("Boolean");
	case layer_id: 			return tr("Layer");
	case parametre_id: 		return tr("Parameter");
	case state_id: 			return tr("State");
	case window_id: 		return tr("Window");
	case window_w: 			return tr("Width");
	case window_h: 			return tr("Height");
	case window_var: 		return tr("Variable");
	case keys: 				return tr("Key(s)");
	case rotation: 			return tr("Rotation");
	case window_num: 		return tr("Display Type");
	case text_id: 			return tr("Text");
	case menu: 				return tr("Menu");
	case window_type: 		return tr("Window Type");
	case item_id: 			return tr("Item");
	case materia_id: 		return tr("Materia");
	case quantity: 			return tr("Quantity");
	case color: 			return tr("Color");
	case animation_id: 		return tr("Animation");
	case music_id: 			return tr("Music");
	case sound_id: 			return tr("Sound");
	case movie_id: 			return tr("Video");
    case label:				return tr("Label");
    case akao:				return tr("Sound operation");
    case shakeType:         return tr("Shake type");
    case xAmplitude:        return tr("X Amplitude");
    case xFrames:           return tr("X Frames");
    case yAmplitude:        return tr("Y Amplitude");
    case yFrames:           return tr("Y Frames");
	}
	return tr("???");
}
