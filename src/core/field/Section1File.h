/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#pragma once

#include <QtCore>
#include "FieldPart.h"
#include "GrpScript.h"
#include "TutFileStandard.h"

#include <FF7String>

class Section1File : public FieldPart
{
public:
	enum ExportFormat {
		XMLText, TXTText
	};

	explicit Section1File(Field *field);
	void clear() override;
	void initEmpty() override;
	bool open() override;
	bool open(const QByteArray &data) override;
	QByteArray save() const override;
	bool exporter(QIODevice *device, ExportFormat format);
	bool importer(QIODevice *device, ExportFormat format);
	bool isModified() const override;

	int modelID(quint8 grpScriptID) const;
	void bgParamAndBgMove(QHash<quint8, quint8> &paramActifs, qint16 *z = nullptr, qint16 *x = nullptr, qint16 *y = nullptr) const;

	const QList<GrpScript> &grpScripts() const;
	const GrpScript &grpScript(int groupID) const;
	GrpScript &grpScript(int groupID);
	qsizetype grpScriptCount() const;
	inline static int maxGrpScriptCount() { return 256; }
	bool insertGrpScript(int row, const GrpScript &grpScript);
	void removeGrpScript(int row);
	bool moveGrpScript(int row, bool direction);

	void searchAllVars(QList<FF7Var> &vars) const;
	bool searchOpcode(int opcode, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchVar(quint8 bank, quint16 address, Opcode::Operation op, int value, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchExec(quint8 group, quint8 script, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchMapJump(quint16 map, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchTextInScripts(const QRegularExpression &text, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchText(const QRegularExpression &text, int &textID, qsizetype &from, qsizetype &size) const;
	bool searchOpcodeP(int opcode, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchVarP(quint8 bank, quint16 address, Opcode::Operation op, int value, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchExecP(quint8 group, quint8 script, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchMapJumpP(quint16 map, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchTextInScriptsP(const QRegularExpression &text, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchTextP(const QRegularExpression &text, int &textID, qsizetype &from, qsizetype &index, qsizetype &size) const;
	void setWindow(const FF7Window &win);
	void listWindows(QMultiMap<quint64, FF7Window> &windows, QMultiMap<quint8, quint64> &text2win) const;
	void listWindows(int textID, QList<FF7Window> &windows, int winID = -1) const;
	void listModelPositions(QMultiMap<int, FF7Position> &positions) const;
	int modelCount() const;
	void linePosition(QMap<int, std::pair<FF7Position, FF7Position>> &positions) const;

	void shiftTutIds(int row, int shift);
	void shiftPalIds(int row, int shift);
	bool compileScripts(int &groupID, int &scriptID, int &opcodeID, QString &errorStr);
	void removeTexts();
	void cleanTexts();
	void autosizeTextWindows();

	const QList<FF7String> &texts() const;
	qsizetype textCount() const;
	inline static int maxTextCount() { return 256; }
	const FF7String &text(int textID) const;
	void setText(int textID, const FF7String &text);
	bool insertText(int textID, const FF7String &text);
	bool replaceText(const QRegularExpression &search, const QString &after, int textID, int from);
	void deleteText(int textID);
	void clearTexts();
	QSet<quint8> listUsedTexts() const;
	QSet<quint8> listUsedTuts() const;

	const QString &author() const;
	void setAuthor(const QString &author);

	quint16 scale() const;
	void setScale(quint16 scale);

	qsizetype availableBytesForScripts() const;
private:
	QString _author;
	QByteArray _empty;
	QList<GrpScript> _grpScripts;
	QList<FF7String> _texts;
	quint16 _scale;
	quint16 _version;
};
