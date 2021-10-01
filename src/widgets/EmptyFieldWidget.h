#pragma once

#include <QtWidgets>

class EmptyFieldWidget : public QWidget
{
	Q_OBJECT
public:
	explicit EmptyFieldWidget(QWidget *parent = nullptr);

signals:
	void createMapClicked();
	void importMapClicked();
};
