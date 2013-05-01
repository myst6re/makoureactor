#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include <QtGui>
#include "FontWidget.h"

class FontManager : public QDialog
{
public:
	explicit FontManager(QWidget *parent=0);
private:
	FontWidget *fontWidget;
};

#endif // FONTMANAGER_H
