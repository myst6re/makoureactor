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
#ifndef DEF_COMMANDE
#define DEF_COMMANDE

#include <QtGui>
#include "FF7Text.h"
#include "Var.h"
#include "Data.h"
#include "parametres.h"

#define B1(v)		((v>>4)&0xF)
#define B2(v)		(v&0xF)

typedef struct {
	quint16 opcode;
	qint16 x, y;
	quint16 w, h;
	quint16 ask_first, ask_last;
	quint8 type;
	quint16 groupID, scriptID, commandeID;
} FF7Window;

class Commande
{	
public:
	explicit Commande(const QByteArray &commande, int pos=0);
	Commande();

	virtual quint8 id() const;
	virtual QString toString() const;
	virtual QByteArray params() const;

	quint8 size() const;
	const quint8 &getOpcode() const;
	QByteArray &getParams();
	const QByteArray &getConstParams() const;
	QByteArray toByteArray() const;
	quint16 getIndent() const;
	quint16 getPos() const;
	int subParam(int cur, int paramSize) const;

	void setCommande(const QByteArray &commande);
	void setPos(quint16 pos);

	bool isVoid() const;

	bool rechercherVar(quint8 bank, quint8 adress, int value=65536) const;
	QList<int> searchAllVars();
	bool rechercherExec(quint8 group, quint8 script) const;
	bool rechercherTexte(const QRegExp &texte) const;
	void listUsedTexts(QSet<quint8> &usedTexts) const;
	void listUsedTuts(QSet<quint8> &usedTuts) const;
	void shiftTextIds(int textId, int steps);
	void shiftTutIds(int tutId, int steps);
	void listWindows(QMultiMap<quint8, FF7Window> &windows, QMultiMap<quint8, quint8> &text2win) const;

	QString toString2() const;
	QString traduction() const;

	static QString _personnage(quint8 persoID);

protected:
	quint8 opcode;
	quint16 pos;
	QByteArray _params;

	static QString _script(quint8 param);
	static QString _text(quint8 textID);
	static QString _item(const QByteArray &param, quint8 bank);
	static QString _materia(const QByteArray &param, quint8 bank);
	static QString _field(const QByteArray &param);
	static QString _movie(quint8 movieID);
	// static QString _objet3D(quint8 objet3D_ID);

	static QString _bank(quint8 adress, quint8 bank);
	static QString _var(const QByteArray &param, quint8 bank);
	static QString _lvar(const QByteArray &param, quint8 bank1, quint8 bank2);
	static QString _lvar(const QByteArray &param, quint8 bank1, quint8 bank2, quint8 bank3);
	static QString _svar(const QByteArray &param, quint8 bank);
	static QString _var(int value, quint8 bank);
	static quint32 _toInt(const QByteArray &param);
	static qint16 _toSInt(const QByteArray &param);

	static QString _operateur(quint8 param);
	static QString _miniGame(quint8 ID, quint8 param);
	static QString _menu(quint8 ID, const QString &param);
	static QString _windowType(quint8 param);
	static QString _windowNum(quint8 param);
	static QString _windowCorner(quint8 param, quint8 bank);
	static QString _sensRotation(quint8 param);
	static QString _key(quint16 param);
	static QString _battleMode(quint32 param);
	QString _special() const;
	QString _kawai() const;
	
};

class OpcodeEmpty : public Commande
{
public:
	explicit OpcodeEmpty(quint8 id);

	virtual quint8 id() const;
	virtual QString toString() const;
private:
	quint8 _id;
};

class OpcodeRET : public Commande
{
public:
	explicit OpcodeRET();

	virtual quint8 id() const;
	virtual QString toString() const;
};

class OpcodeExec : public Commande
{
public:
	explicit OpcodeExec(const QByteArray &params);

	virtual quint8 id() const=0;
	virtual QString toString() const=0;
	QByteArray params() const;
protected:
	quint8 groupID;
	quint8 scriptID;
	quint8 priority;
};

class OpcodeREQ : public OpcodeExec
{
public:
	explicit OpcodeREQ(const QByteArray &params);

	virtual quint8 id() const;
	virtual QString toString() const;
};

class OpcodeREQSW : public OpcodeExec
{
public:
	explicit OpcodeREQSW(const QByteArray &params);

	virtual quint8 id() const;
	virtual QString toString() const;
};

class OpcodeREQEW : public OpcodeExec
{
public:
	explicit OpcodeREQEW(const QByteArray &params);

	virtual quint8 id() const;
	virtual QString toString() const;
};

class OpcodeExecChar : public Commande
{
public:
	explicit OpcodeExecChar(const QByteArray &params);

	virtual quint8 id() const=0;
	virtual QString toString() const=0;
	QByteArray params() const;
protected:
	quint8 charID;
	quint8 scriptID;
	quint8 priority;
};

class OpcodePREQ : public OpcodeExecChar
{
public:
	explicit OpcodePREQ(const QByteArray &params);

	virtual quint8 id() const;
	virtual QString toString() const;
};

class OpcodePRQSW : public OpcodeExecChar
{
public:
	explicit OpcodePRQSW(const QByteArray &params);

