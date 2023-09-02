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
#include "ScriptEditorGenericList.h"
#include "Delegate.h"

ScriptEditorGenericList::ScriptEditorGenericList(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent),
	addButton(nullptr), delButton(nullptr), tableView(nullptr), model(nullptr)
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
	delegate->setScriptsAndTexts(scriptsAndTexts());
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

	connect(model, &QStandardItemModel::itemChanged, this, &ScriptEditorGenericList::opcodeChanged);
	connect(addButton, &QPushButton::clicked, this, &ScriptEditorGenericList::addParam);
	connect(delButton, &QPushButton::clicked, this, &ScriptEditorGenericList::delLastRow);
}

Opcode ScriptEditorGenericList::buildOpcode()
{
	bool isLabel;
	QByteArray newOpcode = parseModel(&isLabel);
	if (newOpcode.isEmpty()) {
		return opcode();
	}

	if (isLabel) {
		quint16 label;
		memcpy(&label, newOpcode.constData() + 1, 2);
		OpcodeLABEL opcodeLABEL;
		opcodeLABEL._label = label;
		ScriptEditorView::setOpcode(opcodeLABEL);
	} else {
		ScriptEditorView::setOpcode(Opcode(newOpcode.constData(), newOpcode.size()));
	}

	return opcode();
}

void ScriptEditorGenericList::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);
	fillModel();
}

