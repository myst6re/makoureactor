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
#include <QtGlobal>
#ifdef Q_OS_WIN
#include <windows.h>
#include <winbase.h>
// #include <winerror.h>
#include <winreg.h>
#endif
#include <LZS.h>
#include "Data.h"
#include "core/Config.h"
#include "core/FF7Text.h"

QStringList Data::char_names;
QStringList Data::item_names;
QStringList Data::weapon_names;
QStringList Data::armor_names;
QStringList Data::accessory_names;
QStringList Data::materia_names;
//QStringList Data::currentCharNames;
int Data::currentModelID=-1;
QStringList *Data::currentHrcNames=0;
QList<QStringList> *Data::currentAnimNames=0;
QStringList Data::field_names;
QStringList Data::movie_names_cd1;
QStringList Data::movie_names_cd2;
QStringList Data::movie_names_cd3;
QStringList Data::music_names;
QStringList Data::music_desc;
QStringList Data::key_names;
QString Data::ff7DataPath_cache;
QString Data::ff7AppPath_cache;
QString Data::ff7RereleasePath_cache;
bool Data::ff7RereleaseAlreadySearched = false;
WindowBinFile Data::windowBin;

void Data::refreshFF7Paths()
{
	ff7AppPath_cache = ff7DataPath_cache = QString();
}

QString Data::regValue(const QString &regPath, const QString &regKey,
                       RegTarget target, RegLocation loc)
{
#ifdef Q_OS_WIN
	HKEY phkResult;
	LONG error;
	REGSAM flags = KEY_READ;
	HKEY hkey = HKEY_LOCAL_MACHINE;

	if (target == Target32) {
#ifdef KEY_WOW64_32KEY
		flags |= KEY_WOW64_32KEY; // if you compile in 64-bit, force reg search into 32-bit entries
#endif
	} else if (target == Target64) {
#ifdef KEY_WOW64_64KEY
		flags |= KEY_WOW64_64KEY;
#endif
	}

	// Open regPath relative to HKEY_LOCAL_MACHINE or HKEY_CURRENT_USER
	if (loc == LocationUser) {
		hkey = HKEY_CURRENT_USER;
	}
	error = RegOpenKeyEx(hkey, QDir::toNativeSeparators("SOFTWARE/" % regPath).toStdString().c_str(), 0, flags, &phkResult);
	if (ERROR_SUCCESS == error) {
		BYTE value[MAX_PATH];
		DWORD cValue = MAX_PATH, type;

		// Open regKey which must is a string value (REG_SZ)
		RegQueryValueEx(phkResult, regKey.toStdString().c_str(), nullptr, &type, value, &cValue);
		if (ERROR_SUCCESS == error && type == REG_SZ) {
			RegCloseKey(phkResult);
			return QString::fromUtf16((ushort *)value);
		}
		RegCloseKey(phkResult);
	}
#else
	Q_UNUSED(regPath)
	Q_UNUSED(regKey)
	Q_UNUSED(target)
	Q_UNUSED(loc)
#endif

	return QString();
}

QString Data::regValue(const QString &regPath, const QString &regKey,
                       RegLocation loc)
{
	QString ret;

#if defined(KEY_WOW64_32KEY) || defined(KEY_WOW64_64KEY)
	ret = regValue(regPath, regKey, Target32, loc);
	if (ret.isEmpty()) {
		ret = regValue(regPath, regKey, Target64, loc);
	}
#else
	ret = regValue(regPath, regKey, TargetNone, loc);
#endif

	return ret;
}

const QString &Data::searchRereleasedFF7Path()
{
#ifdef Q_OS_WIN
	if (ff7RereleasePath_cache.isNull() && !ff7RereleaseAlreadySearched) {
		ff7RereleaseAlreadySearched = true;
		HKEY phkResult, phkResult2;
		LONG error;
		REGSAM flags = KEY_READ;

#ifdef KEY_WOW64_64KEY
		flags |= KEY_WOW64_64KEY; // if you compile in 64-bit, force reg search into 64-bit entries
#endif

		// direct
		ff7RereleasePath_cache = regValuePath("Microsoft/Windows/CurrentVersion/Uninstall/{141B8BA9-BFFD-4635-AF64-078E31010EC3}_is1", "InstallLocation");
		if (!ff7RereleasePath_cache.isEmpty()) {
			return ff7RereleasePath_cache;
		}

		// if another id
		error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"), 0, flags, &phkResult);

		if (ERROR_SUCCESS == error) {
			DWORD index = 0;
			CHAR subKeyName[MAX_PATH];
			DWORD subKeyCName = MAX_PATH;
			while (ERROR_NO_MORE_ITEMS != (error = RegEnumKeyEx(phkResult, index, subKeyName, &subKeyCName, nullptr, nullptr, nullptr, nullptr))) {
				QString subKeyNameStr = QString::fromUtf16((ushort *)subKeyName);
				if (subKeyNameStr.endsWith("_is1")) {
					error = RegOpenKeyEx(phkResult, QString("%1\\").arg(subKeyNameStr).toStdString().c_str(), 0, KEY_READ, &phkResult2);
					if (ERROR_SUCCESS == error) {
						BYTE value[MAX_PATH];
						DWORD cValue = MAX_PATH, type;
						error = RegQueryValueEx(phkResult2, TEXT("DisplayName"), nullptr, &type, value, &cValue);
						if (ERROR_SUCCESS == error) {
							if (type == REG_SZ) {
								QString softwareNameStr = QString::fromUtf16((ushort *)value);
								if (softwareNameStr.compare("FINAL FANTASY VII", Qt::CaseInsensitive) == 0) {
									cValue = MAX_PATH;
									error = RegQueryValueEx(phkResult2, TEXT("InstallLocation"), nullptr, &type, value, &cValue);
									if (ERROR_SUCCESS == error) {
										if (type == REG_SZ) {
											RegCloseKey(phkResult2);
											RegCloseKey(phkResult);
											ff7RereleasePath_cache = QDir::fromNativeSeparators(QDir::cleanPath(QString::fromUtf16((ushort *)value)));
											return ff7RereleasePath_cache;
										}
									}
								}
							}
						}


					}
					RegCloseKey(phkResult2);
				}
				++index;
				subKeyCName = MAX_PATH;
			}

			RegCloseKey(phkResult);
		}
	}
#endif
	return ff7RereleasePath_cache;
}

