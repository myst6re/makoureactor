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
#ifndef DEF_SCRIPTEDITORGENERICLIST
#define DEF_SCRIPTEDITORGENERICLIST

#include <QtGui>
#include "ScriptEditorView.h"

class ScriptEditorGenericList : public ScriptEditorView
{
	Q_OBJECT

public:
	enum paramType {
		inconnu=0, field_id, tuto_id, group_id, script_id, /*5*/personnage_id, party_id, cd_id, minijeu_id, polygone_id,
		/*10*/layer_id, parametre_id, state_id, window_id, text_id, /*15*/item_id, materia_id, animation_id, music_id, sound_id, /*20*/movie_id,
		operateur, keys, color, coord_x, coord_y, coord_z, window_w, window_h, window_num, window_type, window_var, direction, vitesse, vitesse2,
		priorite, menu, jump, jump_l, rotation, quantity,
		bank, adress, byte, word, sword, bit, boolean, label
	};

	ScriptEditorGenericList(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent=0);
	virtual ~ScriptEditorGenericList();

	virtual Opcode *opcode();
public slots:
	virtual void setOpcode(Opcode *opcode);
private slots:
	void addParam();
	void delLastRow();

private:
	void build();
	void addRow(int value, int minValue, int maxValue, int type);
	void fillModel();
	QByteArray parseModel(bool *isLabel);
	QList<int> paramTypes(int id);
	int paramSize(int type);
	bool paramIsSigned(int type);
	QString paramName(int type);

	QPushButton *addButton, *delButton;
	QTableView *tableView;
	QStandardItemModel *model;
};

#endif // DEF_SCRIPTEDITORGENERICLIST
