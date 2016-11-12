#include "ModelColorsLayout.h"

ModelColorLayout::ModelColorLayout(QWidget *parent) :
    QHBoxLayout(parent)
{
	colorWidget = new ColorDisplay(parent);
	colorWidget->setColors(QList<QRgb>() << Qt::black);
	addWidget(colorWidget);

	for(quint8 i = 0; i < 3; ++i) {
		dirWidget[i] = new QSpinBox(parent);
		dirWidget[i]->setRange(-32768, 32767);
		addWidget(dirWidget[i]);
		connect(dirWidget[i], SIGNAL(editingFinished()), SLOT(relayEdition()));
	}

	colorWidget->setMinimumSize(dirWidget[0]->minimumSizeHint());

	connect(colorWidget, SIGNAL(colorEdited(int,QRgb)), SLOT(relayEdition()));
}

void ModelColorLayout::relayEdition()
{
	emit colorDirEdited(FieldModelColorDir(
	                        dirWidget[0]->value(),
	                        dirWidget[1]->value(),
	                        dirWidget[2]->value(),
	                        colorWidget->colors().first()));
}

void ModelColorLayout::setModelColorDir(const FieldModelColorDir &dir)
{
	colorWidget->setColors(QList<QRgb>() << dir.color);
	dirWidget[0]->setValue(dir.dirA);
	dirWidget[1]->setValue(dir.dirB);
	dirWidget[2]->setValue(dir.dirC);
}

void ModelColorLayout::setReadOnly(bool ro)
{
	colorWidget->setReadOnly(ro);
	for(quint8 i = 0; i < 3; ++i) {
		dirWidget[i]->setReadOnly(ro);
	}
}

ModelColorsLayout::ModelColorsLayout(QWidget *parent) :
    QGridLayout(parent)
{
	addWidget(new QLabel(tr("Color")), 0, 0);
	addWidget(new QLabel(tr("X")), 0, 1);
	addWidget(new QLabel(tr("Y")), 0, 2);
	addWidget(new QLabel(tr("Z")), 0, 3);

	for(quint8 i = 0; i < 3; ++i) {
		modelColorLayout[i] = new ModelColorLayout(parent);
		addLayout(modelColorLayout[i], i + 1, 0, 1, 4);
	}

	connect(modelColorLayout[0], SIGNAL(colorDirEdited(FieldModelColorDir)),
	        SLOT(relayColorDirEdited0(FieldModelColorDir)));
	connect(modelColorLayout[1], SIGNAL(colorDirEdited(FieldModelColorDir)),
	        SLOT(relayColorDirEdited1(FieldModelColorDir)));
	connect(modelColorLayout[2], SIGNAL(colorDirEdited(FieldModelColorDir)),
	        SLOT(relayColorDirEdited2(FieldModelColorDir)));
}

void ModelColorsLayout::setModelColorDirs(const QList<FieldModelColorDir> &dirs)
{
	for(quint8 i = 0; i < 3; ++i) {
		modelColorLayout[i]->setModelColorDir(dirs.at(i));
	}
}

void ModelColorsLayout::setReadOnly(bool ro)
{
	for(quint8 i = 0; i < 3; ++i) {
		modelColorLayout[i]->setReadOnly(ro);
	}
}
