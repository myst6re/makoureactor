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
	return s.at(index) == 0;
}

void Splitter::setCollapsed(int index, bool collapsed)
{
	if (index <= -1 || index >= count()
	        || !isCollapsible(index)) {
		return;
	}

	QList<int> s = sizes();
	if ((s.at(index) == 0) == collapsed) {
		return; // Already in the right state
	}
	if (collapsed) {
		_lastSizes.insert(index, s.at(index));
		s[index] = 0;
	} else {
		s[index] = _lastSizes.value(index, 1);
	}
	setSizes(s);
}