QString Data::searchSteamFF7Path()
{
	QString ret = regValuePath("Microsoft/Windows/CurrentVersion/Uninstall/Steam App 39140", "InstallLocation");
	if (ret.isEmpty() || !QFile::exists(ret)) {
		QString steamPath = regValue("Valve/Steam", "SteamPath", LocationUser);
		return steamPath % "/steamapps/common/FINAL FANTASY VII";
	}

	return ret;
}

QString Data::searchFF7Exe(FF7Version version)
{
	QString ff7Path;

	switch (version) {
	case Standard:
		ff7Path = regValuePath(FF7_WIN_REGISTER_PATH, "AppPath");
		if (!ff7Path.isEmpty() && QFile::exists(ff7Path + "/ff7.exe")) {
			return ff7Path + "/ff7.exe";
		}
		return QString();
	case Rerelease:
		ff7Path = searchRereleasedFF7Path();
		if (!ff7Path.isEmpty() && QFile::exists(ff7Path + "/FF7_Launcher.exe")) {
			return ff7Path + "/FF7_Launcher.exe";
		}
		return QString();
	case Steam:
		ff7Path = searchSteamFF7Path();
		if (!ff7Path.isEmpty() && QFile::exists(ff7Path + "/FF7_Launcher.exe")) {
			return ff7Path + "/FF7_Launcher.exe";
		}
		return QString();
	case Custom:
		return Config::value("customFF7Path").toString();
	}

	/*
	QString ff7MusicPath = regValuePath("FF7Music", "InstDir");
	if (!ff7MusicPath.isEmpty() && QFile::exists(ff7MusicPath + "/launchff7.exe")) {
		return ff7MusicPath + "/launchff7.exe";
	}
	*/
	return QString();
}

QString Data::searchFF7DataPath(FF7Version version)
{
	QString dataPath, ff7Path;

	switch (version) {
	case Standard:
		dataPath = regValuePath(FF7_WIN_REGISTER_PATH, "DataPath");
		if (!dataPath.isEmpty() && QFile::exists(dataPath)) {
			return dataPath;
		}
		return QString();
	case Rerelease:
		ff7Path = searchRereleasedFF7Path();
		if (!ff7Path.isEmpty() && QFile::exists(ff7Path + "/data")) {
			return ff7Path + "/data";
		}
		return QString();
	case Steam:
		ff7Path = searchSteamFF7Path();
		if (!ff7Path.isEmpty() && QFile::exists(ff7Path + "/data")) {
			return ff7Path + "/data";
		}
		return QString();
	case Custom:
		ff7Path = Config::value("customFF7Path").toString();
		ff7Path = ff7Path.left(ff7Path.lastIndexOf('/'));
		if (QFile::exists(ff7Path + "/data")) {
			return ff7Path + "/data";
		}
	}

	return QString();
}

QMap<Data::FF7Version, QString> Data::ff7PathList(QString (*searchFF7Path)(FF7Version))
{
	QMap<FF7Version, QString> ff7List;

	foreach(FF7Version version,
			QList<FF7Version>() << Standard << Rerelease << Steam << Custom) {
		QString dataPath = (*searchFF7Path)(version);
		if (!dataPath.isEmpty()) {
			ff7List.insert(version, dataPath);
		}
	}

	return ff7List;
}

QMap<Data::FF7Version, QString> Data::ff7AppPathList()
{
	return ff7PathList(searchFF7Exe);
}

QMap<Data::FF7Version, QString> Data::ff7DataPathList()
{
	return ff7PathList(searchFF7DataPath);
}

QString Data::ff7Path(const QMap<FF7Version, QString> &pathList)
{
	FF7Version version = FF7Version(Config::value("FF7ExePathToUse", 0).toInt());

	if (!pathList.contains(version)) {
		for (const QString &appPath : pathList) {
			return appPath;
		}
	} else {
		return pathList.value(version);
	}

	return QString();
}

