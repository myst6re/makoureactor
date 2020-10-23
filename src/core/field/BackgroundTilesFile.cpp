#include "BackgroundTilesFile.h"
#include "BackgroundTilesIO.h"
#include "Field.h"

BackgroundTilesFile::BackgroundTilesFile(Field *field) :
      FieldPart(field)
{
}

BackgroundTilesFile::~BackgroundTilesFile()
{
}

bool BackgroundTilesFile::open()
{
	return open(field()->sectionData(Field::Tiles));
}

bool BackgroundTilesFile::open(const QByteArray &data)
{
	QBuffer buffer;
	buffer.setData(data);

	if (BackgroundTilesIOPS(&buffer).read(_tiles)) {
		setOpen(true);

		return true;
	}

	return false;
}

QByteArray BackgroundTilesFile::save() const
{
	QBuffer buffer;

	BackgroundTilesIOPS(&buffer).write(_tiles);

	return buffer.data();
}

void BackgroundTilesFile::clear()
{
	_tiles.clear();
}
