#include "AkaoFile.h"

QString ByteArrayDate::toString() const
{
	return QString("%1/%2/%3 %4:%5:%6")
	    .arg(QChar(at(0)))
	    .arg(QChar(at(1)))
	    .arg(QChar(at(2)))
	    .arg(QChar(at(3)))
	    .arg(QChar(at(4)))
	    .arg(QChar(at(5)));
}

AkaoFile::AkaoFile()
{
}
