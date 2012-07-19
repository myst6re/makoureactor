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
#include "Var.h"

int Var::load()
{
	//Chargement du fichier dans Var::var_names
	QFile fic(qApp->applicationDirPath()+"/vars.cfg");
	if(!fic.exists() && fic.open(QIODevice::WriteOnly))
	{
		fic.write(
			QString("1|3|$AerisLovePoints\r\n").toLocal8Bit()+
			QString("1|4|$TifaLovePoints\r\n").toLocal8Bit()+
			QString("1|5|$YuffieLovePoints\r\n").toLocal8Bit()+
			QString("1|6|$BarretLovePoints\r\n").toLocal8Bit()+
			QString("1|64|$KeyItems\r\n").toLocal8Bit()+
			QString("1|65|$KeyItems\r\n").toLocal8Bit()+
			QString("1|66|$KeyItems\r\n").toLocal8Bit()+
			QString("1|67|$KeyItems\r\n").toLocal8Bit()+
			QString("1|68|$KeyItems\r\n").toLocal8Bit()+
			QString("1|69|$KeyItems\r\n").toLocal8Bit()+
			QString("1|70|$KeyItems\r\n").toLocal8Bit()+
			QString("1|85|$ChocoboType1Catched\r\n").toLocal8Bit()+
			QString("1|86|$ChocoboType2Catched\r\n").toLocal8Bit()+
			QString("1|87|$ChocoboType3Catched\r\n").toLocal8Bit()+
			QString("1|88|$ChocoboType4Catched\r\n").toLocal8Bit()+
			QString("1|124|$ChocoboOnMap\r\n").toLocal8Bit()+
			QString("1|167|$VictoryFortCondor\r\n").toLocal8Bit()+
			QString("1|182|$DefeatFortCondor\r\n").toLocal8Bit()+
			QString("2|0|$GameMoment\r\n").toLocal8Bit()+
			QString("2|24|$BattleCount\r\n").toLocal8Bit()+
			QString("2|26|$BattleEscaped\r\n").toLocal8Bit()+
			QString("2|180|$GoldFortCondor\r\n").toLocal8Bit()+
			QString("3|9|$PartyLeader\r\n").toLocal8Bit()+
			QString("3|88|$NumStablesBought\r\n").toLocal8Bit()+
			QString("3|89|$NumStablesFilled\r\n").toLocal8Bit()+
			QString("13|31|$ErrorAddMateria\r\n").toLocal8Bit()+
			QString("13|84|$LastCloud\r\n").toLocal8Bit()+
			QString("13|85|$LastBarret\r\n").toLocal8Bit()+
			QString("13|86|$LastTifa\r\n").toLocal8Bit()+
			QString("13|87|$LastRedXIII\r\n").toLocal8Bit()+
			QString("13|88|$LastCid\r\n").toLocal8Bit()+
			QString("13|89|$LastYuffi\r\n").toLocal8Bit()+
			QString("13|90|$LastCaitSith\r\n").toLocal8Bit()+
			QString("13|92|$LastVincent\r\n").toLocal8Bit()
		);
		fic.close();
	}

	if(fic.open(QIODevice::ReadOnly))
	{
		QString line;
		QStringList liste;
		bool ok;
		int bank, adress;
		while(true)
		{
			line = QString(fic.readLine());
			if(line.isEmpty())	return 0;
			
			liste = line.split(QChar('|'));
			if(liste.size()!=3)	return 1;
			
			bank = liste.at(0).toInt(&ok);
			if(!ok)				return 1;
			if(bank<1 || bank>15)	continue;
			
			adress = liste.at(1).toInt(&ok);
			if(!ok)				return 1;
			if(adress<0 || adress>255)	continue;
			
			line = liste.at(2);
			if(line.isEmpty())	return 1;
			if(line.size()>50)	line = line.left(50);	
			
			Var::set(bank, adress, line.simplified());
		}
		fic.close();
		return 0;
	}
	return 1;
}

int Var::save(const QMap<quint16, QString> &new_var_names)
{
	//Enregistrement de Var::var_names dans le fichier
	QFile fic(qApp->applicationDirPath()+"/vars.cfg");
	if(fic.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		Var::var_names = new_var_names;
		fic.setTextModeEnabled(true);
		QMapIterator<quint16, QString> i(Var::var_names);
		while(i.hasNext())
		{
			i.next();
			QString chaine = QString("%1|").arg(i.key() >> 8);
			chaine += QString("%1|").arg(i.key() & 255);
			chaine += i.value() + "\n";
			fic.write(QByteArray(chaine.toLocal8Bit()));
		}
		
		fic.close();
		return 0;
	}
	return 1;
}

QMap<quint16, QString> Var::var_names;

QString Var::name(quint8 bank, quint8 adress)
{
	return var_names.value(adress | (bank << 8), QString());//Retourne le nom de la variable s'il est listé, retourne "" sinon
}

const QMap<quint16, QString> &Var::get()
{
	return var_names;
}

void Var::set(quint8 bank, quint8 adress, const QString &name)
{
	var_names.insert(adress | (bank << 8), name);
}

void Var::del(quint8 bank, quint8 adress)
{
	var_names.remove(adress | (bank << 8));
}

bool Var::exists(quint8 bank, quint8 adress)
{
	return var_names.contains(adress | (bank << 8));
}
