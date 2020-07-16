#include "ModelColorsLayout.h"

ModelColorWidget::ModelColorWidget(QWidget *parent) :
      QObject(parent)
{
	_colorWidget = new ColorDisplay(parent);
	_colorWidget->setColors(QList<QRgb>() << Qt::black);

	for (quint8 i = 0; i < 3; ++i) {
		_dirWidget[i] = new QSpinBox(parent);
		_dirWidget[i]->setRange(-32768, 32767);
		connect(_dirWidget[i], SIGNAL(editingFinished()), SLOT(relayEdition()));
	}

	_dirWidget[0]->adjustSize();
	_colorWidget->setMinimumWidth(_dirWidget[0]->height());
	_colorWidget->setMinimumHeight(_dirWidget[0]->height());

	connect(_colorWidget, SIGNAL(colorEdited(int,QRgb)), SLOT(relayEdition()));
}

void ModelColorWidget::relayEdition()
{
	emit colorDirEdited(FieldModelColorDir(
	                        _dirWidget[0]->value(),
	                        _dirWidget[1]->value(),
	                        _dirWidget[2]->value(),
	                        _colorWidget->colors().first()));
}

void ModelColorWidget::setModelColorDir(const FieldModelColorDir &dir)
{
	_colorWidget->setColors(QList<QRgb>() << dir.color);
	_dirWidget[0]->setValue(dir.dirA);
	_dirWidget[1]->setValue(dir.dirB);
	_dirWidget[2]->setValue(dir.dirC);
}

void ModelColorWidget::setReadOnly(bool ro)
{
	_colorWidget->setReadOnly(ro);
	for (quint8 i = 0; i < 3; ++i) {
		_dirWidget[i]->setReadOnly(ro);
	}
}

ModelColorsLayout::ModelColorsLayout(QWidget *parent) :
    QGridLayout(parent)
{
	addWidget(new QLabel(tr("Color")), 0, 0);
	addWidget(new QLabel(tr("X")), 0, 1);
	addWidget(new QLabel(tr("Y")), 0, 2);
	addWidget(new QLabel(tr("Z")), 0, 3);

	for (quint8 i = 0; i < 3; ++i) {
		modelColorWidget[i] = new ModelColorWidget(parent);
		addWidget(modelColorWidget[i]->colorWidget(), i + 1, 0);
		addWidget(modelColorWidget[i]->dirWidget(0), i + 1, 1);
		addWidget(modelColorWidget[i]->dirWidget(1), i + 1, 2);
		addWidget(modelColorWidget[i]->dirWidget(2), i + 1, 3);
	}

	connect(modelColorWidget[0], SIGNAL(colorDirEdited(FieldModelColorDir)),
	        SLOT(relayColorDirEdited0(FieldModelColorDir)));
	connect(modelColorWidget[1], SIGNAL(colorDirEdited(FieldModelColorDir)),
	        SLOT(relayColorDirEdited1(FieldModelColorDir)));
	connect(modelColorWidget[2], SIGNAL(colorDirEdited(FieldModelColorDir)),
	        SLOT(relayColorDirEdited2(FieldModelColorDir)));
}

void ModelColorsLayout::setModelColorDirs(const QList<FieldModelColorDir> &dirs)
{
	for (quint8 i = 0; i < 3; ++i) {
		modelColorWidget[i]->setModelColorDir(dirs.at(i));
	}
}

void ModelColorsLayout::setReadOnly(bool ro)
{
	for (quint8 i = 0; i < 3; ++i) {
		modelColorWidget[i]->setReadOnly(ro);
	}
}
