#ifndef EMPTYFIELDWIDGET_H
#define EMPTYFIELDWIDGET_H

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

#endif // EMPTYFIELDWIDGET_H
