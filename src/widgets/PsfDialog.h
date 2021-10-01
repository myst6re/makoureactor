#pragma once

#include <QtWidgets>
#include "core/PsfFile.h"

class PsfDialog : public QDialog
{
	Q_OBJECT
public:
	explicit PsfDialog(const QString &title, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
	PsfDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags()) :
	      PsfDialog(QString(), parent, f) {}
	PsfTags tags() const;
	void setNoTitle(bool noTitle);
private:
	QLineEdit *_psflib, *_title, *_artist, *_game, *_year, *_genre;
	QLineEdit *_copyright, *_author;
	QTextEdit *_comment;
};