const QString &Data::ff7AppPath()
{
	if (ff7AppPath_cache.isNull()) {
		QString appPath = ff7Path(ff7AppPathList());

		if (!appPath.isEmpty()) {
			ff7AppPath_cache = appPath;
		}
	}
	return ff7AppPath_cache;
}

const QString &Data::ff7DataPath()
{
	if (ff7DataPath_cache.isNull()) {
		QString appPath = ff7Path(ff7DataPathList());

		if (!appPath.isEmpty()) {
			ff7DataPath_cache = appPath;
		}
	}
	return ff7DataPath_cache;
}

QString Data::ff7KernelPath()
{
	QString path = ff7DataPath();
	if (!path.isEmpty()) {
		if (QFile::exists(path + "/kernel/KERNEL.BIN")) {
			path.append("/kernel");
		} else {
			QString lang = QLocale::system().name().toLower();
			lang = Config::value("lang", lang.left(lang.indexOf("_"))).toString().toLower();
			if (QFile::exists(path + QString("/lang-%1/kernel").arg(lang)))
				path.append(QString("/lang-%1/kernel").arg(lang));
			else {
				QStringList langs;
				langs << "de" << "en" << "es" << "fr" << "ja";
				for (const QString &lang : qAsConst(langs)) {
					if (QFile::exists(path + QString("/lang-%1/kernel").arg(lang))) {
						path.append(QString("/lang-%1/kernel").arg(lang));
						break;
					}
				}
			}
		}
	}

	return path;
}

QString Data::charlgp_path()
{
	QString charPath = Config::value("charPath").toString();
	if (charPath.isEmpty()) {
		if (Data::ff7DataPath().isEmpty())	return QString();
		charPath = Data::ff7DataPath()%"/field/char.lgp";
	}

	return charPath;
}

int Data::loadKernel2Bin()
{
	QString path = Config::value("kernel2Path").toString();
	if (path.isEmpty()) {
		path = ff7KernelPath();
		if (path.isEmpty()) {
			return 2;
		}
		path.append("/kernel2.bin");
	}

	QFile fic(path);
	if (fic.open(QIODevice::ReadOnly)) {
		quint32 fileSize;
		if (fic.read((char *)&fileSize, 4) != 4) {
			return 2;
		}

		if (fileSize + 4 != fic.size()) {
			return 2;
		}

		const QByteArray &data = LZS::decompressAll(fic.read(fileSize));
		const char *constData = data.constData();
		int pos = 0, dataSize = data.size();

		fic.close();

		for (int i=0; i<10; ++i) {
			if (pos + 4 >= dataSize) {
				return 1;
			}

			memcpy(&fileSize, constData + pos, 4);
			pos += fileSize + 4;
		}

		if (pos + 4 >= dataSize) {
			return 1;
		}

		memcpy(&fileSize, constData + pos, 4);
		pos += 4;

		if (pos + (int)fileSize > dataSize) {
			return 1;
		}

		item_names.clear();
		fill(data, pos, fileSize, item_names);
		pos += fileSize;

		if (pos + 4 > dataSize) {
			return 1;
		}

		memcpy(&fileSize, constData + pos, 4);
		pos += 4;

		if (pos + (int)fileSize > dataSize) {
			return 1;
		}

		weapon_names.clear();
		fill(data, pos, fileSize, weapon_names);
		pos += fileSize;

		if (pos + 4 > dataSize) {
			return 1;
		}

		memcpy(&fileSize, constData + pos, 4);
		pos += 4;

		if (pos + (int)fileSize > dataSize) {
			return 1;
		}

		armor_names.clear();
		fill(data, pos, fileSize, armor_names);
		pos += fileSize;

		if (pos + 4 > dataSize) {
			return 1;
		}

		memcpy(&fileSize, constData + pos, 4);
		pos += 4;

		if (pos + (int)fileSize > dataSize) {
			return 1;
		}

		accessory_names.clear();
		fill(data, pos, fileSize, accessory_names);
		pos += fileSize;

		if (pos + 4 > dataSize) {
			return 1;
		}

		memcpy(&fileSize, constData + pos, 4);
		pos += 4;

		if (pos + (int)fileSize > dataSize) {
			return 1;
		}

		materia_names.clear();
		fill(data, pos, fileSize, materia_names);
	}

	return 0;
}

int Data::loadWindowBin()
{
	QString path = Config::value("windowBinPath").toString();
	if (path.isEmpty()) {
		path = ff7KernelPath();
		if (path.isEmpty()) {
			return 1;
		}
		path.append("/window.bin");
	}

	if (!windowBin.open(path)) {
		qWarning() << "Cannot open" << path;
		return 2;
	}

	return 0;
}

