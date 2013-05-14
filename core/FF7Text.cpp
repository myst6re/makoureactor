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
#include "FF7Text.h"
#include "Config.h"

FF7Text::FF7Text(const QByteArray &data)
{
	int index;
	_data = (index = data.indexOf('\xFF')) != -1 ? data.left(index) : data;
}

FF7Text::FF7Text(const QString &text, bool jp)
{
	setText(text, jp);
}

const QByteArray &FF7Text::data() const
{
	return _data;
}

QString FF7Text::text(bool jp, bool simplified) const
{
	QString trad, character;
	quint8 index;
//	bool jp = Config::value("jp_txt", false).toBool();
	int size = _data.size();

	for(quint16 i=0 ; i<size ; ++i)
	{
		index = (quint8)_data.at(i);
		if(index == 0xFF)	break;
		switch(index) {
		case 0xFA:
			++i;
			if(size<=i)	return trad.append(simplified ? "¶" : "{xfa}");
			if(jp) {
				character = getCaract(_data.at(i), 3);
				if(character.isEmpty()) {
					character = simplified ? "¶" : QString("{xfa}{x%1}").arg((quint8)_data.at(i), 2, 16, QChar('0'));
				}
				trad.append(character);
			} else {
				trad.append(simplified ? "¶" : QString("{xfa}{x%1}").arg((quint8)_data.at(i), 2, 16, QChar('0')));
			}
		break;
		case 0xFB:
			++i;
			if(size<=i)	return trad.append(simplified ? "¶" : "{xfb}");
			if(jp) {
				character = getCaract(_data.at(i), 4);
				if(character.isEmpty()) {
					character = simplified ? "¶" : QString("{xfb}{x%1}").arg((quint8)_data.at(i), 2, 16, QChar('0'));
				}
				trad.append(character);
			} else {
				trad.append(simplified ? "¶" : QString("{xfb}{x%1}").arg((quint8)_data.at(i), 2, 16, QChar('0')));
			}
		break;
		case 0xFC:
			++i;
			if(size<=i)	return trad.append(simplified ? "¶" : "{xfc}");
			if(jp) {
				character = getCaract(_data.at(i), 5);
				if(character.isEmpty()) {
					character = simplified ? "¶" : QString("{xfc}{x%1}").arg((quint8)_data.at(i), 2, 16, QChar('0'));
				}
				trad.append(character);
			} else {
				trad.append(simplified ? "¶" : QString("{xfc}{x%1}").arg((quint8)_data.at(i), 2, 16, QChar('0')));
			}
		break;
		case 0xFD:
			++i;
			if(size<=i)	return trad.append(simplified ? "¶" : "{xfd}");
			if(jp) {
				character = getCaract(_data.at(i), 6);
				if(character.isEmpty()) {
					character = simplified ? "¶" : QString("{xfd}{x%1}").arg((quint8)_data.at(i), 2, 16, QChar('0'));
				}
				trad.append(character);
			} else {
				trad.append(simplified ? "¶" : QString("{xfd}{x%1}").arg((quint8)_data.at(i), 2, 16, QChar('0')));
			}
		break;
		case 0xFE:
			++i;
			if(size<=i) 	return trad.append(simplified ? "¶" : "{xfe}");;
			index = (quint8)_data.at(i);

			if(index == 0xE2) {
				if(!simplified) {
					if(i+4 < size && (quint8)_data.at(i+4)==0 && (quint8)_data.at(i+2) <= 4) {
						quint8 bank;
						switch((quint8)_data.at(i+2)) {
						case 0:		bank = 1;	break;// 1 & 2
						case 1:		bank = 3;	break;// 3 & 4
						case 2:		bank = 11;	break;// 11 & 12
						case 3:		bank = 13;	break;// 13 & 14
						case 4:		bank = 15;	break;// 7 & 15
						}

						trad.append(QString("{MEMORY:var[%2][%1];size=%3}").arg((quint8)_data.at(i+1)).arg(bank).arg((quint8)_data.at(i+3)));
						i+=4;
					} else {
						trad.append("{xfe}{xe2}");
						++i;
						for(int i2=i+4 ; i<i2 ; ++i) {
							if(size<=i) 	return trad;
							trad.append(QString("{x%1}").arg((quint8)_data.at(i), 2, 16, QChar('0')));
						}
						--i;
					}
				} else {
					i+=4;
				}
			} else if(index == 0xDD) {
				++i;
				if(!simplified) {
					if(size<=i)	return trad.append("{xfe}{xdd}");
					trad.append(QString("{PAUSE%1}").arg((quint8)_data.at(i), 3, 10, QChar('0')));
				}
			} else {
				if(jp) {
					character = getCaract(index, 7);
					if(index >= 0xd2 && simplified) {
						character = "¶";
					} else if(character.isEmpty()) {
						character = simplified ? "¶" : QString("{xfe}{x%1}").arg(index, 2, 16, QChar('0'));
					}
					trad.append(character);
				} else {
					character = QString();
					if(index >= 0xd2 && !simplified) {
						character = getCaract(index, 7);
					}
					if(character.isEmpty()) {
						character = simplified ? "¶" : QString("{xfe}{x%1}").arg(index, 2, 16, QChar('0'));
					}
					trad.append(character);
				}
			}
		break;
		default:
			character = getCaract(index, jp ? 2 : 0);
			if((index == 0xe0 || index == 0xe1 || index == 0xe7 || index == 0xe8) && simplified) {
				character = " ";
			}
			else if(character.isEmpty()) {
				character = simplified ? "¶" : QString("{x%1}").arg(index, 2, 16, QChar('0'));
			}
			trad.append(character);
		break;
		}
	}
	return trad;
}

