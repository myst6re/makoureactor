#ifndef ISOARCHIVEFF7_H
#define ISOARCHIVEFF7_H

#include "IsoArchive.h"

class IsoArchiveFF7 : public IsoArchive
{
public:
	enum Compression {
		None, LZS, GZIP
	};

	enum Country {
		NoCountry, Jp, Us, Uk, Fr, De, Es
	};

	IsoArchiveFF7(const QString &name);
	virtual ~IsoArchiveFF7();

	const QByteArray &fileLzs(const QString &path, quint32 maxSize=0);
	Country country() const;
	IsoFile *exe() const;
	QByteArray windowBinData() const;
	IsoDirectory *fieldDirectory() const;
	IsoDirectory *initDirectory() const;
	bool isDemo() const;
private:
	Q_DISABLE_COPY(IsoArchiveFF7)
	bool updateFieldBin();
	bool updateYamadaBin();
	bool reorganizeModifiedFilesAfter(QMap<quint32, const IsoFile *> &writeToTheMain, QList<const IsoFile *> &writeToTheEnd);
	QList<QIODevice *> _devicesToDelete;
};

#endif // ISOARCHIVEFF7_H