bool Data::load()
{
	if (char_names.isEmpty()) {
		char_names
				<< QObject::tr("Cloud") << QObject::tr("Barret") << QObject::tr("Tifa")
				<< QObject::tr("Aerith") << QObject::tr("Red XIII") << QObject::tr("Yuffie")
				<< QObject::tr("Cait Sith") << QObject::tr("Vincent") << QObject::tr("Cid")
				<< QObject::tr("Yound Cloud") << QObject::tr("Sephiroth") << QObject::tr("Chocobo");
	}

	if (key_names.isEmpty()) {
		key_names
				<< QObject::tr("[CAMERA|L2]") << QObject::tr("[TARGET|R2]")
				<< QObject::tr("[PAGE UP|L1]") << QObject::tr("[PAGE DOWN|R1]")
				<< QObject::tr("[MENU|TRIANGLE]") << QObject::tr("[OK|CIRCLE]")
				<< QObject::tr("[CANCEL|CROSS]") << QObject::tr("[SWITCH|SQUARE]")
				<< QObject::tr("[ASSIST|SELECT]") << QString("[???]")
				<< QString("[???]") << QObject::tr("[START]")
				<< QObject::tr("[UP]") << QObject::tr("[RIGHT]")
				<< QObject::tr("[DOWN]") << QObject::tr("[LEFT]");
	}

	if (movie_names_cd1.isEmpty()) {
		QStringList movie_names_common;
		for (int i=0; i<20; ++i) {
			movie_names_common.append(movieList[i]);
		}

		movie_names_cd1.append(movie_names_common);
		movie_names_cd2.append(movie_names_common);
		movie_names_cd3.append(movie_names_common);
		for (int i=20; i<54; ++i) {
			movie_names_cd1.append(movieList[i]);
		}
		for (int i=54; i<96; ++i) {
			movie_names_cd2.append(movieList[i]);
		}
		for (int i=96; i<106; ++i) {
			movie_names_cd3.append(movieList[i]);
		}
	}

	if (music_names.isEmpty()) {
		for (int i=0; i<100; ++i) {
			music_names.append(musicList[i]);
			music_desc.append(musicList2[i]);
		}
	}

	bool ok = true;

	if (loadKernel2Bin() != 0) {
		ok = false;
	}

	if (loadWindowBin() != 0) {
		ok = false;
	}

	return ok;
}

void Data::fill(const QByteArray &data, int pos, int dataSize, QStringList &names)
{
	int i, count;
	quint16 position, lastPosition=0;
	const char *constData = data.constData();
	QList<quint16> positions;

	if (dataSize < 2) {
		return;
	}

	memcpy(&position, constData + pos, 2);

	count = position / 2;

	if (dataSize < position) {
		return;
	}

	for (i=0; i<count; ++i) {
		memcpy(&position, constData + pos + i*2, 2);
		if (position >= dataSize || lastPosition > position) {
			return;
		}
		positions.append(position);
		lastPosition = position;
	}
	positions.append(dataSize);

	i=0;
	for (quint16 position : positions) {
		names.append(FF7Text(data.mid(pos + position, positions.at(i+1) - position)).text(false, true));
		++i;
		if (i == count)	break;
	}
}

bool Data::openMaplist(const QByteArray &data)
{
	if (data.size() < 2) {
		return false;
	}

	quint16 nbMap;
	memcpy(&nbMap, data.constData(), 2);

	if (data.size() != 2+nbMap*32) {
		return false;
	}

	field_names.clear();
	for (int i=0; i<nbMap; ++i) {
		field_names.append(QString(data.mid(2+i*32, 32)).simplified());
	}
	// Remove empty entries at the end
	while (!field_names.isEmpty() && field_names.last().isEmpty()) {
		field_names.removeLast();
	}

	return true;
}

bool Data::saveMaplist(QByteArray &data)
{
	if (field_names.size() > 65535) {
		return false;
	}
	quint16 nbMap = field_names.size();
	data.append((char *)&nbMap, 2);

	for (const QString &fieldName : qAsConst(field_names)) {
		data.append(fieldName.toLatin1()
		            .leftJustified(32, '\0', true));
	}

	return true;
}

const char *Data::movieList[106] = {
	"fship2", "fship2n", "d_ropego", "d_ropein", "u_ropein", "u_ropego",
	"gold2", "gold3", "gold4", "gold6", "gold5",
	"boogup", "boogdown", "junair_u", "junair_d", "junelein", "junelego", "junin_in", "junin_go",
	"moriya", /*20*/"mkup", "northmk", "mk8", "ontrain", "mainplr", "smk", "southmk", "plrexp", "fallpl",
	"monitor", "bike", "mtnvl", "mtnvl2", "brgnvl", "nvlmk", "nivlsfs", "jenova_e",
	"junon", "hiwind0", "mtcrl", "gold1", "biskdead", "boogdemo", "boogstar", "setogake",
	"rcktfail", "jairofly", "jairofal", "gold7", "gold7_2", "earithdd", "funeral",
	"car_1209", "opening", /*ENDCD1*//*20*/"greatpit", "c_scene1", "c_scene2", "c_scene3", "biglight",
	"meteosky", "weapon0", "weapon1", "weapon2", "weapon3", "weapon4", "weapon5", "hwindfly",
	"phoenix", "nrcrl", "nrcrl_b", "dumcrush", "zmind01", "zmind02", "zmind03",
	"gelnica", "rcketoff", "white2", "junsea", "rckethit0", "rckethit1", "meteofix",
	"canonon", "feelwin0", "feelwin1", "canonht1", "canonht2", "canonh3f",
	"parashot", "hwindjet", "canonht0", "wh2e2", "loslake1", "lslmv", "canonh1p",
	"canon", "", /*ENDCD2*//*20*/"last4_2", "last4_3", "last4_4", "lastmap", "lastflor",
	"ending1", "ending3", "fcar", "white2", "ending2"
};

