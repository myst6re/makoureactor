#ifndef SCRIPTEDITORMOVIEPAGE_H
#define SCRIPTEDITORMOVIEPAGE_H

#include <QtGui>
#include "ScriptEditorView.h"

class ScriptEditorMoviePage : public ScriptEditorView
{
	Q_OBJECT
public:
	ScriptEditorMoviePage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private slots:
	void setMovieListItemTexts(int discID);
private:
	void build();
	void buildDiscList();
	void buildMovieList(int discID);
	QComboBox *movieList, *discList;
};

#endif // SCRIPTEDITORMOVIEPAGE_H
