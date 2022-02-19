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
#include "KeyEditorDialog.h"
#include "Data.h"

KeyEditorDialog::KeyEditorDialog(quint16 value, QWidget *parent)
	: QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint)
{
	setWindowTitle(tr("Keys"));
	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	QVBoxLayout *layout = new QVBoxLayout(this);

	for (int i = 0; i < Data::key_names.size(); ++i) {
		QCheckBox *checkBox = new QCheckBox(Data::key_names.at(i), this);
		checkBox->setCheckState(((value >> i) & 1) ? Qt::Checked : Qt::Unchecked);
		layout->addWidget(checkBox);
	}
	layout->addWidget(buttonBox);
	
	connect(buttonBox, &QDialogButtonBox::accepted, this, &KeyEditorDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &KeyEditorDialog::reject);
}

int KeyEditorDialog::keys() const
{
	QList<QCheckBox *> all = this->findChildren<QCheckBox *>();
	quint16 result = 0;
	
	for (int i=0; i<all.size(); ++i)
	{
		if (all.at(i)->checkState() == Qt::Checked)
		{
			result |= 1 << i;
		}
	}
	return result;
}
