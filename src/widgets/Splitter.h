#pragma once

#include <QSplitter>
#include <QMap>

class Splitter : public QSplitter
{
	Q_OBJECT
public:
	explicit Splitter(QWidget *parent = nullptr);
	explicit Splitter(Qt::Orientation orientation, QWidget *parent = nullptr);
	bool isCollapsed(int index);
public slots:
	void setCollapsed(int index, bool collapsed);
	inline void toggleCollapsed(int index) {
		setCollapsed(index, !isCollapsed(index));
	}
private:
	QMap<int, int> _lastSizes;
};