QByteArray ScriptEditorGenericList::parseModel(bool *isLabel)
{
	*isLabel = false;

	QByteArray newOpcode;
	quint8 byte, length, start;

	*isLabel = opcode().id() == OpcodeKey::LABEL;
	byte = opcode().id() & 0xFF;
	newOpcode.append(char(byte));
	// Compute opcode length
	length = 0;
	start = 1;
	
	if (byte == 0x0F) { // SPECIAL
		quint8 byte2 = quint8(opcode().op().opcodeSPECIAL.subKey);
		newOpcode.append(char(byte2));
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
		quint8 byte3 = quint8(opcode().op().opcodeKAWAI.subKey);
		length = quint8(model->rowCount() + 3);
		newOpcode.append(char(length));
		newOpcode.append(char(byte3));
		for (int i = 0; i < model->rowCount(); ++i) {
			newOpcode.append(char(model->item(i, 1)->text().toUInt()));
		}
		return newOpcode;
	}

	length += Opcode::length[opcode().id()];
	newOpcode.append(QByteArray(length - start, '\x0'));
	QList<int> paramTypes = this->paramTypes(opcode().id());

	if (!paramTypes.isEmpty()) {
		int cur = 8;
		for (quint8 i = 0; i < paramTypes.size(); ++i) {
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
				newOpcode[startBA] = char(quint8(newOpcode.at(startBA)) | (value << (8 - paramSize - startLocal)));
			} else if (paramSize == 8) {
				newOpcode[startBA] = char(value & 0xFF);
			} else {
				for (int j=0; j<sizeBA; j++) {
					newOpcode[startBA + j] = char((value >> (j * 8)) & 0xFF);
				}
			}

			cur += paramSize;
		}
	} else {
		for (quint8 i = start; i < length; ++i) {
			newOpcode[i] = char(model->item(i-start, 1)->text().toUInt());
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
	standardItem->setData(maxValue, Qt::UserRole + 1);
	standardItem->setData(type, Qt::UserRole + 2);
	items.append(standardItem);
	model->appendRow(items);
}

void ScriptEditorGenericList::fillModel()
{
	addButton->hide();
	delButton->hide();
	model->clear();

	int paramSize, paramType, cur = 0, maxValue, minValue, value=0;
	QList<int> paramTypes = this->paramTypes(opcode().id());
	
	if (opcode().id() == OpcodeKey::LABEL) {
		addRow(int(opcode().op().opcodeLABEL._label), 0, int(pow(2, 15)) - 1, label);
	} else if (paramTypes.isEmpty()) {
		int start = 0;
		if (opcode().id() == OpcodeKey::SPECIAL) {
			start = 1;
		}
		if (opcode().id() == OpcodeKey::KAWAI) {
			start = 2;
			addButton->show();
			delButton->show();
		}
		const QByteArray params = opcode().params();
		for (int i=start; i<params.size(); ++i) {
			addRow(quint8(params.at(i)), 0, 255, inconnu);
		}
	} else {
		for (quint8 i=0; i<paramTypes.size(); ++i) {
			paramType = paramTypes.at(i);
			paramSize = this->paramSize(paramType);
			value = opcode().subParam(cur, paramSize);
			// qDebug() << value;
			if (paramIsSigned(paramType)) {
				maxValue = int(pow(2, paramSize-1)) - 1;
				minValue = -maxValue-1;
				if (value>maxValue) {
					value -= int(pow(2, paramSize));
				}
			} else {
				maxValue = int(pow(2, paramSize)) - 1;
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
	//case OpcodeKey::RET:break;
	case OpcodeKey::REQ:case OpcodeKey::REQSW:case OpcodeKey::REQEW:
		paramTypes<<group_id<<priorite<<script_id;break;
	case OpcodeKey::PREQ:case OpcodeKey::PRQSW:case OpcodeKey::PRQEW:
		paramTypes<<party_id<<priorite<<script_id;break;
	case OpcodeKey::RETTO:
		paramTypes<<priorite<<script_id;break;
	case OpcodeKey::JOIN:
		paramTypes<<vitesse;break;
	case OpcodeKey::SPLIT:
		paramTypes<<bank<<bank<<bank<<bank<<bank<<bank<<coord_x<<coord_y<<direction<<coord_x<<coord_y<<direction<<vitesse;break;
	case OpcodeKey::SPTYE:case OpcodeKey::GTPYE:
		paramTypes<<bank<<bank<<bank<<bank<<party_id<<party_id<<party_id;break;
	//case 0x0C:case 0x0D:break;
	case OpcodeKey::DSKCG:
		paramTypes<<cd_id;break;
	//case OpcodeKey::SPECIAL:return;
	case OpcodeKey::JMPF:case OpcodeKey::JMPB:
		paramTypes<<jump;break;
	case OpcodeKey::JMPFL:case OpcodeKey::JMPBL:
		paramTypes<<jump_l;break;
	case OpcodeKey::IFUB:
		paramTypes<<bank<<bank<<byte<<byte<<operateur<<jump;break;
	case OpcodeKey::IFUBL:
		paramTypes<<bank<<bank<<byte<<byte<<operateur<<jump_l;break;
	case OpcodeKey::IFSW:
		paramTypes<<bank<<bank<<sword<<sword<<operateur<<jump;break;
	case OpcodeKey::IFSWL:
		paramTypes<<bank<<bank<<sword<<sword<<operateur<<jump_l;break;
	case OpcodeKey::IFUW:
		paramTypes<<bank<<bank<<word<<word<<operateur<<jump;break;
	case OpcodeKey::IFUWL:
		paramTypes<<bank<<bank<<word<<word<<operateur<<jump_l;break;
	case OpcodeKey::Unused1A:
		paramTypes<<word<<word<<dword<<byte;break;
	case OpcodeKey::Unused1B:
		paramTypes<<jump_l;break;
	case OpcodeKey::Unused1C:
		paramTypes<<dword<<byte;break;
	//case 0x1D:case 0x1E:case 0x1F:break;
	case OpcodeKey::MINIGAME:
		paramTypes<<field_id<<coord_x<<coord_y<<polygone_id<<byte<<minijeu_id;break;
	case OpcodeKey::TUTOR:
		paramTypes<<tuto_id;break;
	case OpcodeKey::BTMD2:
		paramTypes<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit;break;
	case OpcodeKey::BTRLD:
		paramTypes<<bank<<bank<<byte;break;
	case OpcodeKey::WAIT:
		paramTypes<<word;break;
	case OpcodeKey::NFADE:
		paramTypes<<bank<<bank<<bank<<bank<<byte<<color<<vitesse2;break;
	case OpcodeKey::BLINK:
		paramTypes<<boolean;break;
	case OpcodeKey::BGMOVIE:
		paramTypes<<boolean;break;
	//case OpcodeKey::KAWAI:
	//case OpcodeKey::KAWIW:break;
	case OpcodeKey::PMOVA:
		paramTypes<<party_id;break;
	case OpcodeKey::SLIP:
		paramTypes<<boolean;break;
	case OpcodeKey::BGPDH:
		paramTypes<<bank<<bank<<layer_id<<coord_z;break;
	case OpcodeKey::BGSCR:
		paramTypes<<bank<<bank<<layer_id<<sword<<sword;break;
	case OpcodeKey::WCLS:
		paramTypes<<window_id;break;
	case OpcodeKey::WSIZW:
		paramTypes<<window_id<<coord_x<<coord_y<<window_w<<window_h;break;
	case OpcodeKey::IFKEY:case OpcodeKey::IFKEYON:case OpcodeKey::IFKEYOFF:
		paramTypes<<keys<<jump;break;
	case OpcodeKey::UC:
		paramTypes<<boolean;break;
	case OpcodeKey::PDIRA:
		paramTypes<<party_id;break;
	case OpcodeKey::PTURA:
		paramTypes<<party_id<<vitesse<<rotation;break;
	case OpcodeKey::WSPCL:
		paramTypes<<window_id<<window_num<<byte<<byte;break;
	case OpcodeKey::WNUMB:
		paramTypes<<bank<<bank<<window_id<<word<<word<<byte;break;
	case OpcodeKey::STTIM:
		paramTypes<<bank<<bank<<bank<<bank<<byte<<byte<<byte;break;
	case OpcodeKey::GOLDu:case OpcodeKey::GOLDd:
		paramTypes<<bank<<bank<<word<<word;break;
	case OpcodeKey::CHGLD:
		paramTypes<<bank<<bank<<address<<address;break;
	//case OpcodeKey::HMPMAX1:case OpcodeKey::HMPMAX2:case OpcodeKey::MHMMX:case OpcodeKey::HMPMAX3:break;
	case OpcodeKey::MESSAGE:
		paramTypes<<window_id<<text_id;break;
	case OpcodeKey::MPARA:
		paramTypes<<bank<<bank<<window_id<<window_var<<byte;break;
	case OpcodeKey::MPRA2:
		paramTypes<<bank<<bank<<window_id<<window_var<<word;break;
	case OpcodeKey::MPNAM:
		paramTypes<<text_id;break;
	//case OpcodeKey::Unknown9:case OpcodeKey::Unknown10:break;
	case OpcodeKey::MPu:case OpcodeKey::MPd:
		paramTypes<<bank<<bank<<party_id<<word;break;
	case OpcodeKey::ASK:
		paramTypes<<bank<<bank<<window_id<<text_id<<byte<<byte<<address;break;
	case OpcodeKey::MENU:
		paramTypes<<bank<<bank<<menu<<byte;break;
	case OpcodeKey::MENU2:
		paramTypes<<boolean;break;
	case OpcodeKey::BTLTB:
		paramTypes<<byte;break;
	//case OpcodeKey::Unknown11:case OpcodeKey::Unknown12:break;
	case OpcodeKey::HPu:case OpcodeKey::HPd:
		paramTypes<<bank<<bank<<party_id<<word;break;
	case OpcodeKey::WINDOW:
		paramTypes<<window_id<<coord_x<<coord_y<<window_w<<window_h;break;
	case OpcodeKey::WMOVE:
		paramTypes<<window_id<<coord_x<<coord_y;break;
	case OpcodeKey::WMODE:
		paramTypes<<window_id<<window_type<<boolean;break;
	case OpcodeKey::WREST:case OpcodeKey::WCLSE:
		paramTypes<<window_id;break;
	case OpcodeKey::WROW:
		paramTypes<<window_id<<byte;break;
	case OpcodeKey::GWCOL:
		paramTypes<<bank<<bank<<bank<<bank<<address<<address<<address<<address;break;
	case OpcodeKey::SWCOL:
		paramTypes<<bank<<bank<<bank<<bank<<byte<<color;break;
	case OpcodeKey::STITM:case OpcodeKey::DLITM:
		paramTypes<<bank<<bank<<item_id<<quantity;break;
	case OpcodeKey::CKITM:
		paramTypes<<bank<<bank<<item_id<<address;break;
	case OpcodeKey::SMTRA:
		paramTypes<<bank<<bank<<bank<<bank<<materia_id<<byte<<byte<<byte;break;
	case OpcodeKey::DMTRA:
		paramTypes<<bank<<bank<<bank<<bank<<materia_id<<byte<<byte<<byte<<quantity;break;
	case OpcodeKey::CMTRA:
		paramTypes<<bank<<bank<<bank<<bank<<bank<<bank<<byte<<byte<<byte<<byte<<materia_id<<address;break;
	case OpcodeKey::SHAKE:
		paramTypes<<bank<<bank<<bank<<bank<<shakeType<<xAmplitude<<xFrames<<yAmplitude<<yFrames;break;
	//case OpcodeKey::NOP:break;
	case OpcodeKey::MAPJUMP:
		paramTypes<<field_id<<coord_x<<coord_y<<polygone_id<<direction;break;
	//case OpcodeKey::SCRLO:// TODO
	case OpcodeKey::SCRLC:
		paramTypes<<bank<<bank<<vitesse2<<byte;break;
	case OpcodeKey::SCRLA:
		paramTypes<<bank<<bank<<vitesse2<<group_id<<byte;break;
	case OpcodeKey::SCR2D:
		paramTypes<<bank<<bank<<coord_x<<coord_y;break;
	//case OpcodeKey::SCRCC:case OpcodeKey::SCRLW:break;
	case OpcodeKey::SCR2DC:case OpcodeKey::SCR2DL:
		paramTypes<<bank<<bank<<bank<<bank<<coord_x<<coord_y<<vitesse2;break;
	case OpcodeKey::MPDSP:
		paramTypes<<boolean;break;
	case OpcodeKey::VWOFT:
		paramTypes<<bank<<bank<<sword<<sword<<byte;break;
	case OpcodeKey::FADE:
		paramTypes<<bank<<bank<<bank<<bank<<color<<vitesse<<byte<<byte;break;
	//case OpcodeKey::FADEW:break;
	case OpcodeKey::IDLCK:
		paramTypes<<polygone_id<<boolean;break;
	case OpcodeKey::LSTMP:
		paramTypes<<bank<<bank<<address;break;
	case OpcodeKey::SCRLP:
		paramTypes<<bank<<bank<<vitesse2<<party_id<<byte;break;
	case OpcodeKey::BATTLE:
		paramTypes<<bank<<bank<<word;break;
	case OpcodeKey::BTLON:
		paramTypes<<boolean;break;
	case OpcodeKey::BTLMD:
		paramTypes<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit<<bit;break;
	case OpcodeKey::PGTDR:case OpcodeKey::GETPC:
		paramTypes<<bank<<bank<<party_id<<address;break;
	case OpcodeKey::PXYZI:
		paramTypes<<bank<<bank<<bank<<bank<<party_id<<address<<address<<address<<address;break;
	case 0x76:case 0x78:case 0x80:case 0x82:case 0x83:case 0x84:case 0x85:case 0x87:case 0x89:case 0x8B:case 0x8D:case 0x8F:case 0x91:case 0x93:case 0x9A:
		paramTypes<<bank<<bank<<address<<byte;break;
	case 0x77:case 0x79:case 0x81:case 0x86:case 0x88:case 0x8A:case 0x8C:case 0x8E:case 0x90:case 0x92:case 0x94:case 0x9B:
		paramTypes<<bank<<bank<<address<<word;break;
	case 0x7A:case 0x7B:case 0x7C:case 0x7D:case 0x95:case 0x96:case 0x97:case 0x98:case 0x99:
		paramTypes<<bank<<bank<<address;break;
	case 0x7E:
		paramTypes<<boolean;break;
	case 0x7F:
		paramTypes<<bank<<bank<<byte;break;
	case 0x9C:
		paramTypes<<bank<<bank<<bank<<bank<<address<<byte<<byte;break;
	//case 0x9D:case 0x9E:case 0x9F://TODO
	case OpcodeKey::PC:
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
		paramTypes<<bank<<bank<<group_id<<address;break;
	case 0xB8:
		paramTypes<<bank<<bank<<group_id<<address<<address;break;
	case 0xBA:
		paramTypes<<animation_id<<vitesse;break;
	//case 0xBE:break;
	case 0xBF:
		paramTypes<<group_id;break;
	case 0xC0:
		paramTypes<<bank<<bank<<bank<<bank<<coord_x<<coord_y<<polygone_id<<word;break;
	case 0xC1:
		paramTypes<<bank<<bank<<bank<<bank<<group_id<<address<<address<<address<<address;break;
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
		paramTypes<<bank<<bank<<address;break;
	case 0xFB:
		paramTypes<<boolean;break;
	//case 0xFC://TODO
	case 0xFD:
		paramTypes<<music_id<<inconnu<<inconnu<<inconnu<<inconnu<<inconnu<<inconnu;break;
	case 0xFE:
		paramTypes<<bank<<bank<<address;break;
	//case 0xFF:break;
	case OpcodeKey::LABEL:
		paramTypes<<label;break;
	}
	return paramTypes;
}

int ScriptEditorGenericList::paramSize(int type)
{
	switch (type) {
	case dword:
		return 32;
	case color:
		return 24;
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
	case label:
	case field_id:
		return 16;
	case script_id:
		return 5;
	case bank:
		return 4;
	case priorite:
		return 3;
	case bit:
		return 1;
	}
	return 8;
}

bool ScriptEditorGenericList::paramIsSigned(int type)
{
	switch (type) {
	case sword:
	case coord_x:
	case coord_y:
	case coord_z:
		return true;
	default:
		return false;
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
	case address:			return tr("Address");
	case priorite:			return tr("Priority");
	case bit:				return tr("Flag");
	case jump:				return tr("Jump (short)");
	case jump_l:			return tr("Jump (long)");
	case operateur:			return tr("Operator");
	case boolean:			return tr("Boolean");
	case layer_id:			return tr("Layer");
	case parametre_id:		return tr("Parameter");
	case state_id:			return tr("State");
	case window_id:			return tr("Window");
	case window_w:			return tr("Width");
	case window_h:			return tr("Height");
	case window_var:		return tr("Variable");
	case keys:				return tr("Key(s)");
	case rotation:			return tr("Rotation");
	case window_num:		return tr("Display Type");
	case text_id:			return tr("Text");
	case menu:				return tr("Menu");
	case window_type:		return tr("Window Type");
	case item_id:			return tr("Item");
	case materia_id:		return tr("Materia");
	case quantity:			return tr("Quantity");
	case color:				return tr("Color");
	case animation_id:		return tr("Animation");
	case music_id:			return tr("Music");
	case sound_id:			return tr("Sound");
	case movie_id:			return tr("Video");
	case label:				return tr("Label");
	case akao:				return tr("Sound operation");
	case shakeType:			return tr("Shake type");
	case xAmplitude:		return tr("X Amplitude");
	case xFrames:			return tr("X Frames");
	case yAmplitude:		return tr("Y Amplitude");
	case yFrames:			return tr("Y Frames");
	}
	return tr("???");
}

ScriptEditorNoParameterPage::ScriptEditorNoParameterPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorNoParameterPage::build()
{
}

Opcode ScriptEditorNoParameterPage::buildOpcode()
{
	return opcode();
}

ScriptEditorBooleanPage::ScriptEditorBooleanPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent),
	_boolean(nullptr)
{
}

void ScriptEditorBooleanPage::build()
{
	_boolean = new QComboBox(this);
	_boolean->addItems(QStringList() << QString() << QString());

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(_boolean, 0, 0);
	layout->setRowStretch(1, 1);
	layout->setContentsMargins(QMargins());

	connect(_boolean, &QComboBox::currentIndexChanged, this, &ScriptEditorBooleanPage::opcodeChanged);
}

Opcode ScriptEditorBooleanPage::buildOpcode()
{
	bool disabled = _boolean->currentIndex() != 0;

	switch (opcode().id()) {
	case OpcodeKey::LINON:
		opcode().op().opcodeLINON.enabled = !disabled;
		break;
	case OpcodeKey::MENU2:
		opcode().op().opcodeMENU2.disabled = disabled;
		break;
	case OpcodeKey::VISI:
		opcode().op().opcodeVISI.show = !disabled;
		break;
	case OpcodeKey::FCFIX:
		opcode().op().opcodeFCFIX.disabled = disabled;
		break;
	case OpcodeKey::BLINK:
		opcode().op().opcodeBLINK.closed = disabled;
		break;
	case OpcodeKey::TLKON:
		opcode().op().opcodeTLKON.disabled = disabled;
		break;
	case OpcodeKey::SOLID:
		opcode().op().opcodeSOLID.disabled = disabled;
		break;
	case OpcodeKey::SLIP:
		opcode().op().opcodeSLIP.disabled = disabled;
		break;
	case OpcodeKey::BGMOVIE:
		opcode().op().opcodeBGMOVIE.disabled = disabled;
		break;
	case OpcodeKey::BTLON:
		opcode().op().opcodeBTLON.disabled = disabled;
		break;
	case OpcodeKey::MPJPO:
		opcode().op().opcodeMPJPO.disabled = disabled;
		break;
	case OpcodeKey::UC:
		opcode().op().opcodeUC.disabled = disabled;
		break;
	case OpcodeKey::SPECIAL:
		switch (OpcodeSpecialKey(opcode().subKey())) {
		case OpcodeSpecialKey::ARROW:
			opcode().op().opcodeSPECIALARROW.disabled = disabled;
			break;
		case OpcodeSpecialKey::BTLCK:
			opcode().op().opcodeSPECIALBTLCK.lock = disabled;
			break;
		case OpcodeSpecialKey::MVLCK:
			opcode().op().opcodeSPECIALMVLCK.lock = disabled;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	
	return opcode();
}

void ScriptEditorBooleanPage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);
	
	switch (opcode.id()) {
	case OpcodeKey::LINON:
		setText(tr("Enable"), tr("Disable"));
		_boolean->setCurrentIndex(!opcode.op().opcodeLINON.enabled);
		break;
	case OpcodeKey::MENU2:
		setText(tr("Enable"), tr("Disable"));
		_boolean->setCurrentIndex(opcode.op().opcodeMENU2.disabled);
		break;
	case OpcodeKey::VISI:
		setText(tr("Show"), tr("Hide"));
		_boolean->setCurrentIndex(!opcode.op().opcodeVISI.show);
		break;
	case OpcodeKey::FCFIX:
		setText(tr("Enable"), tr("Disable"));
		_boolean->setCurrentIndex(opcode.op().opcodeFCFIX.disabled);
		break;
	case OpcodeKey::BLINK:
		setText(tr("Enable"), tr("Disable"));
		_boolean->setCurrentIndex(opcode.op().opcodeBLINK.closed);
		break;
	case OpcodeKey::TLKON:
		setText(tr("Activate"), tr("Deactivate"));
		_boolean->setCurrentIndex(opcode.op().opcodeTLKON.disabled);
		break;
	case OpcodeKey::SOLID:
		setText(tr("Activate"), tr("Deactivate"));
		_boolean->setCurrentIndex(opcode.op().opcodeSOLID.disabled);
		break;
	case OpcodeKey::SLIP:
		setText(tr("ON"), tr("OFF"));
		_boolean->setCurrentIndex(opcode.op().opcodeSLIP.disabled);
		break;
	case OpcodeKey::BGMOVIE:
		setText(tr("ON"), tr("OFF"));
		_boolean->setCurrentIndex(opcode.op().opcodeBGMOVIE.disabled);
		break;
	case OpcodeKey::BTLON:
		setText(tr("Activate"), tr("Deactivate"));
		_boolean->setCurrentIndex(opcode.op().opcodeBTLON.disabled);
		break;
	case OpcodeKey::MPJPO:
		setText(tr("Activate"), tr("Deactivate"));
		_boolean->setCurrentIndex(opcode.op().opcodeMPJPO.disabled);
		break;
	case OpcodeKey::UC:
		setText(tr("Activate"), tr("Deactivate"));
		_boolean->setCurrentIndex(opcode.op().opcodeUC.disabled);
		break;
	case OpcodeKey::SPECIAL:
		switch (OpcodeSpecialKey(opcode.subKey())) {
		case OpcodeSpecialKey::ARROW:
			setText(tr("Display"), tr("Hide"));
			_boolean->setCurrentIndex(opcode.op().opcodeSPECIALARROW.disabled);
			break;
		case OpcodeSpecialKey::BTLCK:
			setText(tr("Activate"), tr("Deactivate"));
			_boolean->setCurrentIndex(opcode.op().opcodeSPECIALBTLCK.lock);
			break;
		case OpcodeSpecialKey::MVLCK:
			setText(tr("Activate"), tr("Deactivate"));
			_boolean->setCurrentIndex(opcode.op().opcodeSPECIALMVLCK.lock);
			break;
		default:
			break;
		}
		
		break;
	default:
		break;
	}
}

void ScriptEditorBooleanPage::setText(const QString &choice1, const QString &choice2)
{
	_boolean->setItemText(0, choice1);
	_boolean->setItemText(1, choice2);
}

ScriptEditorOneVarOrValue::ScriptEditorOneVarOrValue(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script,
                                                                 int opcodeID, QWidget *parent) :
    ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorOneVarOrValue::build()
{
	_label = new QLabel(this);
	_varOrValue = new VarOrValueWidget(this);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(_label, 0, 0);
	layout->addWidget(_varOrValue, 0, 1);
	layout->setRowStretch(1, 1);
	layout->setColumnStretch(2, 1);
	layout->setContentsMargins(QMargins());

	connect(_varOrValue, SIGNAL(changed()), SIGNAL(opcodeChanged()));
}

Opcode ScriptEditorOneVarOrValue::buildOpcode()
{
	quint8 bank = 0;
	int value;
	
	if (_varOrValue->isValue()) {
		value = _varOrValue->value();
	} else {
		quint8 address;
		_varOrValue->var(bank, address);
		value = address;
	}

	switch (opcode().id()) {
	case OpcodeKey::SPECIAL:
		switch (OpcodeSpecialKey(opcode().subKey())) {
		case OpcodeSpecialKey::GMSPD:
			opcode().op().opcodeSPECIALGMSPD.banks = BANK(0, bank);
			opcode().op().opcodeSPECIALGMSPD.varSpeed = quint8(value);
			break;
		case OpcodeSpecialKey::SMSPD:
			opcode().op().opcodeSPECIALSMSPD.banks = BANK(0, bank);
			opcode().op().opcodeSPECIALSMSPD.speed = quint8(value);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}

	return opcode();
}

void ScriptEditorOneVarOrValue::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	VarOrValueWidget::Size size = VarOrValueWidget::Long;
	bool onlyVar = false;
	qint8 bank = opcode.bank2();
	int valueOrAddress = 0;

	switch (opcode.id()) {
	case OpcodeKey::SPECIAL:
		switch (OpcodeSpecialKey(opcode.subKey())) {
		case OpcodeSpecialKey::GMSPD:
			size = VarOrValueWidget::Short;
			valueOrAddress = opcode.op().opcodeSPECIALGMSPD.varSpeed;
			break;
		case OpcodeSpecialKey::SMSPD:
			size = VarOrValueWidget::Short;
			valueOrAddress = opcode.op().opcodeSPECIALSMSPD.speed;
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}

	if (size == VarOrValueWidget::Short) {
		_varOrValue->setShort();
	} else {
		_varOrValue->setLong();
	}
	_varOrValue->setOnlyVar(onlyVar);
	_varOrValue->setVarOrValue(quint8(bank), valueOrAddress);
}
