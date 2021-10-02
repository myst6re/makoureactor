/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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
#include "FF7Font.h"
#include "Config.h"
#include "Data.h"

FF7Font::FF7Font(WindowBinFile *windowBinFile, const QByteArray &txtFileData) :
	_windowBinFile(windowBinFile), modified(false), readOnly(false)
{
	openTxt(QString::fromUtf8(txtFileData.data()));
}

WindowBinFile *FF7Font::windowBinFile() const
{
	return _windowBinFile;
}

const QList<QStringList> &FF7Font::tables() const
{
	return _tables;
}

void FF7Font::setTables(const QList<QStringList> &tables)
{
	_tables = tables;
	modified = true;
}

void FF7Font::setChar(int tableId, int charId, const QString &c)
{
	if (tableId < _tables.size() && charId < 224) {
		_tables[tableId][charId] = c;
		modified = true;
	}
}

bool FF7Font::isValid() const
{
	return !_tables.isEmpty() && _windowBinFile->isValid();//TODO: _windowBinFile->isNull()?
}

bool FF7Font::isModified() const
{
	return modified /*|| _windowBinFile->isModified()*/;//TODO
}

void FF7Font::setModified(bool modified)
{
	this->modified = modified;
//	_windowBinFile->setModified(modified);//TODO
}

const QString &FF7Font::name() const
{
	return _name;
}

void FF7Font::setName(const QString &name)
{
	_name = name;
	modified = true;
}

void FF7Font::setPaths(const QString &txtPath, const QString &windowBinFilePath)
{
	_txtPath = txtPath;
	_windowBinFilePath = windowBinFilePath;
	if (_name.isEmpty()) {
		_name = txtPath.mid(txtPath.lastIndexOf('/')+1);
		_name = _name.left(_name.size() - 4);
	}
}

const QString &FF7Font::txtPath() const
{
	return _txtPath;
}

const QString &FF7Font::windowBinFilePath() const
{
	return _windowBinFilePath;
}

void FF7Font::setReadOnly(bool ro)
{
	readOnly = ro;
}

bool FF7Font::isReadOnly() const
{
	return readOnly;
}

void FF7Font::openTxt(const QString &data)
{
	int tableCount = _windowBinFile->tableCount();
	QStringList lines = data.split(QRegExp("\\s*(\\r\\n|\\r|\\n)\\s*"), Qt::SkipEmptyParts);
	QRegExp nameRegExp("#NAME\\s+(\\S.*)"), letterRegExp("\\s*\"([^\"]*|\\\"*)\"\\s*,?\\s*");
	// nameRegExp:		#NAME blah blah blah
	// letterRegExp:	"Foo", "Foo","Foo"
	QStringList table;

	if (tableCount < 1) {
		qWarning() << "invalid windowBinFile!";
		return;
	}

	for (const QString &line : qAsConst(lines)) {
		if (line.startsWith("#")) {
			if (nameRegExp.indexIn(line) != -1) {
				QStringList capturedTexts = nameRegExp.capturedTexts();
				_name = capturedTexts.at(1).trimmed();
			}
		}
		else {
			int offset=0;
			while ((offset = letterRegExp.indexIn(line, offset)) != -1) {
				QStringList capturedTexts = letterRegExp.capturedTexts();
				table.append(capturedTexts.at(1));
				offset += capturedTexts.first().size();

				if (table.size() == 224) {
					_tables.append(table);
					if (_tables.size() > tableCount) {
						//print();
						return;
					}
					table = QStringList();
				}
			}
		}
	}

	if (!table.isEmpty()) {
		if (table.size() < 224) {
			for (int i=table.size(); i<224; ++i) {
				table.append(QString());
			}
		}

		_tables.append(table);
	}

	//print();
}

QString FF7Font::saveTxt()
{
	QString data;

	if (!_name.isEmpty()) {
		data.append("#NAME\t").append(_name).append("\n");
	}

	for (const QStringList &t : qAsConst(_tables)) {
		for (int j=0; j<14; ++j) {
			for (int i=0; i<16; ++i) {
				data.append(QString("\"%1\"").arg(t[j*16 + i]));
				if (i<15) {
					data.append(",");
				}
			}
			if (j<13) {
				data.append(",");
			}
			data.append("\n");
		}
		data.append("\n");
	}

	return data;
}

