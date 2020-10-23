#ifndef MODELCOLORSLAYOUT_H
#define MODELCOLORSLAYOUT_H

#include <QtWidgets>
#include "ColorDisplay.h"
#include "core/field/FieldModelLoader.h"

class ModelColorWidget : public QObject
{
	Q_OBJECT
public:
	explicit ModelColorWidget(QWidget *parent = 0);
	void setModelColorDir(const FieldModelColorDir &dir);
	void setReadOnly(bool ro);
	inline ColorDisplay *colorWidget() const {
		return _colorWidget;
	}
	inline QSpinBox *dirWidget(int id) const {
		return _dirWidget[id];
	}
signals:
	void colorDirEdited(const FieldModelColorDir &colorDir);
private slots:
	void relayEdition();
private:
	ColorDisplay *_colorWidget;
	QSpinBox *_dirWidget[3];
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
	ModelColorWidget *modelColorWidget[3];
};

#endif // MODELCOLORSLAYOUT_H
