#pragma once

#include <QtCore>
#include "FieldPart.h"
#include "BackgroundTiles.h"

// This class is only used for repairing PC backgrounds
class BackgroundTilesFile : public FieldPart
{
public:
	explicit BackgroundTilesFile(Field *field);
	virtual ~BackgroundTilesFile();

	bool open();
	bool open(const QByteArray &data);
	QByteArray save() const;
	void clear();

	inline const BackgroundTiles &tiles() const {
		return _tiles;
	}

private:
	BackgroundTiles _tiles;
};