	virtual quint8 id() const;
	virtual QString toString() const;
};

class OpcodePRQEW : public OpcodeExecChar
{
public:
	explicit OpcodePRQEW(const QByteArray &params);

	virtual quint8 id() const;
	virtual QString toString() const;
};

class OpcodeRETTO : public Commande
{
public:
	explicit OpcodeRETTO(const QByteArray &params);

	virtual quint8 id() const;
	virtual QString toString() const;
	virtual QByteArray params() const;
private:
	quint8 scriptID;
	quint8 priority;
};

class OpcodeJOIN : public Commande
{
public:
	explicit OpcodeJOIN(const QByteArray &params);

	virtual quint8 id() const;
	virtual QString toString() const;
	virtual QByteArray params() const;
private:
	quint8 speed;
};

class OpcodeSPLIT : public Commande
{
public:
	explicit OpcodeSPLIT(const QByteArray &params);

	virtual quint8 id() const;
	virtual QString toString() const;
	virtual QByteArray params() const;
private:
	quint8 banks[3];
	qint16 targetX1, targetY1, targetX2, targetY2;
	quint8 direction1, direction2;
	quint8 speed;
};

class OpcodePartyE : public Commande
{
public:
	explicit OpcodePartyE(const QByteArray &params);

	virtual quint8 id() const=0;
	virtual QString toString() const=0;
	QByteArray params() const;
protected:
	quint8 banks[2];
	quint8 party1, party2, party3;
};

class OpcodeSPTYE : public OpcodePartyE
{
public:
	explicit OpcodeSPTYE(const QByteArray &params);

	virtual quint8 id() const;
	virtual QString toString() const;
};

class OpcodeGTPYE : public OpcodePartyE
{
public:
	explicit OpcodeGTPYE(const QByteArray &params);

	virtual quint8 id() const;
	virtual QString toString() const;
};

class OpcodeDSKCG : public Commande
{
public:
	explicit OpcodeDSKCG(const QByteArray &params);

	virtual quint8 id() const;
	virtual QString toString() const;
	virtual QByteArray params() const;
private:
	quint8 diskID;
};

class OpcodeSPECIALARROW : public Commande
{
public:
	explicit OpcodeSPECIALARROW(const QByteArray &params);

	virtual quint8 id() const;
	virtual QString toString() const;
	virtual QByteArray params() const;
private:
	quint8 hide;
};

class OpcodeSPECIALPNAME : public Commande
{
public:
	explicit OpcodeSPECIALPNAME(const QByteArray &params);

	virtual quint8 id() const;
	virtual QString toString() const;
	virtual QByteArray params() const;
private:
	quint8 unknown;
};

class OpcodeSPECIALGMSPD : public Commande
{
public:
	explicit OpcodeSPECIALGMSPD(const QByteArray &params);

	virtual quint8 id() const;
	virtual QString toString() const;
	virtual QByteArray params() const;
private:
	quint8 speed;
};

class OpcodeSPECIALSMSPD : public Commande
{
public:
	explicit OpcodeSPECIALSMSPD(const QByteArray &params);

	virtual quint8 id() const;
	virtual QString toString() const;
	virtual QByteArray params() const;
private:
	quint8 unknown, speed;
};

class OpcodeSPECIALFLMAT : public Commande
{
public:
	explicit OpcodeSPECIALFLMAT();

	virtual quint8 id() const;
	virtual QString toString() const;
};

class OpcodeSPECIALFLITM : public Commande
{
public:
	explicit OpcodeSPECIALFLITM();

	virtual quint8 id() const;
	virtual QString toString() const;
};

class OpcodeSPECIALBTLCK : public Commande
{
public:
	explicit OpcodeSPECIALBTLCK(const QByteArray &params);

	virtual quint8 id() const;
	virtual QString toString() const;
	virtual QByteArray params() const;
private:
	quint8 lock;
};

class OpcodeSPECIALMVLCK : public Commande
{
public:
	explicit OpcodeSPECIALMVLCK(const QByteArray &params);

	virtual quint8 id() const;
	virtual QString toString() const;
	virtual QByteArray params() const;
private:
	quint8 lock;
};

class OpcodeSPECIALSPCNM : public Commande
{
public:
	explicit OpcodeSPECIALSPCNM(const QByteArray &params);

	virtual quint8 id() const;
	virtual QString toString() const;
	virtual QByteArray params() const;
private:
	quint8 charID;
	quint8 textID;
};

class OpcodeSPECIALRSGLB : public Commande
{
public:
	explicit OpcodeSPECIALRSGLB();

	virtual quint8 id() const;
	virtual QString toString() const;
};

class OpcodeSPECIALCLITM : public Commande
{
public:
	explicit OpcodeSPECIALCLITM();

	virtual quint8 id() const;
	virtual QString toString() const;
};

class OpcodeSPECIAL : public Commande
{
public:
	explicit OpcodeSPECIAL(const QByteArray &params);

	virtual quint8 id() const;
	virtual QString toString() const;
	virtual QByteArray params() const;
private:
	Commande *opcode;
};



#endif
