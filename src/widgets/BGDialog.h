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
#pragma once

#include <QtWidgets>
#include "ApercuBGLabel.h"
#include "BackgroundEditor.h"

class Field;

class BGDialog : public QDialog
{
    Q_OBJECT
public:
	explicit BGDialog(QWidget *parent);
	void fill(Field *field, bool reload = false);
	void clear();
	void saveConfig();
signals:
	void modified();
private slots:
	void showLayersPage(int index);
	void parameterChanged(int index);
	void layerChanged();
	void sectionChanged();
	void enableState(QListWidgetItem *item);
	void enableLayer(QListWidgetItem *item);
	void enableSection(QListWidgetItem *item);
	void changeZ(int value);
	void saveImage();
	void tryToRepairBG();
	void updateBG();
private:
	QImage background(bool *bgWarning = nullptr);
	void fillWidgets();

	Field *_field;
	ApercuBGLabel *image;
	QTabWidget *tabWidget;
	QWidget *viewerPage;
	BackgroundEditor *editorPage;
	QComboBox *parametersWidget;
	QListWidget *statesWidget, *layersWidget, *sectionsWidget;
	QSpinBox *zWidget;
	QPushButton *buttonRepair;
	QTabBar *tabBar;

	QMap<LayerParam, quint8> allparams;
	QHash<quint8, quint8> params;
	QSet<quint16> sections;
	QScrollArea *imageBox;
	float zoomFactor;
	bool layers[4];
	qint16 x[3], y[3], z[3];
protected:
	void showEvent(QShowEvent *event) override;
	bool eventFilter(QObject *, QEvent *) override;
	void resizeEvent(QResizeEvent*) override;
};
