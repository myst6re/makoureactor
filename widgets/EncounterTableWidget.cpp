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
#include "EncounterTableWidget.h"

EncounterTableWidget::EncounterTableWidget(const QString &title, QWidget *parent) :
	QGroupBox(title, parent)
{
	setCheckable(true);
	mainRate = new QSlider(Qt::Horizontal);
	rateLabel = new QLabel(this);
	probaLabel = new QLabel(this);
	probaLabel2 = new QLabel(this);
	rateLabel->setTextFormat(Qt::PlainText);
	probaLabel->setTextFormat(Qt::PlainText);
	probaLabel2->setTextFormat(Qt::PlainText);

	mainRate->setRange(0, 255);

	QHBoxLayout *rateLayout = new QHBoxLayout;
	rateLayout->addWidget(new QLabel(tr("-")));
	rateLayout->addWidget(mainRate);
	rateLayout->addWidget(new QLabel(tr("+")));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(rateLabel, 0, 0, 1, 3, Qt::AlignLeft);
	layout->addLayout(rateLayout, 1, 0, 1, 3, Qt::AlignLeft);
	layout->addWidget(new QLabel(tr("ID Combat"), this), 2, 1);
	layout->addWidget(new QLabel(tr("Probabilité"), this), 2, 2);
	QStringList specialBattleNames;
	specialBattleNames << tr("Attaque par l'arrière 1") << tr("Attaque par l'arrière 2") << tr("Attaque de côté") << tr("Attaque des deux côtés");
	QSpinBox *spinBox;
	int row=3;
	for(int i=0 ; i<10 ; ++i) {
		layout->addWidget(new QLabel(i>=6 ? specialBattleNames.at(i-6) : tr("Combat %1").arg(i+1), this), row, 0);
		spinBox = new QSpinBox(this);
		spinBox->setRange(0, 1023);
		battleIds.append(spinBox);
		layout->addWidget(spinBox, row, 1);
		spinBox = new QSpinBox(this);
		spinBox->setRange(0, 63);
		battleProbas.append(spinBox);
		layout->addWidget(spinBox, row, 2);

		++row;

		if(row == 8) {
			layout->addWidget(probaLabel, row, 0, 1, 3);
			++row;
		} else if(row == 13) {
			layout->addWidget(probaLabel2, row, 0, 1, 3);
			++row;
		}
	}

	connect(mainRate, SIGNAL(valueChanged(int)), SLOT(changePercent()));
	foreach(QSpinBox *battleProba, battleProbas)
		connect(battleProba, SIGNAL(valueChanged(int)), SLOT(changeProbaCount()));

	changePercent();
	changeProbaCount();
}

QList<int> EncounterTableWidget::battleIdsValues()
{
	QList<int> ids;
	foreach(QSpinBox *battleId, battleIds) {
		ids.append(battleId->value());
	}
	return ids;
}

QList<int> EncounterTableWidget::battleProbasValues()
{
	QList<int> probas;
	foreach(QSpinBox *battleProba, battleProbas) {
		probas.append(battleProba->value());
	}
	return probas;
}

int EncounterTableWidget::rateValue()
{
	return 255-mainRate->value();
}

void EncounterTableWidget::setBattleIdsValues(const QList<int> &ids)
{
	for(int i=0 ; i<10 ; ++i) {
		battleIds.at(i)->setValue(ids.at(i));
	}
}

void EncounterTableWidget::setBattleProbasValues(const QList<int> &probas)
{
	for(int i=0 ; i<10 ; ++i) {
		battleProbas.at(i)->setValue(probas.at(i));
	}
}

void EncounterTableWidget::setRateValue(int value)
{
	mainRate->setValue(255-value);
}

void EncounterTableWidget::changePercent()
{
	rateLabel->setText(tr("Fréquence des combats : %1/255").arg(rateValue()));
}

void EncounterTableWidget::setProbaLabelColor(QLabel *label, qint16 points)
{
	QPalette pal = label->palette();
	QColor col = QColor(points<0 ? Qt::red : (points==0 ? Qt::darkGreen : Qt::gray));
	pal.setColor(QPalette::Active, QPalette::WindowText, col);
	pal.setColor(QPalette::Inactive, QPalette::WindowText, col);
	label->setPalette(pal);
}

void EncounterTableWidget::changeProbaCount()
{
	qint16 points = 64;
	for(quint8 i=0 ; i<6 ; ++i) {
		points -= battleProbas.at(i)->value();
	}

	setProbaLabelColor(probaLabel, points);
	probaLabel->setText(tr("Points de probabilité restants : %1").arg(points));

	points = 64;
	for(quint8 i=6 ; i<10 ; ++i) {
		points -= battleProbas.at(i)->value();
	}

	setProbaLabelColor(probaLabel2, points);
	probaLabel2->setText(tr("Points de probabilité restants : %1").arg(points));
}
