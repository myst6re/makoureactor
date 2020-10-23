#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QList>

#define VERSION_FF7_FIELD_SCRIPT_OPCODES 1
#define MIME_FF7_FIELD_SCRIPT_OPCODES "application/x-makoureactor-ff7-field-script-opcodes"
#define VERSION_FF7_FIELD_SCRIPT_GROUPS 1
#define MIME_FF7_FIELD_SCRIPT_GROUPS "application/x-makoureactor-ff7-field-script-groups"

class Opcode;
class GrpScript;
class QClipboard;

class Clipboard
{
public:
	static Clipboard *instance();
	bool hasFf7FieldScriptOpcodes() const;
	QList<Opcode *> ff7FieldScriptOpcodes() const;
	void setFF7FieldScriptOpcodes(const QList<Opcode *> &opcodes);
	bool hasFf7FieldScriptGroups() const;
	QList<GrpScript *> ff7FieldScriptGroups() const;
	void setFF7FieldScriptGroups(const QList<GrpScript *> &groups);
private:
	Clipboard() {}
	static bool hasData(const QString &mimeType);
	static QByteArray data(const QString &mimeType);
	static void setData(const QString &mimeType, const QByteArray &data);

	static Clipboard *_instance;
	
};

#endif // CLIPBOARD_H
