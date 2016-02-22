#ifndef SPLITTER_H
#define SPLITTER_H

#include <QSplitter>

class Splitter : public QSplitter
{
	Q_OBJECT
public:
	explicit Splitter(QWidget *parent = 0);
	explicit Splitter(Qt::Orientation orientation, QWidget *parent = 0);
	bool isCollapsed(int index);
public slots:
	void setCollapsed(int index, bool collapsed);
	inline void toggleCollapsed(int index) {
		setCollapsed(index, !isCollapsed(index));
	}
};

#endif // SPLITTER_H
