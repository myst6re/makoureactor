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
#ifndef ENCOUNTERTABLEWIDGET_H
#define ENCOUNTERTABLEWIDGET_H

#include <QtWidgets>

class EncounterTableWidget : public QGroupBox
{
    Q_OBJECT
public:
	explicit EncounterTableWidget(const QString &title, QWidget *parent=0);
	QList<int> battleIdsValues();
	QList<int> battleProbasValues();
	int rateValue();
	void setBattleIdsValues(const QList<int> &ids);
	void setBattleProbasValues(const QList<int> &probas);
	void setRateValue(int value);
public slots:
	void changePercent();
	void changeProbaCount();
private:
	static void setProbaLabelColor(QLabel *label, qint16 points);
	QLabel *rateLabel, *probaLabel, *probaLabel2;
	QSlider *mainRate;
	QGroupBox *group;
	QList<QSpinBox *> battleIds;
	QList<QSpinBox *> battleProbas;
};

#endif // ENCOUNTERTABLEWIDGET_H