void FF7Text::setText(const QString &string, bool jp)
{
	QChar comp;
	int stringSize = string.size(), i, table;
	bool ok;
	ushort value;
//	bool jp = Config::value("jp_txt", false).toBool();

	_data.clear();

	for(int c=0 ; c<stringSize ; ++c)
	{
		comp = string.at(c);
		if(comp=='\n') {//\n{New Page}\n,\n
			if(string.mid(c+1, 11).compare("{New Page}\n", Qt::CaseInsensitive) == 0) {
				_data.append('\xe8');
				c += 11;
			}
			else if(string.mid(c+1, 13).compare("{New Page 2}\n", Qt::CaseInsensitive) == 0) {
				_data.append('\xe9');
				c += 13;
			}
			else {
				_data.append('\xe7');
			}
			continue;
		}
		else if(comp=='{') {
			QString rest = string.mid(c);
			for(i=0 ; i<16 ; ++i) {
				if(rest.startsWith(getCaract(0xea + i), Qt::CaseInsensitive)) {//{Name},{Key}
					_data.append((char)(0xea + i));
					c += getCaract(0xea + i).size()-1;
					goto end;
				}
			}

			for(i=0 ; i<11 ; ++i) {
				if(rest.startsWith(getCaract(0xd2 + i, 7), Qt::CaseInsensitive)) {//{Colors},{PAUSE}
					_data.append('\xfe').append((char)(0xd2 + i));
					c += getCaract(0xd2 + i, 7).size()-1;
					goto end;
				}
			}

			for(i=0 ; i<4 ; ++i) {
				if(rest.startsWith(getCaract(0xde + i, 7), Qt::CaseInsensitive)) {//{Vars}
					_data.append('\xfe').append((char)(0xde + i));
					c += getCaract(0xde + i, 7).size()-1;
					goto end;
				}
			}

			if(rest.startsWith(QString("{MEMORY:var["))) {
				QRegExp rx("^\\{MEMORY:var\\[(\\d+)\\]\\[(\\d+)\\];size=(\\d+)\\}");
				if(rx.indexIn(rest) != -1) {
					QStringList list = rx.capturedTexts();
					quint8 bank;
					switch(list.at(1).toInt()) {
					case 1:case 2:
						bank = 0;
						break;
					case 3:case 4:
						bank = 1;
						break;
					case 11:case 12:
						bank = 2;
						break;
					case 13:case 14:
						bank = 3;
						break;
					case 7:case 15:
						bank = 4;
						break;
					default:
						bank = 0;
						break;
					}
					_data.append("\xfe\xe2", 2).append((char)list.at(2).toInt()).append(bank).append((char)list.at(3).toInt()).append('\x00');
					c += rx.matchedLength()-1;
					goto end;
				}
			}

			if(rest.startsWith(getCaract(0xe9, 7), Qt::CaseInsensitive)) {//{SPACED CHARACTERS}
				_data.append("\xfe\xe9", 2);
				c += getCaract(0xe9, 7).size()-1;
				goto end;
			}

			if(rest.startsWith("{PAUSE", Qt::CaseInsensitive) && 9<rest.size() && rest.at(9)=='}') {//{PAUSE000}
				value = rest.mid(6,3).toUShort(&ok);
				if(ok) {
					_data.append("\xfe\xdd", 2).append((char)value);
					c += 9;
					goto end;
				}
			}

			if(!jp) {
				if(rest.startsWith(getCaract(0xe0), Qt::CaseInsensitive)) {//{CHOICE}
					_data.append('\xe0');
					c += 7;
					goto end;
				}

				for(i=0 ; i<3 ; ++i) {
					if(rest.startsWith(getCaract(0xe2 + i))) {//{, }{."}{?"}
						_data.append((char)(0xe2 + i));
						c += 3;
						goto end;
					}
				}
			}

			if(string.at(c+1)=='x' && string.at(c+4)=='}') {//{x00}
				value = string.mid(c+2,2).toUShort(&ok,16);
				if(ok && value != 0xff) {
					_data.append((char)value);
					c += 4;
					continue;
				}
			}

			_data.append('\x5b');// {

			continue;
		}

		for(i=0x00 ; i<=0xfe ; ++i)
		{
			if(QString::compare(comp, getCaract(i, jp ? 2 : 0))==0)
			{
				_data.append((char)i);
				goto end;
			}
		}

		if(jp) {
			for(table=3 ; table<8 ; ++table)
			{
				for(i=0x00 ; i<=0xfe ; ++i)
				{
					if(QString::compare(comp, getCaract(i, table))==0)
					{
						switch(table) {
						case 3:
							_data.append('\xfa');
							break;
						case 4:
							_data.append('\xfb');
							break;
						case 5:
							_data.append('\xfc');
							break;
						case 6:
							_data.append('\xfd');
							break;
						case 7:
							_data.append('\xfe');
							break;
						}
						_data.append((char)i);
						goto end;
					}
				}
			}
		}

		end:;
	}
}

