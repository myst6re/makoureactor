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
#ifndef DATA_H
#define DATA_H

#include <QtCore>
#include "core/Lgp.h"
#include "core/WindowBinFile.h"
#include "core/field/CharArchive.h"

#define FF7_WIN_REGISTER_PATH "Square Soft, Inc./Final Fantasy VII"

class Data
{
public:
	enum FF7Version {
		Standard, Rerelease, Steam, Custom
	};

	static void refreshFF7Paths();
	static const QString &ff7DataPath();
	static const QString &ff7AppPath();
	static QMap<FF7Version, QString> ff7AppPathList();
	static QMap<FF7Version, QString> ff7DataPathList();
	static QString ff7KernelPath();
	static QString charlgp_path();
	static WindowBinFile windowBin;
	static int loadKernel2Bin();
	static int loadWindowBin();
	static bool load();
	static bool openMaplist(const QByteArray &data);
	static void openMaplist(bool PC=false);
	static QStringList char_names;
	static QStringList key_names;
	static QStringList item_names;
	static QStringList weapon_names;
	static QStringList armor_names;
	static QStringList accessory_names;
	static QStringList materia_names;
//	static QStringList currentCharNames;
	static int currentModelID;
	static QStringList *currentHrcNames;
	static QList<QStringList> *currentAnimNames;
	static QStringList field_names;
	static QStringList movie_names_cd1, movie_names_cd2, movie_names_cd3;
	static QStringList music_names;
	static QStringList music_desc;
	static QString regValue(const QString &regPath, const QString &regKey);
	static inline QString regValuePath(const QString &regPath, const QString &regKey) {
		return QDir::fromNativeSeparators(QDir::cleanPath(regValue(regPath, regKey)));
	}
private:
	static const QString &searchRereleasedFF7Path();
	static QString searchSteamFF7Path();
	static QString searchFF7Exe(FF7Version version);
	static QString searchFF7DataPath(FF7Version version);
	static QString ff7Path(const QMap<FF7Version, QString> &pathList);
	static QMap<Data::FF7Version, QString> ff7PathList(QString (*searchFF7Path)(FF7Version));
	static void fill(const QByteArray &data, int pos, int dataSize, QStringList &names);
	static QString ff7DataPath_cache;
	static QString ff7AppPath_cache;
	static QString ff7RereleasePath_cache;
	static bool ff7RereleaseAlreadySearched;
	static const char *movieList[106];
	static const char *mapList[788];
	static const char *musicList[100];
	static const char *musicList2[100];

};

#endif // DATA_H
