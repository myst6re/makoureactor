#include "WindowBinFile.h"
#include "GZIP.h"

#define LEFT_PADD(w)	(w >> 5)
#define CHAR_WIDTH(w)	(w & 0x1F)

WindowBinFile::WindowBinFile()
{
}

bool WindowBinFile::open(const QString &path)
{
	QFile windowFile(path);
	if(windowFile.open(QIODevice::ReadOnly)) {
		QByteArray windowData = windowFile.readAll();
		windowFile.close();
		if(!open(windowData)) {
			return false;
		}
	} else {
		return false;
	}

	return true;
}

bool WindowBinFile::open(const QByteArray &data)
{
	const char *constData = data.constData();
	int cur=0;
	quint16 size;

	// Section 1 (icons texture)
	if(data.size() < 2) {
		return false;
	}

	memcpy(&size, constData, 2);

	// Section 2 (font texture)
	cur += 6 + size;

	if(data.size() < cur + 2) {
		return false;
	}

	memcpy(&size, constData + cur, 2);

	if(!openFont(GZIP::decompress(data.mid(cur + 6, size), 0))) {
		return false;
	}

	// Section 3 (font size)
	cur += 6 + size;

	if(data.size() < cur + 2) {
		return false;
	}

	memcpy(&size, constData + cur, 2);

	if(!openFontSize(GZIP::decompress(data.mid(cur + 6, size), 0))) {
		return false;
	}

	return true;
}

bool WindowBinFile::isValid() const
{
	return !_charWidth.isEmpty();
}

bool WindowBinFile::openFont(const QByteArray &data)
{
	_font = TimFile(data);
	return _font.isValid() && _font.colorTableCount() == 16; //TODO: jp font
}

bool WindowBinFile::openFontSize(const QByteArray &data)
{
	if(data.size() != 1302) {
		return false;
	}

	_charWidth.resize(data.size());

	memcpy(_charWidth.data(), data.constData(), data.size());

	return true;
}

QImage WindowBinFile::letter(quint8 table, quint8 id) const
{
	if(table != 0) {
		return QImage(); //TODO: jp font
	}

	return _font.image().copy((id % 21) * 12,
							  (id / 21) * 12,
							  12, 12);
}

void WindowBinFile::setFontColor(FontColor color)
{
	_font.setCurrentColorTable(int(color) * 2); //TODO: jp font
}

quint8 WindowBinFile::charWidth(quint8 table, quint8 id) const
{
	return CHAR_WIDTH(charInfo(table, id));
}

quint8 WindowBinFile::charLeftPadding(quint8 table, quint8 id) const
{
	return LEFT_PADD(charInfo(table, id));
}