/*
void FF7Font::print()
{
	int tid=1;
	for (const QStringList &t : _tables) {
		qDebug() << QString("table %1").arg(tid++).toLatin1().data();
		for (int j=0; j<14; ++j) {
			QString buf;
			for (int i=0; i<16; ++i) {
				buf += QString("\"%1\",").arg(t[j*16 + i]);
			}
			qDebug() << buf.toLatin1().data();
		}
	}
} */

QMap<QString, FF7Font *> FF7Font::fonts;
QString FF7Font::font_dirPath;

bool FF7Font::listFonts()
{
	qDeleteAll(fonts);
	fonts.clear();

#ifdef Q_OS_WIN
	font_dirPath = qApp->applicationDirPath();
//	font_dirPath = QDir::cleanPath(QSettings(QSettings::IniFormat, QSettings::UserScope, MAKOU_REACTOR_NAME, MAKOU_REACTOR_NAME).fileName());
//	font_dirPath = font_dirPath.left(font_dirPath.lastIndexOf('/'));
#else
	font_dirPath = QDir::cleanPath(QSettings(MAKOU_REACTOR_NAME, MAKOU_REACTOR_NAME).fileName());
	font_dirPath = font_dirPath.left(font_dirPath.lastIndexOf('/'));
#endif

	QDir dir(font_dirPath);
	dir.mkpath(font_dirPath);
	QStringList stringList = dir.entryList(QStringList("*.windowBinFile"), QDir::Files, QDir::Name);

	FF7Font *latinFont = openFont(":/fonts/sysfnt.windowBinFile", ":/fonts/sysfnt.txt");
	FF7Font *jpFont = openFont(":/fonts/sysfnt_jp.windowBinFile", ":/fonts/sysfnt_jp.txt");

	if (!latinFont || !jpFont) {
		if (latinFont) delete latinFont;
		if (jpFont)    delete jpFont;
		return false;
	}

	latinFont->setReadOnly(true);
	jpFont->setReadOnly(true);

	fonts.insert("00", latinFont);
	fonts.insert("01", jpFont);

	for (const QString &str : qAsConst(stringList)) {
		int index = str.lastIndexOf('.');
		fonts.insert(str.left(index), nullptr);
	}

	return true;
}

QStringList FF7Font::fontList()
{
	return fonts.keys();
}

FF7Font *FF7Font::openFont(const QString &windowBinFilePath, const QString &txtPath)
{
	WindowBinFile *windowBinFile = nullptr;
	QFile f(windowBinFilePath);
	if (f.open(QIODevice::ReadOnly)) {
		windowBinFile = new WindowBinFile();
		if (!windowBinFile->open(f.readAll())) {
			qWarning() << "Cannot open windowBinFile file!" << f.fileName();
			delete windowBinFile;
			windowBinFile = nullptr;
		}
		f.close();
	}

	if (!windowBinFile) {
		return nullptr;
	}

	FF7Font *ff7Font;
	QFile f2(txtPath);
	if (f2.open(QIODevice::ReadOnly)) {
		ff7Font = new FF7Font(windowBinFile, f2.readAll());
		f2.close();
	} else {
		ff7Font = new FF7Font(windowBinFile, QByteArray());
	}
	ff7Font->setPaths(txtPath, windowBinFilePath);
	return ff7Font;
}

FF7Font *FF7Font::font(QString name)
{
	if (name.isEmpty()) {
		name = "00";
	}

	if (fonts.contains(name)) {
		FF7Font *ff7Font = fonts.value(name);
		if (!ff7Font) {
			ff7Font = openFont(font_dirPath + "/" + name + ".windowBinFile", font_dirPath + "/" + name + ".txt");
			if (!ff7Font) {
				fonts.remove(name);// Bad font, we can remove it
			} else {
				fonts.insert(name, ff7Font);
			}
		}
		return ff7Font;
	}

	return nullptr;
}

FF7Font *FF7Font::getCurrentConfigFont()
{
	QString fnt = Config::value("encoding", "00").toString();

	QStringList fontL = fontList();
	if (fontL.contains(fnt)) {
		return font(fnt);
	}
	return font(fontL.first());
}

