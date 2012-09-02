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
#include "MiscWidget.h"

MiscWidget::MiscWidget(InfFile *data, Field *field, QWidget *parent) :
	QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint),
	data(data), field(field)
{
	setWindowTitle(tr("Divers"));

	mapName = new QLineEdit(this);
	mapName->setMaxLength(8);
	mapAuthor = new QLineEdit(this);
	mapAuthor->setMaxLength(7);
	mapScale = new QSpinBox(this);
	mapScale->setRange(0, 65535);

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Nom écran :")), 0, 0);
	layout->addWidget(mapName, 0, 1);
	layout->addWidget(new QLabel(tr("Auteur :")), 1, 0);
	layout->addWidget(mapAuthor, 1, 1);
	layout->addWidget(new QLabel(tr("Zoom écran :")), 2, 0);
	layout->addWidget(mapScale, 2, 1);
	layout->addWidget(buttonBox, 3, 0, 1, 2, Qt::AlignRight);

	connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));

	fill();
}

void MiscWidget::fill()
{
	mapName->setText(data->mapName());
	mapAuthor->setText(field->scriptsAndTexts()->author());
	mapScale->setValue(field->scriptsAndTexts()->scale());
}

void MiscWidget::accept()
{
	data->setMapName(mapName->text());
	field->scriptsAndTexts()->setAuthor(mapAuthor->text());
	field->scriptsAndTexts()->setScale(mapScale->value());

	QDialog::accept();
}
