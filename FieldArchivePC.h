#ifndef FIELDARCHIVEPC_H
#define FIELDARCHIVEPC_H

#include "FieldArchive.h"

class FieldArchivePC : public FieldArchive
{
public:
	FieldArchivePC();
	explicit FieldArchivePC(const QString &path, bool isDirectory=false);
	virtual ~FieldArchivePC();
	inline bool isPC() const { return true; }

	void clear();

	TutFile *tut(const QString &name);
	const QMap<QString, TutFile *> &tuts() const;
	void addTut(const QString &name);

	void setSaved();

	FieldArchiveIO *io() const;
private:
	QMap<QString, TutFile *> _tuts;
};

#endif // FIELDARCHIVEPC_H