bool FF7Font::saveFonts()
{
	bool ok = true;

	for (FF7Font *font : qAsConst(fonts)) {
		if (font && !font->isReadOnly() && font->isModified()) {
			QFile f1(font->txtPath());
			if (f1.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
				f1.write(font->saveTxt().toUtf8());
				f1.close();
			} else {
				ok = false;
			}
			QFile f2(font->windowBinFilePath());
			if (f2.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
				//TODO
				/*QByteArray tdwData;
				if (font->windowBinFile()->save(tdwData)) {
					f2.write(tdwData);
				} else {
					ok = false;
				}*/
				f2.close();
			} else {
				ok = false;
			}
			if (ok) {
				font->setModified(false);
			}
		}
	}

	return ok;
}

bool FF7Font::addFont(const QString &name, const QString &from,
                      const QString &displayName)
{
	if (fonts.contains(name)) {
		return false;
	}

	FF7Font *ff7Font = font(from);
	if (!ff7Font) {
		return false;
	}

	QFile ftxt(font_dirPath + "/" + name + ".txt");
	if (!ftxt.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		return false;
	}

	QFile ftxt2(ff7Font->txtPath());
	if (!ftxt2.open(QIODevice::ReadOnly)) {
		return false;
	}

	ftxt.write(ftxt2.readAll());

	ftxt.close();
	ftxt2.close();

	QFile ftdw(font_dirPath + "/" + name + ".windowBinFile");
	if (!ftdw.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		return false;
	}

	QFile ftdw2(ff7Font->windowBinFilePath());
	if (!ftdw2.open(QIODevice::ReadOnly)) {
		return false;
	}

	ftdw.write(ftdw2.readAll());

	ftdw.close();
	ftdw2.close();

	fonts.insert(name, nullptr);

	ff7Font = font(name);
	if (!ff7Font) {
		return false;
	}

	ff7Font->setName(displayName);
	ff7Font->setReadOnly(false);

	return true;
}

bool FF7Font::removeFont(const QString &name)
{

	FF7Font *ff7Font = font(name);

	if (!ff7Font || ff7Font->isReadOnly()) {
		return false;
	}

	if (!QFile::remove(ff7Font->txtPath())
	|| !QFile::remove(ff7Font->windowBinFilePath())) {
		return false;
	}

	delete fonts.take(name);

	return true;
}

const QString &FF7Font::fontDirPath()
{
	return font_dirPath;
}

QSize FF7Font::calcSize(const QByteArray &ff7Text)
{
	QList<int> pagesPos;
	return calcSize(ff7Text, pagesPos);
}

