#ifndef SCRIPTMANAGER_H
#define SCRIPTMANAGER_H

#include <QtWidgets>
#include "widgets/GrpScriptList.h"
#include "widgets/ScriptList.h"
#include "widgets/OpcodeList.h"

class ScriptManager : public QWidget
{
	Q_OBJECT
public:
	explicit ScriptManager(QWidget *parent = 0);
	void saveConfig();
	void clear();
	inline GrpScriptList *groupScriptList() const {
		return _groupScriptList;
	}
	inline ScriptList *scriptList() const {
		return _scriptList;
	}
	inline OpcodeList *opcodeList() const {
		return _opcodeList;
	}
	inline int currentGrpScriptId() const {
		return _groupScriptList->selectedID();
	}
	inline int currentScriptId() const {
		return _scriptList->selectedID();
	}
	inline int currentOpcodeId() const {
		return _opcodeList->selectedID();
	}
signals:
	void changed();
	void editText(int);
	void groupScriptCurrentChanged(int);
public slots:
	void fill(Field *field);
	void fillScripts();
	void fillOpcodes();
	void compile();
	void refresh();
	void gotoOpcode(int grpScriptID, int scriptID, int opcodeID);
private:
	GrpScriptList *_groupScriptList;
	ScriptList *_scriptList;
	OpcodeList *_opcodeList;
	QLabel *_compileScriptIcon, *_compileScriptLabel;
	Field *_field;
};

#endif // SCRIPTMANAGER_H
