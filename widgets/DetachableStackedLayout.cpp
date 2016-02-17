#include "DetachableStackedLayout.h"

DetachableStackedLayout::DetachableStackedLayout(Qt::WindowFlags windowsFlags) :
    QStackedLayout(), _windowsFlags(windowsFlags)
{
}

DetachableStackedLayout::DetachableStackedLayout(Qt::WindowFlags windowsFlags,
                                                 QWidget *parent) :
    QStackedLayout(parent), _windowsFlags(windowsFlags)
{
}

DetachableStackedLayout::DetachableStackedLayout(QWidget *parent) :
    QStackedLayout(parent), _windowsFlags(Qt::Tool)
{
}

DetachableStackedLayout::DetachableStackedLayout(Qt::WindowFlags windowsFlags,
                                                 QLayout *parentLayout) :
    QStackedLayout(parentLayout), _windowsFlags(windowsFlags)
{
}

DetachableStackedLayout::DetachableStackedLayout(QLayout *parentLayout) :
    QStackedLayout(parentLayout), _windowsFlags(Qt::Tool)
{
}

void DetachableStackedLayout::detachWidget(QWidget *widget)
{
	if(!widget) {
		return;
	}
	removeWidget(widget);
	widget->setWindowFlags(_windowsFlags);
	widget->adjustSize();
	widget->show();
}

void DetachableStackedLayout::detachWidget(int index)
{
	detachWidget(widget(index));
}

void DetachableStackedLayout::detachCurrentWidget()
{
	detachWidget(currentWidget());
}

void DetachableStackedLayout::attachWidget(QWidget *widget)
{
	if(!widget) {
		return;
	}
	addWidget(widget);
	widget->setWindowFlags(Qt::Widget);
	widget->show();
}

void DetachableStackedLayout::attachWidget(int index)
{
	attachWidget(widget(index));
}
