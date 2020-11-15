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
	explicit ScriptManager(QWidget *parent = nullptr);
	void saveConfig();
	void removeCopiedReferences();
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
	inline int selectedOpcode() const {
		return _opcodeList->selectedOpcode();
	}
signals:
	void changed();
	void editText(int textID);
	void groupScriptCurrentChanged(int groupID);
	void gotoField(int fieldID);
	void searchOpcode(int opcodeID);
public slots:
	void refreshOpcode(int groupID, int scriptID, int opcodeID);
	void fill(Field *field);
	void fillScripts();
	void fillOpcodes();
	void compile();
	void refresh();
	void gotoScript(int grpScriptID, int scriptID);
	void gotoOpcode(int grpScriptID, int scriptID, int opcodeID);
private:
	GrpScriptList *_groupScriptList;
	ScriptList *_scriptList;
	OpcodeList *_opcodeList;
	QLabel *_compileScriptIcon, *_compileScriptLabel;
	Field *_field;
};

#endif // SCRIPTMANAGER_H
