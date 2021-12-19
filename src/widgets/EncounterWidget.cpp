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
#include "EncounterWidget.h"

EncounterWidget::EncounterWidget(EncounterFile *data, QWidget *parent) :
	QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint), data(data)
{
	setWindowTitle(tr("Encounters"));

	group1 = new EncounterTableWidget(tr("Encounters 1"), this);
	group2 = new EncounterTableWidget(tr("Encounters 2"), this);

	QLabel *helpIcon = new QLabel(this);
	helpIcon->setPixmap(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation).pixmap(32));
	QLabel *help = new QLabel(tr("<b>Encounters:</b> There are two groups of independent encounters, by default it is the group 1 that is active, but the opcode BTLTB (in scripts) can modify to the group 2.<br/>"
								 "<b>Battle Rate:</b> The lower the percentage, the higher the fighting will be frequent."), this);
	help->setTextFormat(Qt::RichText);
	help->setWordWrap(true);
	QHBoxLayout *helpLayout = new QHBoxLayout;
	helpLayout->addWidget(helpIcon);
	helpLayout->addWidget(help, 1);

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(group1, 0, 0);
	layout->addWidget(group2, 0, 1);
	layout->addLayout(helpLayout, 1, 0, 1, 2, Qt::AlignTop);
	layout->addWidget(buttonBox, 2, 0, 1, 2);

	fillGroup(group1, data->encounterTable(EncounterFile::Table1));
	fillGroup(group2, data->encounterTable(EncounterFile::Table2));

	connect(buttonBox, &QDialogButtonBox::accepted, this, &EncounterWidget::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &EncounterWidget::reject);
}

void EncounterWidget::fillGroup(EncounterTableWidget *group, const EncounterTable &encounterTable)
{
	group->setChecked(encounterTable.enabled & 1);
	group->setRateValue(encounterTable.rate);

	QList<int> probas, ids;
	for (int i=0; i<6; ++i) {
		probas.append(PROBABILITY(encounterTable.enc_standard[i]));
		ids.append(BATTLE_ID(encounterTable.enc_standard[i]));
	}

	for (int i=0; i<4; ++i) {
		probas.append(PROBABILITY(encounterTable.enc_special[i]));
		ids.append(BATTLE_ID(encounterTable.enc_special[i]));
	}

	group->setBattleProbasValues(probas);
	group->setBattleIdsValues(ids);
}

EncounterTable EncounterWidget::saveGroup(EncounterTableWidget *group, EncounterTable encounterTable)
{
	encounterTable.enabled = (encounterTable.enabled & 0xFE) | quint8(group->isChecked());
	encounterTable.rate = quint8(group->rateValue());

	QList<int> battleProbas = group->battleProbasValues(), battleIds = group->battleIdsValues();

	for (int i = 0; i < 6; ++i) {
		encounterTable.enc_standard[i] = quint16((battleProbas.at(i) << 10) | battleIds.at(i));
	}

	for (int i = 0; i < 4; ++i) {
		encounterTable.enc_special[i] = quint16((battleProbas.at(6 + i) << 10) | battleIds.at(6 + i));
	}

	return encounterTable;
}

void EncounterWidget::accept()
{
	data->setEncounterTable(EncounterFile::Table1, saveGroup(group1, data->encounterTable(EncounterFile::Table1)));
	data->setEncounterTable(EncounterFile::Table2, saveGroup(group2, data->encounterTable(EncounterFile::Table2)));

	QDialog::accept();
}
