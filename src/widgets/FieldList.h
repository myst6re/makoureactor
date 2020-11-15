#ifndef FIELDLIST_H
#define FIELDLIST_H

#include <QtWidgets>
#include "core/field/FieldArchive.h"

class FieldList : public QTreeWidget
{
	Q_OBJECT
public:
	explicit FieldList(QWidget *parent = nullptr);
	int currentMapId() const;
	inline QToolBar *toolBar() {
		return _toolBar;
	}
	FieldArchive::Sorting getFieldSorting();
	inline QLineEdit *lineSearch() const {
		return _lineSearch;
	}
	void setEnabled(bool enabled);
	void enableActions(bool enabled);
public slots:
	inline void rename() {
		rename(currentItem(), 1);
	}
	void rename(QTreeWidgetItem *item, int column);
	void renameOK(QTreeWidgetItem *item, int column);
	void fill(FieldArchive *fieldArchive);
	void filterMap(const QString &name = QString());
private slots:
	void evidence(QTreeWidgetItem *current, QTreeWidgetItem *previous);
	void add();
	void del();

signals:
	void changed();
	void fieldDeleted();

private:
	static QTreeWidgetItem *createItem(Field *f, int mapID);
	void adjustWidth();

	QLineEdit *_lineSearch;
	QToolBar *_toolBar;
	
	FieldArchive *_fieldArchive;
};

#endif // FIELDLIST_H