const char *Data::_mapList[788] = {
	"dummy", "wm0", "wm1", "wm2", "wm3", "wm4", "wm5", "wm6",
	"wm7", "wm8", "wm9", "wm10", "wm11", "wm12", "wm13", "wm14",
	"wm15", "wm16", "wm17", "wm18", "wm19", "wm20", "wm21", "wm22",
	"wm23", "wm24", "wm25", "wm26", "wm27", "wm28", "wm29", "wm30",
	"wm31", "wm32", "wm33", "wm34", "wm35", "wm36", "wm37", "wm38",
	"wm39", "wm40", "wm41", "wm42", "wm43", "wm44", "wm45", "wm46",
	"wm47", "wm48", "wm49", "wm50", "wm51", "wm52", "wm53", "wm54",
	"wm55", "wm56", "wm57", "wm58", "wm59", "wm60", "wm61", "wm62",
	"wm63", "startmap", "fship_1", "fship_12", "fship_2", "fship_22",
	"fship_23", "fship_24", "fship_25", "fship_3", "fship_4", "fship_42",
	"fship_5", "hill", "zz1", "zz2", "zz3", "zz4", "zz5", "zz6",
	"zz7", "zz8", "sea", "sky", "q_1"/*qa*/, "q_2"/*qb*/, "q_3"/*qc*/, "q_4"/*qd*/, "q_5"/*qe*/,
	"blackbg1", "blackbg2", "blackbg3", "blackbg4", "blackbg5", "blackbg6",
	"blackbg7", "blackbg8", "blackbg9", "blackbga", "blackbgb", "blackbgc",
	"blackbgd", "blackbge", "blackbgf", "blackbgg", "blackbgh", "blackbgi",
	"blackbgj", "blackbgk", "whitebg1", "whitebg2", "whitebg3", "md1stin",
	"md1_1", "md1_2", "nrthmk", "nmkin_1", "elevtr1", "nmkin_2", "nmkin_3",
	"nmkin_4", "nmkin_5", "southmk1", "southmk2", "smkin_1", "smkin_2",
	"smkin_3", "smkin_4", "smkin_5", "md8_1", "md8_2", "md8_3", "md8_4",
	"md8brdg", "cargoin", "tin_1", "tin_2", "tin_3", "tin_4", "rootmap",
	"mds7st1", "mds7st2", "mds7st3", "mds7st32", "mds7_w1", "mds7_w2",
	"mds7_w3", "mds7", "mds7_im", "7min1"/*min71*/, "mds7pb_1", "mds7pb_2",
	"mds7plr1", "mds7plr2", "pillar_1", "pillar_2", "pillar_3", "tunnel_1",
	"tunnel_2", "tunnel_3", "4sbwy_1"/*sbwy4_1*/, "4sbwy_2"/*sbwy4_2*/, "4sbwy_3"/*sbwy4_3*/, "4sbwy_4"/*sbwy4_4*/,
	"4sbwy_5"/*sbwy4_5*/, "4sbwy_6"/*sbwy4_6*/, "mds5_5", "mds5_4", "mds5_3", "mds5_2",
	"5min1_1"/*min51_1*/, "5min1_2"/*min51_2*/, "mds5_dk", "mds5_1", "mds5_w", "mds5_i",
	"mds5_m", "church", "chrin_1a", "chrin_1b", "chrin_2", "chrin_3a",
	"chrin_3b", "eals_1", "ealin_1", "ealin_12", "ealin_2", "mds6_1", "mds6_2",
	"mds6_22", "mds6_3", "mrkt2", "mkt_w", "mkt_mens", "mkt_ia", "mktinn",
	"mkt_m", "mkt_s1", "mkt_s2", "mkt_s3", "mktpb", "mrkt1", "colne_1",
	"colne_2", "colne_3", "colne_4", "colne_5", "colne_6", "colne_b1",
	"colne_b3", "mrkt3", "onna_1", "onna_2", "onna_3", "onna_4", "onna_5",
	"onna_52", "onna_6", "mrkt4", "wcrimb_1", "wcrimb_2", "md0", "roadend",
	"sinbil_1", "sinbil_2", "blinst_1", "blinst_2", "blinst_3", "blinele",
	"eleout", "blin1", "blin2", "blin2_i", "blin3_1", "blin59", "blin60_1",
	"blin60_2", "blin61", "blin62_1", "blin62_2", "blin62_3", "blin63_1",
	"blin63_t", "blin64", "blin65_1", "blin65_2", "blin66_1", "blin66_2",
	"blin66_3", "blin66_4", "blin66_5", "blin66_6", "blin67_1", "blin671b",
	"blin67_2", "blin67_3", "blin673b", "blin67_4", "blin68_1", "blin68_2",
	"blin69_1", "blin69_2", "blin70_1", "blin70_2", "blin70_3", "blin70_4",
	"niv_w", "nvmin1_1", "nvmin1_2", "nivinn_1", "nivinn_2", "nivinn_3",
	"niv_cl", "trackin", "trackin2", "nivgate", "nivgate2", "nivgate3",
	"nivl", "nivl_2", "nivl_3", "nivl_4", "niv_ti1", "niv_ti2", "niv_ti3",
	"niv_ti4", "nivl_b1", "nivl_b12", "nivl_b2", "nivl_b22", "nivl_e1",
	"nivl_e2", "nivl_e3", "sinin1_1", "sinin1_2", "sinin2_1", "sinin2_2",
	"sinin3", "sininb1", "sininb2", "sininb31", "sininb32", "sininb33",
	"sininb41", "sininb42", "sininb51", "sininb52", "mtnvl2", "mtnvl3",
	"mtnvl4", "mtnvl5", "mtnvl6", "mtnvl6b", "nvdun1", "nvdun2", "nvdun3",
	"nvdun31", "nvdun4", "nvmkin1", "nvmkin21", "nvmkin22", "nvmkin23",
	"nvmkin31", "nvmkin32", "elm_wa", "elm_i", "elmpb", "elminn_1", "elminn_2",
	"elmin1_1", "elmin1_2", "elm", "elmin2_1", "elmin2_2", "elmin3_1",
	"elmin3_2", "elmtow", "elmin4_1", "elmin4_2", "farm", "frmin", "frcyo",
	"trap", "fr_e", "sichi", "psdun_1", "psdun_2", "psdun_3", "psdun_4",
	"condor1", "condor2", "convil_1", "convil_2", "convil_3", "convil_4",
	"junon", "junonr1", "junonr2", "junonr3", "junonr4", "jun_wa", "jun_i1",
	"jun_m", "junmin1", "junmin2", "junmin3", "junonl1", "junonl2", "junonl3",
	"jun_w", "jun_a", "jun_i2", "juninn", "junpb_1", "junpb_2", "junpb_3",
	"junmin4", "junmin5", "jundoc1a", "jundoc1b", "junair", "junair2",
	"junin1", "junin1a", "junele1", "junin2", "junin3", "junele2",
	"junin4", "junin5", "junin6", "junin7", "junbin1", "junbin12",
	"junbin21", "junbin22", "junbin3", "junbin4", "junbin5", "junmon",
	"junsbd1", "subin_1a", "subin_1b", "subin_2a", "subin_2b", "subin_3",
	"subin_4", "junone2", "junone3", "junone4", "junone5", "junone6", "junone7",
	"spgate", "spipe_1", "spipe_2", "semkin_1", "semkin_2", "semkin_8", "semkin_3",
	"semkin_4", "semkin_5", "semkin_6", "semkin_7", "ujunon1", "ujunon2", "ujunon3",
	"prisila", "ujun_w", "jumin", "ujunon4", "ujunon5", "ship_1", "ship_2", "shpin_22",
	"shpin_2", "shpin_3", "del1", "del12", "del2", "delinn", "delpb", "delmin1",
	"delmin12", "delmin2", "del3", "ncorel", "ncorel2", "ncorel3", "ncoin1",
	"ncoin2", "ncoin3", "ncoinn", "ropest", "mtcrl_0", "mtcrl_1", "mtcrl_2",
	"mtcrl_3", "mtcrl_4", "mtcrl_5", "mtcrl_6", "mtcrl_7", "mtcrl_8", "mtcrl_9",
	"corel1", "corel2", "corel3", "jail1", "jailin1", "jail2", "jailpb", "jailin2",
	"jailin3", "jailin4", "jail3", "jail4", "dyne", "desert1", "desert2", "corelin",
	"astage_a", "astage_b", "jet", "jetin1", "bigwheel", "bwhlin", "bwhlin2", "ghotel",
	"ghotin_1", "ghotin_4", "ghotin_2", "ghotin_3", "gldst", "gldgate", "gldinfo",
	"coloss", "coloin1", "coloin2", "clsin2_1", "clsin2_2", "clsin2_3", "games", "games_1",
	"games_2", "mogu_1", "chorace", "chorace2", "crcin_1", "crcin_2", "gldelev", "gonjun1",
	"gonjun2", "gnmkf", "gnmk", "gongaga", "gon_wa1", "gon_wa2", "gon_i", "gninn", "gomin",
	"goson", "cos_btm", "cos_btm2", "cosmo", "cosmo2", "cosin1", "cosin1_1", "cosin2",
	"cosin3", "cosin4", "cosin5", "cosmin2", "cosmin3", "cosmin4", "cosmin6", "cosmin7",
	"cos_top", "bugin1a", "bugin1b", "bugin1c", "bugin2", "bugin3", "gidun_1", "gidun_2",
	"gidun_4", "gidun_3", "seto1", "rckt2", "rckt3", "rkt_w", "rkt_i", "rktinn1", "rktinn2",
	"rckt", "rktsid", "rktmin1", "rktmin2", "rcktbas1", "rcktbas2", "rcktin1", "rcktin2",
	"rcktin3", "rcktin4", "rcktin5", "rcktin6", "rcktin7", "rcktin8", "pass", "yougan",
	"yougan2", "yougan3", "uta_wa", "uta_im", "utmin1", "utmin2", "uutai1", "utapb",
	"yufy1", "yufy2", "hideway1", "hideway2", "hideway3", "5tower"/*tower5*/, "uutai2", "uttmpin1",
	"uttmpin2", "uttmpin3", "uttmpin4", "datiao_1", "datiao_2", "datiao_3", "datiao_4",
	"datiao_5", "datiao_6", "datiao_7", "datiao_8", "jtempl", "jtemplb", "jtmpin1",
	"jtmpin2", "kuro_1", "kuro_2", "kuro_3", "kuro_4", "kuro_5", "kuro_6", "kuro_7",
	"kuro_8", "kuro_82", "kuro_9", "kuro_10", "kuro_11", "kuro_12", "bonevil", "slfrst_1",
	"slfrst_2", "anfrst_1", "anfrst_2", "anfrst_3", "anfrst_4", "anfrst_5", "sango1",
	"sango2", "sango3", "sandun_1", "sandun_2", "lost1", "losin1", "losin2", "losin3",
	"lost2", "lost3", "losinn", "loslake1", "loslake2", "loslake3", "blue_1", "blue_2",
	"white1", "white2", "hekiga", "whitein", "ancnt1", "ancnt2", "ancnt3", "ancnt4",
	"snw_w", "sninn_1", "sninn_2", "sninn_b1", "snow", "snmin1", "snmin2", "snmayor",
	"hyou1", "hyou2", "hyou3", "icedun_1", "icedun_2", "hyou4", "hyou5_1", "hyou5_2",
	"hyou5_3", "hyou5_4", "hyou6", "hyoumap", "move_s", "move_i", "move_f", "move_r",
	"move_u", "move_d", "hyou7", "hyou8_1", "hyou8_2", "hyou9", "hyou10", "hyou11",
	"hyou12", "hyou13_1", "hyou13_2", "hyou14", "gaiafoot", "holu_1", "holu_2",
	"gaia_1", "gaiin_1", "gaiin_2", "gaia_2", "gaiin_3", "gaia_31", "gaia_32",
	"gaiin_4", "gaiin_5", "gaiin_6", "gaiin_7", "crater_1", "crater_2", "trnad_1",
	"trnad_2", "trnad_3", "trnad_4", "trnad_51", "trnad_52", "trnad_53", "woa_1",
	"woa_2", "woa_3", "itown1a", "itown12", "itown1b", "itown2", "ithill", "itown_w",
	"itown_i", "itown_m", "ithos", "itmin1", "itmin2", "life", "life2", "zmind1",
	"zmind2", "zmind3", "zcoal_1", "zcoal_2", "zcoal_3", "md8_5", "md8_6", "md8_b1",
	"md8_b2", "4sbwy_22"/*sbwy4_22*/, "tunnel_4", "tunnel_5", "md8brdg2", "md8_32", "canon_1",
	"canon_2", "md_e1", "xmvtes", "las0_1", "las0_2", "las0_3", "las0_4", "las0_5",
	"las0_6", "las0_7", "las0_8", "las1_1", "las1_2", "las1_3", "las1_4", "las2_1",
	"las2_2", "las2_3", "las2_4", "las3_1", "las3_2", "las3_3", "las4_0", "las4_1",
	"las4_2", "las4_3", "las4_4", "lastmap", "fallp", "m_endo", "hill2", "bonevil2",
	"junone22", "rckt32", "jtemplc", "fship_26", "las4_42", "tunnel_6", "md8_52", "sininb34",
	"mds7st33", "midgal", "sininb35", "nivgate4", "sininb36", "ztruck", "frcyo2"
};

