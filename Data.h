#ifndef DATA_H
#define DATA_H

#include <QtGui>
#include "FF7Text.h"
#include "Opcode.h"
#include "Config.h"
#include "LZS.h"
#ifdef Q_WS_WIN
#include <windef.h>
#include <winbase.h>
#include <winerror.h>
#include <winreg.h>
#endif

class Data
{
public:
	static const QString &ff7DataPath();
	static const QString &ff7AppPath();
	static bool isRereleasedPath();
	static QString charlgp_path();
	static void charlgp_loadListPos(QFile *);
	static void charlgp_loadAnimBoneCount();
	static QHash<QString, int> charlgp_listPos;
	static QHash<QString, int> charlgp_animBoneCount;
	static int load();
	static void openMaplist(const QByteArray &data);
	static void openMaplist(bool PC=false);
	static QStringList char_names;
	static QStringList operateur_names;
	static QStringList key_names;
	static QStringList item_names;
	static QStringList weapon_names;
	static QStringList armor_names;
	static QStringList accessory_names;
	static QStringList materia_names;
	static const QList<FF7Text *> *currentTextes;
	static QStringList currentGrpScriptNames;
//	static QStringList currentCharNames;
	static int currentModelID;
	static QStringList *currentHrcNames;
	static QList<QStringList> *currentAnimNames;
	static QStringList field_names;
	static QStringList movie_names;

private:
	static const QString &searchRereleasedFF7Path();
	static void fill(const QByteArray &data, QStringList &names);
	static QString ff7DataPath_cache;
	static QString ff7AppPath_cache;
	static QString ff7RereleasePath_cache;
	static bool ff7RereleaseAlreadySearched;

};

#endif // DATA_H
