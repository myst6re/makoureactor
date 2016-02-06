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
#include "Config.h"

bool Var::load()
{
	//Chargement du fichier dans Var::var_names
	QFile fic(Config::programResourceDir()+"/vars.cfg");
	if(!fic.exists() && fic.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		fic.write(
			QString("1|3|$AerisLovePoints\n").toLocal8Bit()+
			QString("1|4|$TifaLovePoints\n").toLocal8Bit()+
			QString("1|5|$YuffieLovePoints\n").toLocal8Bit()+
			QString("1|6|$BarretLovePoints\n").toLocal8Bit()+
			QString("1|20|$Hours\n").toLocal8Bit()+
			QString("1|21|$Minutes\n").toLocal8Bit()+
			QString("1|22|$Seconds\n").toLocal8Bit()+
			QString("1|23|$Frames\n").toLocal8Bit()+
			QString("1|64|$KeyItems\n").toLocal8Bit()+
			QString("1|65|$KeyItems\n").toLocal8Bit()+
			QString("1|66|$KeyItems\n").toLocal8Bit()+
			QString("1|67|$KeyItems\n").toLocal8Bit()+
			QString("1|68|$KeyItems\n").toLocal8Bit()+
			QString("1|69|$KeyItems\n").toLocal8Bit()+
			QString("1|70|$KeyItems\n").toLocal8Bit()+
			QString("1|85|$ChocoboType1Catched\n").toLocal8Bit()+
			QString("1|86|$ChocoboType2Catched\n").toLocal8Bit()+
			QString("1|87|$ChocoboType3Catched\n").toLocal8Bit()+
			QString("1|88|$ChocoboType4Catched\n").toLocal8Bit()+
			QString("1|124|$ChocoboOnMap\n").toLocal8Bit()+
			QString("1|167|$VictoryFortCondor\n").toLocal8Bit()+
			QString("1|182|$DefeatFortCondor\n").toLocal8Bit()+
			QString("2|0|$GameMoment\n").toLocal8Bit()+
			QString("2|24|$BattleCount\n").toLocal8Bit()+
			QString("2|26|$BattleEscaped\n").toLocal8Bit()+
			QString("2|180|$GoldFortCondor\n").toLocal8Bit()+
			QString("3|9|$PartyLeader\n").toLocal8Bit()+
			QString("3|88|$NumStablesBought\n").toLocal8Bit()+
			QString("3|89|$NumStablesFilled\n").toLocal8Bit()+
			QString("13|31|$ErrorAddMateria\n").toLocal8Bit()+
			QString("13|84|$LastCloud\n").toLocal8Bit()+
			QString("13|85|$LastBarret\n").toLocal8Bit()+
			QString("13|86|$LastTifa\n").toLocal8Bit()+
			QString("13|87|$LastRedXIII\n").toLocal8Bit()+
			QString("13|88|$LastCid\n").toLocal8Bit()+
			QString("13|89|$LastYuffi\n").toLocal8Bit()+
			QString("13|90|$LastCaitSith\n").toLocal8Bit()+
			QString("13|92|$LastVincent\n").toLocal8Bit()
		);
		fic.close();
	}

	if(fic.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QString line;
		QStringList liste;
		bool ok;
		int bank, adress;
		forever
		{
			line = QString(fic.readLine());
			if(line.isEmpty())			break;
			
			liste = line.split(QChar('|'));
			if(liste.size() != 3)		return false;
			
			bank = liste.at(0).toInt(&ok);
			if(!ok)						return false;
			if(bank<1 || bank>15)		continue;
			
			adress = liste.at(1).toInt(&ok);
			if(!ok)						return false;
			if(adress<0 || adress>255)	continue;
			
			line = liste.at(2);
			if(line.isEmpty())			return false;
			if(line.size()>50)			line = line.left(50);
			
			set(bank, adress, line.simplified());
		}
		fic.close();

		return true;
	}
	return false;
}

bool Var::save(const QMap<quint16, QString> &new_var_names)
{
	//Enregistrement de var_names dans le fichier
	QFile fic(qApp->applicationDirPath() + "/vars.cfg");
	if(fic.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
	{
		var_names = new_var_names;
		QMapIterator<quint16, QString> i(var_names);
		while(i.hasNext())
		{
			i.next();
			QString str = QString("%1|").arg(i.key() >> 8);
			str += QString("%1|").arg(i.key() & 255);
			str += i.value() + "\n";
			fic.write(str.toLocal8Bit());
		}
		fic.close();

		return true;
	}
	return false;
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