QSize FF7Font::calcSize(const QByteArray &ff7Text, QList<int> &pagesPos)
{
	if (biggestCharWidth <= 0) {
		biggestCharWidth = calcFF7TextWidth(FF7Text("W", false));
	}

	const int baseWidth = 8 + Config::value("autoSizeMarginRight", 14).toInt();
	int line=0, width=baseWidth - 3, height=25, size=ff7Text.size(), maxW=0, maxH=0;
	pagesPos.clear();
	pagesPos.append(0);
	bool jp = Config::value("jp_txt", false).toBool(), spaced_characters=false;
	int spacedCharsW = Config::value("spacedCharactersWidth", 13).toInt(),
	    choiceW = Config::value("choiceWidth", 10).toInt(),
	    tabW = Config::value("tabWidth", 4).toInt();

	for (int i=0; i<size; ++i) {
		quint8 caract = (quint8)ff7Text.at(i);
		if (caract==0xff) break;
		switch (caract) {
		case 0xe8: // New Page
		case 0xe9: // New Page 2
			if (line == 0)	width += 3;
			if (height>maxH)	maxH = height;
			if (width>maxW)	maxW = width;
			++line;
			width = baseWidth;
			height = 25;
			pagesPos.append(i+1);
			break;
		case 0xe7: // \n
			if (line == 0)	width += 3;
			if (width>maxW)	maxW = width;
			++line;
			width = baseWidth;
			height += 16;
			break;
		case 0xfa: // Jap 1
			++i;
			caract = (quint8)ff7Text.at(i);
			if (jp) {
				width += spaced_characters ? spacedCharsW : charFullWidth(2, caract);
			} else if (caract < 0xd2) {
				width += spaced_characters ? spacedCharsW : 1;
			}
			break;
		case 0xfb: // Jap 2
			++i;
			if (jp) {
				caract = (quint8)ff7Text.at(i);
				width += spaced_characters ? spacedCharsW : charFullWidth(3, caract);
			}
			break;
		case 0xfc: // Jap 3
			++i;
			if (jp) {
				caract = (quint8)ff7Text.at(i);
				width += spaced_characters ? spacedCharsW : charFullWidth(4, caract);
			}
			break;
		case 0xfd: // Jap 4
			++i;
			if (jp) {
				caract = (quint8)ff7Text.at(i);
				width += spaced_characters ? spacedCharsW : charFullWidth(5, caract);
			}
			break;
		case 0xfe: // Jap 5 + add
			++i;
			if (i >= size)		break;
			caract = (quint8)ff7Text.at(i);
			if (caract == 0xdd)
				++i;
			else if (caract == 0xde || caract == 0xdf || caract == 0xe1) { // {VARHEX}, {VARDEC}, {VARDECR}
				int zeroId = !jp ? 0x10 : 0x33;
				width += (spaced_characters ? spacedCharsW : charFullWidth(0, zeroId)) * 5;
			} else if (caract == 0xe2) {// {MEMORY}
				if (i + 3 >= size)		break;
				const quint8 len = quint8(ff7Text.at(i + 3));
				width += (spaced_characters ? spacedCharsW : biggestCharWidth) * len;
				i += 4;
			} else if (caract == 0xe9) // {SPACED CHARACTERS}
				spaced_characters = !spaced_characters;
			else if (caract < 0xd2 && jp)
				width += spaced_characters ? spacedCharsW : charFullWidth(6, caract);
			break;
		default:
			if (!jp && caract==0xe0) {// {CHOICE}
				width += spaced_characters ? spacedCharsW * choiceW : 3 * choiceW;
			} else if (!jp && caract==0xe1) {// \t
				width += spaced_characters ? spacedCharsW * tabW : 3 * tabW;
			} else if (!jp && caract>=0xe2 && caract<=0xe4) {// duo
				const char *duo = optimisedDuo[caract-0xe2];
				width += spaced_characters ? spacedCharsW : charFullWidth(1, (quint8)duo[0]);
				width += spaced_characters ? spacedCharsW : charFullWidth(1, (quint8)duo[1]);
			} else if (caract>=0xea && caract<=0xf5) {// Character names
				width += (spaced_characters ? spacedCharsW : biggestCharWidth) * 9;
			} else if (caract>=0xf6 && caract<=0xf9) {// Keys
				width += 17;
			} else {
				if (jp) {
					width += spaced_characters ? spacedCharsW : charFullWidth(1, caract);
				} else {
					width += spaced_characters ? spacedCharsW : charFullWidth(0, caract);
				}
			}
			break;
		}
	}

	if (height>maxH)	maxH = height;
	if (width>maxW)	maxW = width;
	if (maxW>322)	maxW = 322;
	if (maxH>226)	maxH = 226;

	return QSize(maxW, maxH);
}

quint8 FF7Font::charW(int tableId, int charId)
{
	return Data::windowBin.isValid() &&
	               (tableId != 0 || !Data::windowBin.isJp())
	           ? Data::windowBin.charWidth(tableId == 0 ? 0 : tableId - 1, charId)
	           : CHARACTER_WIDTH(charWidth[tableId][charId]);
}

quint8 FF7Font::leftPadding(int tableId, int charId)
{
	return Data::windowBin.isValid() &&
	               (tableId != 0 || !Data::windowBin.isJp())
	           ? Data::windowBin.charLeftPadding(tableId == 0 ? 0 : tableId - 1, charId)
	           : LEFT_PADD(charWidth[tableId][charId]);
}

quint8 FF7Font::charFullWidth(int tableId, int charId)
{
	return charW(tableId, charId) + leftPadding(tableId, charId);
}

int FF7Font::calcFF7TextWidth(const FF7Text &ff7Text)
{
	int width = 0;
	const QByteArray &data = ff7Text.data();

	for (const char &c : qAsConst(data)) {
		if (quint8(c) < 0xe0) {
			width += charFullWidth(0, c);
		}
	}

	return width;
}

int FF7Font::biggestCharWidth;