// Standard mapList
void Data::openMaplist(bool PC)
{
	field_names.clear();
	for (int i=0; i<788; ++i) {
		field_names.append(_mapList[i]);
	}

	if (PC) {
		toPCMaplist(field_names);
	}
}

void Data::toPCMaplist(QStringList &field_names)
{
	field_names[88] = "qa";
	field_names[89] = "qb";
	field_names[90] = "qc";
	field_names[91] = "qd";
	field_names[92] = "qe";
	field_names[153] = "min71";
	field_names[164] = "sbwy4_1";
	field_names[165] = "sbwy4_2";
	field_names[166] = "sbwy4_3";
	field_names[167] = "sbwy4_4";
	field_names[168] = "sbwy4_5";
	field_names[169] = "sbwy4_6";
	field_names[174] = "min51_1";
	field_names[175] = "min51_2";
	field_names[586] = "tower5";
	field_names[735] = "sbwy4_22";
}

const char *Data::musicList[100] =
{
	"none", "nothing", "oa", "ob", "dun2", "guitar2", "fanfare", "makoro", "bat",
	"fiddle", "kurai", "chu", "ketc", "earis", "ta", "tb", "sato",
	"parade", "comical", "yume", "mati", "sido", "siera", "walz", "corneo",
	"horror", "canyon", "red", "seto", "ayasi", "sinra", "sinraslo", "dokubo",
	"bokujo", "tm", "tifa", "costa", "rocket", "earislo", "chase", "rukei",
	"cephiros", "barret", "corel", "boo", "elec", "rhythm", "fan2", "hiku",
	"cannon", "date", "cintro", "cinco", "chu2", "yufi", "aseri", "gold1",
	"mura1", "yado", "over2", "crwin", "crlost", "odds", "geki", "junon",
	"tender", "wind", "vincent", "bee", "jukai", "sadbar", "aseri2", "kita",
	"sid2", "sadsid", "iseki", "hen", "utai", "snow", "yufi2", "mekyu",
	"condor", "lb2", "gun", "weapon", "pj", "sea", "ld", "lb1",
	"sensui", "ro", "jyro", "nointro", "riku", "si", "mogu", "pre",
	"fin", "heart", "roll"
};

