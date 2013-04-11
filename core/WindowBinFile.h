#ifndef WINDOWBINFILE_H
#define WINDOWBINFILE_H

#include <QtCore>
#include "TimFile.h"

class WindowBinFile
{
public:
	enum FontColor {
		DarkGrey=0,
		DarkBlue=1,
		Red=2,
		Purple=3,
		Green=4,
		Cyan=5,
		Yellow=6,
		White=7
	};

	WindowBinFile();
	void clear();
	bool open(const QString &path);
	bool open(const QByteArray &data);
	bool isValid() const;
	bool isModified() const;
	void setModified(bool modified);
	int charCount() const;
	int tableCount() const;
	const QImage &image(FontColor color);
	const QImage &image() const;
	QImage letter(quint8 table, quint8 id) const;
	void setFontColor(FontColor color);
	quint8 charWidth(quint8 table, quint8 id) const;
	quint8 charLeftPadding(quint8 table, quint8 id) const;
	void setCharWidth(quint8 table, quint8 id, quint8 width);
	void setCharLeftPadding(quint8 table, quint8 id, quint8 padding);
private:
	int palette(FontColor color) const;
	bool openFont(const QByteArray &data);
	bool openFontSize(const QByteArray &data);
	inline quint8 charInfo(quint8 table, quint8 id) const {
		return _charWidth.at(table * 217 + id);
	}
	inline void setCharInfo(quint8 table, quint8 id, quint8 info) {
		_charWidth.replace(table * 217 + id, info);
	}

	QVector<quint8> _charWidth;
	TimFile _font;
	bool modified;
};

#endif // WINDOWBINFILE_H
