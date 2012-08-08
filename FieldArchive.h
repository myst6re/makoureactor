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
#ifndef DEF_FIELDARCHIVE
#define DEF_FIELDARCHIVE

#include <QtGui>
#include "Field.h"
#include "IsoArchive.h"
#include "GZIP.h"

class FieldArchive : public QObject, IsoControl
{
	Q_OBJECT

public:
	enum Sorting{
		SortByName, SortByMapId
	};

	FieldArchive();
	FieldArchive(const QString &path, bool isDirectory=false);
	virtual ~FieldArchive();

	int size() const;
	Field *field(quint32 id, bool open=true);
	Field *field(const QString &name, bool open=true);
	QByteArray getLgpData(int position);
	QByteArray getFieldData(Field *field, bool unlzs=true);
	QByteArray getMimData(Field *field, bool unlzs=true);
	QByteArray getModelData(Field *field, bool unlzs=true);
	TutFile *getTut(const QString &name);

	bool isAllOpened();
	QList<FF7Var> searchAllVars();
	void searchAll();// research & debug function
	bool rechercherOpCode(quint8 opCode, int &fieldID, int &groupID, int &scriptID, int &commandeID, Sorting sorting);
	bool rechercherVar(quint8 bank, quint8 adress, int value, int &fieldID, int &groupID, int &scriptID, int &commandeID, Sorting sorting);
	bool rechercherExec(quint8 group, quint8 script, int &fieldID, int &groupID, int &scriptID, int &commandeID, Sorting sorting);
	bool rechercherTexte(const QRegExp &texte, int &fieldID, int &groupID, int &scriptID, int &commandeID, Sorting sorting);
	bool rechercherOpCodeP(quint8 opCode, int &fieldID, int &groupID, int &scriptID, int &commandeID, Sorting sorting);
	bool rechercherVarP(quint8 bank, quint8 adress, int value, int &fieldID, int &groupID, int &scriptID, int &commandeID, Sorting sorting);
	bool rechercherExecP(quint8 group, quint8 script, int &fieldID, int &groupID, int &scriptID, int &commandeID, Sorting sorting);
	bool rechercherTexteP(const QRegExp &texte, int &fieldID, int &groupID, int &scriptID, int &commandeID, Sorting sorting);

	void close();
	quint8 open(QList<QTreeWidgetItem *> &items);
	quint8 save(QString path=QString());

	QString path() const;
	QString chemin() const;
	QString name() const;

	bool isDatFile() const;
	bool isDirectory() const;
	bool isLgp() const;
	bool isIso() const;

	void setIsoOut(int val) {
		QApplication::processEvents();
		emit progress(val);
	}
signals:
	void progress(int);
	void nbFilesChanged(int);
//	void fileChanged(const QString &path);
//	void directoryChanged(const QString &path);
	
private:
	void addDAT(const QString &name, QList<QTreeWidgetItem *> &items);
	qint32 rechercherFichier(const QString &name) const;
	qint8 openField(Field *field);
	void setSaved();
	QByteArray updateFieldBin(const QByteArray &data, IsoDirectory *fieldDirectory);
	
	QList<Field *> fileList;
	QMultiMap<QString, int> fieldsSortByName;
	QMultiMap<QString, int> fieldsSortByMapId;
	QFile *fic;
	QDir *dir;
	IsoArchive *iso;
	IsoDirectory *isoFieldDirectory;
	QMap<QString, int> tutPos;
	QMap<QString, TutFile *> tuts;
	bool isDat;
	// QFileSystemWatcher fileWatcher;
};

#endif
