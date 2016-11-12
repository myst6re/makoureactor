#ifndef MODELCOLORSLAYOUT_H
#define MODELCOLORSLAYOUT_H

#include <QtWidgets>
#include "ColorDisplay.h"
#include "core/field/FieldModelLoader.h"

class ModelColorLayout : public QHBoxLayout
{
	Q_OBJECT
public:
	explicit ModelColorLayout(QWidget *parent = 0);
	void setModelColorDir(const FieldModelColorDir &dir);
	void setReadOnly(bool ro);
signals:
	void colorDirEdited(const FieldModelColorDir &colorDir);
private slots:
	void relayEdition();
private:
	ColorDisplay *colorWidget;
	QSpinBox *dirWidget[3];
};

class ModelColorsLayout : public QGridLayout
{
	Q_OBJECT
public:
	explicit ModelColorsLayout(QWidget *parent = 0);
	void setModelColorDirs(const QList<FieldModelColorDir> &dirs);
	void setReadOnly(bool ro);
signals:
	void colorDirEdited(int id, const FieldModelColorDir &colorDir);
private slots:
	void relayColorDirEdited0(const FieldModelColorDir &colorDir) {
		emit colorDirEdited(0, colorDir);
	}
	void relayColorDirEdited1(const FieldModelColorDir &colorDir) {
		emit colorDirEdited(1, colorDir);
	}
	void relayColorDirEdited2(const FieldModelColorDir &colorDir) {
		emit colorDirEdited(2, colorDir);
	}

private:
	ModelColorLayout *modelColorLayout[3];
};

#endif // MODELCOLORSLAYOUT_H