QString FF7Text::getCaract(quint8 ord, quint8 table)
{
	switch(table) {
	case 2:
		return QString::fromUtf8(FF7Text::caract_jp[ord]);
	case 3:
		return QString::fromUtf8(FF7Text::caract_jp_fa[ord]);
	case 4:
		return QString::fromUtf8(FF7Text::caract_jp_fb[ord]);
	case 5:
		return QString::fromUtf8(FF7Text::caract_jp_fc[ord]);
	case 6:
		return QString::fromUtf8(FF7Text::caract_jp_fd[ord]);
	case 7:
		return QString::fromUtf8(FF7Text::caract_jp_fe[ord]);
	default:
		return QString::fromUtf8(FF7Text::caract[ord]);
	}
}

bool FF7Text::contains(const QRegExp &regExp) const
{
	return text(Config::value("jp_txt", false).toBool())
			.contains(regExp);
}

int FF7Text::indexOf(const QRegExp &regExp, int from, int &size) const
{
	int index = regExp.indexIn(text(Config::value("jp_txt", false).toBool()), from);
	if(index != -1)		size = regExp.matchedLength();
	return index;
}

int FF7Text::lastIndexOf(const QRegExp &regExp, int &from, int &size) const
{
	QString t = text(Config::value("jp_txt", false).toBool());
	int index = regExp.lastIndexIn(t, from);
	if(index != -1) {
		from = index - t.size();
		size = regExp.matchedLength();
	}
	return index;
}
