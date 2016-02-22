#include "Splitter.h"

Splitter::Splitter(QWidget *parent) :
    QSplitter(parent)
{
}

Splitter::Splitter(Qt::Orientation orientation, QWidget *parent) :
    QSplitter(orientation, parent)
{
}

bool Splitter::isCollapsed(int index)
{
	if (index <= -1 || index >= count()
	        || !isCollapsible(index)) {
		return false;
	}

	QList<int> s = sizes();
	return s[index] == 0;
}

void Splitter::setCollapsed(int index, bool collapsed)
{
	if (index <= -1 || index >= count()
	        || !isCollapsible(index)) {
		return;
	}

	QList<int> s = sizes();
	s[index] = int(!collapsed);
	setSizes(s);
}