const char *Data::musicList2[100] =
{
	"none", "nothing", "Opening - Bombing Mission", "Bombing Mission",
	"Chasing The Black-Caped Man", "On The Other Side Of The Mountain",
	"Fanfare (intro)", "Mako Reactor", "Fighting", "Fiddle De Chocobo",
	"Anxious Heart", "Still More Fighting", "Cait Sith's Theme", "Aerith's Theme",
	"Final Fantasy VII Main Theme", "Final Fantasy VII Main Theme B", "sato",
	"Rufus' Welcoming Ceremony", "comical", "Who Are You", "Ahead on Our Way",
	"It's Difficult to Stand on Both Feet, Isn't It", "If You Open Your Heart",
	"Waltz de Chocobo", "Don of the Slums", "Trail Of Blood", "Cosmo Canyon",
	"Red XIII's Theme", "Great Warrior", "Lurking in the Darkness",
	"Shinra Company", "Infiltrating Shinra Tower", "Underneath the Rotting Pizza",
	"Farm Boy", "On That Day, Five Years Ago", "Tifa's Theme", "Costa Del Sol",
	"Oppressed People", "Flowers Blooming in the Church", "Crazy Motorcycle",
	"Sandy Badlands", "Those Chosen by the Planet", "Barret's Theme", "Mining Town",
	"Life Stream", "Electric de Chocobo", "Turk's Theme", "Fanfare",
	"Highwind Takes To The Skies", "The Makou Cannon Is Fired ~ Shinra Explodes A",
	"Interrupted By Fireworks", "Those Chosen By The Planet (Intro)", "Cinco de Chocobo",
	"J-E-N-O-V-A", "Descendant Of Shinobi", "Hurry!", "Gold Saucer",
	"Parochial Town", "Good Night, Until Tomorrow", "Continue", "A Great Success",
	"Tango Of Tears", "Racing Chocobo ~ Place Your Bets", "Debut",
	"Off The Edge Of Despair", "Holding My Thoughts In My Heart", "wind",
	"The Nightmare's Beginning", "Honeybee Manor", "Forest Temple",
	"Mark of the Traitor", "Hurry Faster!", "The Great Northern Cave",
	"Cid's Theme", "Sending a Dream Into the Universe", "You Can Hear the Cry of the Planet",
	"Who Am I", "Utai", "Buried in the Snow", "Stolen Materia", "Reunion",
	"Fortress Of The Condor", "One-Winged Angel", "Shinra Army Wages A Full-Scale Attack",
	"Weapon Raid", "Jenova Absolute", "A Secret, Sleeping in the Deep Sea",
	"Judgment Day", "The Birth of God", "sensui", "The Countdown Begins",
	"Steal The Tiny Bronco!", "Those Chosen By The Planet (no intro)",
	"The Makou Cannon Is Fired ~ Shinra Explodes B",
	"The Makou Cannon Is Fired ~ Shinra Explodes C", "Highwind Takes to the Skies",
	"The Prelude", "World Crisis", "heart", "Staff Roll"
};

