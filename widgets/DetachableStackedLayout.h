#ifndef DETACHABLESTACKEDLAYOUT_H
#define DETACHABLESTACKEDLAYOUT_H

#include <QtWidgets>

class DetachableStackedLayout : public QStackedLayout
{
public:
	explicit DetachableStackedLayout(Qt::WindowFlags windowsFlags = Qt::Tool);
	DetachableStackedLayout(Qt::WindowFlags windowsFlags,
	                        QWidget *parent);
	explicit DetachableStackedLayout(QWidget *parent);
	DetachableStackedLayout(Qt::WindowFlags windowsFlags,
	                        QLayout *parentLayout);
	explicit DetachableStackedLayout(QLayout *parentLayout);

	void detachWidget(QWidget *widget);
	void detachWidget(int index);
	void detachCurrentWidget();
	void attachWidget(QWidget *widget);
	void attachWidget(int index);
private:
	Qt::WindowFlags _windowsFlags;
};

#endif // DETACHABLESTACKEDLAYOUT_H
