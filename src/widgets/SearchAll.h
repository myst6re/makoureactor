#ifndef SEARCHALL_H
#define SEARCHALL_H

#include <QtWidgets>

class Window;
class FieldArchive;

class SearchAll : public QDialog
{
	Q_OBJECT
public:
	explicit SearchAll(Window *parent);
	void setScriptSearch();
	void setTextSearch();
	void setFieldArchive(FieldArchive *fieldArchive);
	void clear();
public slots:
	void addResultOpcode(int mapID, int grpScriptID, int scriptID, int opcodeID);
	void addResultText(int mapID, int textID, int index, int size);
private slots:
	void gotoResult(QTreeWidgetItem *item);
	void copySelected() const;
private:
	enum Mode {
		ScriptSearch, TextSearch
	};

	inline Window *mainWindow() const {
		return reinterpret_cast<Window *>(parentWidget());
	}
	void addResult(int fieldID, QTreeWidgetItem *item);
	QTreeWidgetItem *createItemField(int mapID) const;
	QTreeWidgetItem *createItemOpcode(int mapID, int grpScriptID, int scriptID, int opcodeID) const;
	QTreeWidgetItem *createItemText(int mapID, int textID, int index, int size) const;
	QTreeWidget *_resultList;
	FieldArchive *_fieldArchive;
	QMap<int, QTreeWidgetItem *> itemByMapID;
	Mode _searchMode;
};

#endif // SEARCHALL_H