QColor Data::color(Color color)
{
	bool darkTheme = Config::value("dark_theme", false).toBool();

	switch (color) {
	case ColorEvidence:
		return darkTheme ? qRgb(67, 67, 211) : qRgb(196, 196, 255);
	case ColorRedForeground:
		return darkTheme ? qRgb(0xF0, 0x90, 0x90) : qRgb(0xb0, 0x18, 0x18);
	case ColorGreenForeground:
		return darkTheme ? qRgb(0x00, 0xc6, 0x00) : qRgb(0x00, 0x68, 0x00);
	case ColorBlueForeground:
		return darkTheme ? qRgb(0x5E, 0xAF, 0xFF) : qRgb(0x00, 0x57, 0xAF);
	case ColorPurpleForeground:
		return darkTheme ? qRgb(0xC9, 0x92, 0xFF) : qRgb(0x66, 0x00, 0xCC);
	case ColorOrangeForeground:
		return darkTheme ? qRgb(0xFF, 0x8D, 0x1A) : qRgb(0x8C, 0x46, 0x00);
	case ColorGreyForeground:
		return darkTheme ? qRgb(0xCC, 0xCC, 0xCC) : qRgb(0x59, 0x59, 0x59);
	case ColorDisabledForeground:
		return darkTheme ? qRgb(0xAA, 0xAA, 0xAA) : qRgb(0xCC, 0xCC, 0xCC);
	case ColorRedBackground:
		return darkTheme ? qRgb(0xB6, 0x00, 0x00) : qRgb(0xFF, 0xCC, 0xCC);
	}
	return QColor();
}