quint8 FF7Font::charWidth[7][256] =
{
    { // International
         3, 3, 72, 10, 7, 10, 9, 3, 72, 72, 7, 7, 39, 5, 38, 6,
         8, 71, 8, 8, 8, 8, 8, 8, 8, 8, 69, 4, 7, 8, 7, 6,
         10, 9, 7, 8, 8, 7, 7, 8, 8, 3, 6, 7, 7, 11, 8, 9,
         7, 9, 7, 7, 7, 8, 9, 11, 8, 9, 7, 4, 6, 4, 7, 8,
         4, 7, 7, 6, 7, 7, 6, 7, 7, 3, 4, 6, 3, 11, 7, 7,
         7, 7, 5, 6, 6, 7, 7, 11, 7, 7, 6, 5, 3, 5, 8, 68,
         73, 76, 72, 73, 73, 9, 8, 7, 7, 7, 7, 7, 7, 7, 7, 7,
         7, 7, 4, 4, 4, 4, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
         11, 6, 7, 8, 11, 6, 7, 7, 9, 9, 11, 4, 5, 8, 12, 9,
         11, 7, 7, 7, 9, 7, 7, 7, 9, 8, 4, 6, 6, 9, 11, 7,
         6, 3, 8, 7, 8, 8, 9, 7, 7, 9, 1, 9, 9, 9, 12, 11,
         8, 12, 6, 6, 4, 4, 7, 7, 7, 9, 7, 9, 5, 5, 7, 7,
         8, 3, 4, 6, 13, 9, 7, 9, 7, 7, 3, 4, 4, 3, 9, 9,
         8, 9, 8, 8, 8, 3, 6, 7, 5, 6, 3, 6, 5, 6, 5, 5,
         1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },{ // Jap - 0
         13, 13, 11, 13, 12, 13, 12, 12, 12, 13, 12, 12, 11, 12, 12, 10,
         12, 12, 12, 11, 13, 11, 12, 9, 12, 12, 12, 13, 10, 12, 12, 12,
         12, 12, 12, 12, 12, 12, 8, 11, 12, 13, 12, 10, 12, 12, 12, 12,
         12, 11, 13, 8, 70, 8, 8, 9, 8, 8, 7, 8, 8, 39, 41, 8,
         12, 11, 9, 11, 10, 12, 12, 12, 11, 11, 10, 12, 10, 10, 10, 8,
         11, 11, 10, 9, 11, 10, 11, 9, 10, 11, 11, 11, 9, 10, 10, 11,
         11, 10, 10, 11, 11, 10, 6, 9, 10, 10, 11, 12, 10, 11, 11, 11,
         11, 12, 10, 12, 11, 10, 9, 12, 11, 12, 9, 11, 10, 12, 9, 12,
         11, 11, 10, 11, 11, 10, 10, 9, 8, 8, 11, 10, 8, 11, 9, 10,
         11, 11, 11, 11, 9, 10, 10, 12, 10, 11, 9, 10, 9, 8, 9, 9,
         9, 9, 7, 8, 8, 9, 8, 8, 7, 8, 9, 8, 9, 10, 69, 41,
         73, 74, 4, 9, 10, 9, 9, 9, 9, 9, 9, 9, 69, 7, 8, 9,
         11, 9, 9, 9, 10, 9, 9, 11, 9, 9, 11, 9, 11, 10, 70, 9,
         9, 13, 12, 13, 13, 70, 10, 6, 6, 11, 13, 10, 9, 72, 72, 72,
         72, 9, 9, 1, 1, 1, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },{ // Jap - 1
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 10,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 13, 13, 13, 13, 13, 11,
         13, 12, 10, 13, 13, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },{ // Jap - 2
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 13, 13, 13, 13,
         13, 13, 11, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 9, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 11, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 13, 12, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 11, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },{ // Jap - 3
         12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         10, 13, 13, 12, 13, 13, 13, 13, 13, 12, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },{ // Jap - 4
         13, 13, 13, 13, 13, 13, 13, 13, 11, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 11, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },{ // Jap - 5
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 11, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 12, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 11, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
         13, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

const char *FF7Font::optimisedDuo[3] =
{
    "\x0c\x00",//', '
    "\x0e\x02",//'."'
    "\xa9\x02" //'..."'
};
