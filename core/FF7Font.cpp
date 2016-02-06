/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2012 Arzel Jérôme <myst6re@gmail.com>
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
#include "Parameters.h"
#include "Config.h"

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
	if(tableId < _tables.size() && charId < 224) {
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
	if(_name.isEmpty()) {
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
	QStringList lines = data.split(QRegExp("\\s*(\\r\\n|\\r|\\n)\\s*"), QString::SkipEmptyParts);
	QRegExp nameRegExp("#NAME\\s+(\\S.*)"), letterRegExp("\\s*\"([^\"]*|\\\"*)\"\\s*,?\\s*");
	// nameRegExp:		#NAME blah blah blah
	// letterRegExp:	"Foo", "Foo","Foo"
	QStringList table;

	if(tableCount < 1) {
		qWarning() << "invalid windowBinFile!";
		return;
	}

	foreach(const QString &line, lines) {
		if(line.startsWith("#")) {
			if(nameRegExp.indexIn(line) != -1) {
				QStringList capturedTexts = nameRegExp.capturedTexts();
				_name = capturedTexts.at(1).trimmed();
			}
		}
		else {
			int offset=0;
			while((offset = letterRegExp.indexIn(line, offset)) != -1) {
				QStringList capturedTexts = letterRegExp.capturedTexts();
				table.append(capturedTexts.at(1));
				offset += capturedTexts.first().size();

				if(table.size() == 224) {
					_tables.append(table);
					if(_tables.size() > tableCount) {
						//print();
						return;
					}
					table = QStringList();
				}
			}
		}
	}

	if(!table.isEmpty()) {
		if(table.size() < 224) {
			for(int i=table.size() ; i<224 ; ++i) {
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

	if(!_name.isEmpty()) {
		data.append("#NAME\t").append(_name).append("\n");
	}

	foreach(const QStringList &t, _tables) {
		for(int j=0 ; j<14 ; ++j) {
			for(int i=0 ; i<16 ; ++i) {
				data.append(QString("\"%1\"").arg(t[j*16 + i]));
				if(i<15) {
					data.append(",");
				}
			}
			if(j<13) {
				data.append(",");
			}
			data.append("\n");
		}
		data.append("\n");
	}

	return data;
}

void FF7Font::print()
{
	int tid=1;
	foreach(const QStringList &t, _tables) {
		qDebug() << QString("table %1").arg(tid++).toLatin1().data();
		for(int j=0 ; j<14 ; ++j) {
			QString buf;
			for(int i=0 ; i<16 ; ++i) {
				buf += QString("\"%1\",").arg(t[j*16 + i]);
			}
			qDebug() << buf.toLatin1().data();
		}
	}
}

QMap<QString, FF7Font *> FF7Font::fonts;
QString FF7Font::font_dirPath;

bool FF7Font::listFonts()
{
	fonts.clear();

#ifdef Q_OS_WIN
	font_dirPath = qApp->applicationDirPath();
//	font_dirPath = QDir::cleanPath(QSettings(QSettings::IniFormat, QSettings::UserScope, PROG_NAME, PROG_NAME).fileName());
//	font_dirPath = font_dirPath.left(font_dirPath.lastIndexOf('/'));
#else
	font_dirPath = QDir::cleanPath(QSettings(PROG_NAME, PROG_NAME).fileName());
	font_dirPath = font_dirPath.left(font_dirPath.lastIndexOf('/'));
#endif

	QDir dir(font_dirPath);
	dir.mkpath(font_dirPath);
	QStringList stringList = dir.entryList(QStringList("*.windowBinFile"), QDir::Files, QDir::Name);

	FF7Font *latinFont = openFont(":/fonts/sysfnt.windowBinFile", ":/fonts/sysfnt.txt");
	FF7Font *jpFont = openFont(":/fonts/sysfnt_jp.windowBinFile", ":/fonts/sysfnt_jp.txt");

	if(!latinFont || !jpFont)	return false;

	latinFont->setReadOnly(true);
	jpFont->setReadOnly(true);

	fonts.insert("00", latinFont);
	fonts.insert("01", jpFont);

	foreach(const QString &str, stringList) {
		int index = str.lastIndexOf('.');
		fonts.insert(str.left(index), NULL);
	}

	return true;
}

QStringList FF7Font::fontList()
{
	return fonts.keys();
}

FF7Font *FF7Font::openFont(const QString &windowBinFilePath, const QString &txtPath)
{
	FF7Font *ff7Font = NULL;
	WindowBinFile *windowBinFile = NULL;
	QFile f(windowBinFilePath);
	if(f.open(QIODevice::ReadOnly)) {
		windowBinFile = new WindowBinFile();
		if(!windowBinFile->open(f.readAll())) {
			qWarning() << "Cannot open windowBinFile file!" << f.fileName();
			delete windowBinFile;
			windowBinFile = NULL;
		}
		f.close();
	}

	if(!windowBinFile) {
		return NULL;
	} else {
		QFile f2(txtPath);
		if(f2.open(QIODevice::ReadOnly)) {
			ff7Font = new FF7Font(windowBinFile, f2.readAll());
			f2.close();
		} else {
			ff7Font = new FF7Font(windowBinFile, QByteArray());
		}
		ff7Font->setPaths(txtPath, windowBinFilePath);
		return ff7Font;
	}
}

FF7Font *FF7Font::font(QString name)
{
	if(name.isEmpty()) {
		name = "00";
	}

	if(fonts.contains(name)) {
		FF7Font *ff7Font = fonts.value(name);
		if(!ff7Font) {
			ff7Font = openFont(font_dirPath + "/" + name + ".windowBinFile", font_dirPath + "/" + name + ".txt");
			if(!ff7Font) {
				fonts.remove(name);// Bad font, we can remove it
			} else {
				fonts.insert(name, ff7Font);
			}
		}
		return ff7Font;
	}

	return NULL;
}

FF7Font *FF7Font::getCurrentConfigFont()
{
	QString fnt = Config::value("encoding", "00").toString();

	QStringList fontL = fontList();
	if(fontL.contains(fnt)) {
		return font(fnt);
	}
	return font(fontL.first());
}

bool FF7Font::saveFonts()
{
	bool ok = true;

	foreach(FF7Font *font, fonts) {
		if(font && !font->isReadOnly() && font->isModified()) {
			QFile f1(font->txtPath());
			if(f1.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
				f1.write(font->saveTxt().toUtf8());
				f1.close();
			} else {
				ok = false;
			}
			QFile f2(font->windowBinFilePath());
			if(f2.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
				//TODO
				/*QByteArray tdwData;
				if(font->windowBinFile()->save(tdwData)) {
					f2.write(tdwData);
				} else {
					ok = false;
				}*/
				f2.close();
			} else {
				ok = false;
			}
			if(ok) {
				font->setModified(false);
			}
		}
	}

	return ok;
}

bool FF7Font::copyFont(const QString &name, const QString &from, const QString &name2)
{
	if(fonts.contains(name) || !fonts.contains(from)) {
		return false;
	}

	FF7Font *ff7Font = font(from);
	if(!ff7Font) {
		return false;
	}

	QFile ftxt(font_dirPath + "/" + name + ".txt");
	if(!ftxt.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		return false;
	}

	QFile ftxt2(ff7Font->txtPath());
	if(!ftxt2.open(QIODevice::ReadOnly)) {
		return false;
	}

	ftxt.write(ftxt2.readAll());

	ftxt.close();
	ftxt2.close();

	QFile ftdw(font_dirPath + "/" + name + ".windowBinFile");
	if(!ftdw.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		return false;
	}

	QFile ftdw2(ff7Font->windowBinFilePath());
	if(!ftdw2.open(QIODevice::ReadOnly)) {
		return false;
	}

	ftdw.write(ftdw2.readAll());

	ftdw.close();
	ftdw2.close();

	fonts.insert(name, NULL);

	ff7Font = font(name);
	if(!ff7Font) {
		return false;
	}

	ff7Font->setName(name2);
	ff7Font->setReadOnly(false);

	return true;
}

bool FF7Font::removeFont(const QString &name)
{

	FF7Font *ff7Font = font(name);

	if(!ff7Font || ff7Font->isReadOnly()) {
		return false;
	}

	if(!QFile::remove(ff7Font->txtPath())
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
