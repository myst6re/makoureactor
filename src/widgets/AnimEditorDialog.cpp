/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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
#include "AnimEditorDialog.h"
#include "core/Config.h"
#include "Data.h"

AnimEditorDialog::AnimEditorDialog(int animID, QWidget *parent) :
	QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint)
{
	setWindowTitle(tr("Animation Selector"));
	QWidget *modelWidget;
	if (Config::value("OpenGL", true).toBool()) {
		fieldModel = new FieldModel();
		fieldModel->setFixedSize(304, 214);
		modelWidget = fieldModel;
	} else {
		fieldModel = nullptr;
		modelWidget = new QWidget(this);
	}

	aList = new QListWidget(this);
	aList->setUniformItemSizes(true);
//	aList->addItems(Data::currentAnimNames->value(Data::currentModelID));

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(aList, 0, 0);
	layout->addWidget(modelWidget, 0, 1);
	layout->addWidget(buttonBox, 1, 0, 1, 2);

	if (Data::currentModelID != -1) {
		connect(aList, SIGNAL(currentRowChanged(int)), SLOT(changeModelAnimation()));
	}
	aList->setCurrentRow(animID);
	connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
}

void AnimEditorDialog::changeModelAnimation()
{
//	if (fieldModel)
//		fieldModel->load(Data::currentHrcNames->value(Data::currentModelID), Data::currentAnimNames->value(Data::currentModelID).value(chosenAnimation()), true);
}

int AnimEditorDialog::chosenAnimation()
{
	return aList->currentRow();
}
