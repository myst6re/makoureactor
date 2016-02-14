#ifndef FIELDLIST_H
#define FIELDLIST_H

#include <QtWidgets>
#include "core/field/FieldArchive.h"

class FieldList : public QTreeWidget
{
	Q_OBJECT
public:
	explicit FieldList(QWidget *parent = 0);
	int currentFieldId() const;
	FieldArchive::Sorting getFieldSorting();
	inline QLineEdit *lineSearch() const {
		return _lineSearch;
	}
	void setEnabled(bool enabled);
public slots:
	void fill(FieldArchive *fieldArchive);
	void filterMap(const QString &name = QString());
private:
	QLineEdit *_lineSearch;
};

#endif // FIELDLIST_H
