#ifndef BACKGROUNDTEXTURES_H
#define BACKGROUNDTEXTURES_H

#include <QtCore>
#include "../PsColor.h"
#include "BackgroundTiles.h"

struct BackgroundTexturesPCInfos
{
	quint32 pos;
	quint8 depth, size;
};

//Sizeof : 12
typedef struct {
	quint32 size;// = 12 + w*2*h
	quint16 x, y;
	quint16 w, h;
} MIM;

class BackgroundTextures
{
public:
	BackgroundTextures();
	virtual ~BackgroundTextures();
	inline const QByteArray &data() const {
		return _data;
	}
	inline void setData(const QByteArray &data) const {
		_data = data;
	}
	virtual void clear() {
		_data.clear();
	}
	QVector<uint> tile(const Tile &tile) const;
	virtual inline quint8 depth(const Tile &tile) const {
		return tile.depth;
	}
protected:
	virtual quint16 textureWidth(const Tile &tile) const=0;
	virtual quint32 originInData(const Tile &tile) const=0;
	virtual QRgb directColor(quint16 color) const=0;
	QRgb pixel(quint32 pos) const;
private:
	static QByteArray _data;
};

class BackgroundTexturesPC;
class BackgroundTexturesPS;

class BackgroundTexturesPC : public BackgroundTextures
{
public:
	BackgroundTexturesPC();
	explicit BackgroundTexturesPC(const QHash<quint8, BackgroundTexturesPCInfos> &texInfos);
	bool hasTex(quint8 texID) const;
	quint32 texPos(quint8 texID) const;
	quint8 texDepth(quint8 texID) const;
	quint8 texTileSize(quint8 texID) const;
	BackgroundTexturesPCInfos texInfos(quint8 texID) const;
	void addTexInfos(quint8 texID, const BackgroundTexturesPCInfos &infos);
	void clear();
	void setTexInfos(const QHash<quint8, BackgroundTexturesPCInfos> &texInfos);
	QList<uint> tex(quint8 texID) const;
	BackgroundTexturesPS toPS() const;
protected:
	quint16 textureWidth(const Tile &tile) const;
	quint8 depth(const Tile &tile) const;
	quint32 originInData(const Tile &tile) const;
	QRgb directColor(quint16 color) const;
private:
	QHash<quint8, BackgroundTexturesPCInfos> _texInfos;
};

class BackgroundTexturesPS : public BackgroundTextures
{
public:
	BackgroundTexturesPS();
	quint32 pageDataPos(quint8 pageID) const;
	quint16 pageTexPos(quint8 pageID) const;
	quint16 pageTexWidth(quint8 pageID) const;
	void setDataPos(quint32 dataPos);
	void setHeaderImg(const MIM &headerImg);
	void setHeaderEffect(const MIM &headerEffect);
	BackgroundTexturesPC toPC(const BackgroundTiles &tiles) const;
protected:
	quint16 textureWidth(const Tile &tile) const;
	quint32 originInData(const Tile &tile) const;
	QRgb directColor(quint16 color) const;
private:
	quint32 _dataPos;
	MIM _headerImg, _headerEffect;
};

#endif // BACKGROUNDTEXTURES_H
