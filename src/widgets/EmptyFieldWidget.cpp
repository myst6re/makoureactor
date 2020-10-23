#include "EmptyFieldWidget.h"

EmptyFieldWidget::EmptyFieldWidget(QWidget *parent) : QWidget(parent)
{
	QPushButton *createNewMapButton = new QPushButton("Create new map");
	QPushButton *importMapButton = new QPushButton("Import existing map...");

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addStretch(1);
	layout->addWidget(createNewMapButton);
	layout->addWidget(importMapButton);
	layout->addStretch(1);

	connect(createNewMapButton, SIGNAL(released()), SIGNAL(createMapClicked()));
	connect(importMapButton, SIGNAL(released()), SIGNAL(importMapClicked()));
}
