#ifndef WINDOWBINFILE_H
#define WINDOWBINFILE_H

#include <QtCore>
#include "TimFile.h"

#define LEFT_PADD(w)	(w >> 5)
#define CHAR_WIDTH(w)	(w & 0x1F)

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
	bool isJp() const;
	void setModified(bool modified);
	int charCount() const;
	int tableCount() const;
	const QImage &image(FontColor color);
	QImage letter(quint8 table, quint8 id, FontColor color);
	quint8 charWidth(quint8 table, quint8 id) const;
	quint8 charLeftPadding(quint8 table, quint8 id) const;
	void setCharWidth(quint8 table, quint8 id, quint8 width);
	void setCharLeftPadding(quint8 table, quint8 id, quint8 padding);
private:
	QImage letter(int id, FontColor color);
	int palette(FontColor color, quint8 table) const;
	QRect letterRect(int charId) const;
	bool openFont(const QByteArray &data);
	bool openFont2(const QByteArray &data);
	bool openFontSize(const QByteArray &data);
	static int absoluteId(quint8 table, quint8 id);
	inline quint8 charInfo(quint8 table, quint8 id) const {
		return _charWidth.at(absoluteId(table, id));
	}
	inline void setCharInfo(quint8 table, quint8 id, quint8 info) {
		_charWidth.replace(absoluteId(table, id), info);
	}

	QVector<quint8> _charWidth;
	TimFile _font, _font2;
	bool modified;
};

#endif // WINDOWBINFILE_H
