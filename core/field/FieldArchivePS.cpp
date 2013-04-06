#include "FieldArchivePS.h"

FieldArchivePS::FieldArchivePS() :
	FieldArchive()
{
}

FieldArchivePS::FieldArchivePS(const QString &path, bool isDirectory) :
	FieldArchive(path, isDirectory)
{
}

FieldArchiveIO *FieldArchivePS::io() const
{
	return FieldArchive::io();
}
