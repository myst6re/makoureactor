#ifndef BACKGROUNDTEXTURES_H
#define BACKGROUNDTEXTURES_H

#include <QtCore>

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
	virtual inline void clear() { }
};

class BackgroundTexturesPC : public BackgroundTextures
{
public:
	BackgroundTexturesPC();
	explicit BackgroundTexturesPC(const QHash<quint8, BackgroundTexturesPCInfos> &texInfos);
	bool hasTex(quint8 texID) const;
	quint32 texPos(quint8 texID) const;
	quint8 texDepth(quint8 texID) const;
	void addTexInfos(quint8 texID, const BackgroundTexturesPCInfos &infos);
	void clear();
	void setTexInfos(const QHash<quint8, BackgroundTexturesPCInfos> &texInfos);
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
private:
	quint32 _dataPos;
	MIM _headerImg, _headerEffect;
};

#endif // BACKGROUNDTEXTURES_H
