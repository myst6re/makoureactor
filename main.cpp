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
#include <QApplication>
#include "Window.h"
#include "Var.h"

int main(int argc, char *argv[])
{
	QGLFormat::setDefaultFormat(QGLFormat(QGL::DirectRendering));

	QApplication app(argc, argv);
	app.setWindowIcon(QIcon(":/images/logo-shinra.png"));

	Config::set();

	QString lang = QLocale::system().name().toLower();
	lang = Config::value("lang", lang.left(lang.indexOf("_"))).toString();
	QTranslator translator1;
	if(translator1.load("qt_" % lang, app.applicationDirPath()) || translator1.load("qt_" % lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
		app.installTranslator(&translator1);
	QTranslator translator2;
	if(translator2.load("Makou_Reactor_" % lang, app.applicationDirPath()))
		app.installTranslator(&translator2);
	else
		Config::setValue("lang", "fr");
	
	if(Var::load()==1)
	{
		QMessageBox::warning(0, QObject::tr("Erreur"), QObject::tr("Le fichier 'var.cfg' n'a pas pu être chargé.\nVérifiez que ce fichier est valide ou supprimez-le."));
	}
	Data::load();
	
	Window fenetre;
	fenetre.show();
	if(argc>1)
		fenetre.ouvrir(argv[1]);


	/*
	// some tests for new opcode style
	QByteArray filledFF(128, '\xff'), filled00(128, '\x00'), filledRandom;

	srand(QTime::currentTime().second());

	for(int i=0 ; i<128 ; ++i) {
		filledRandom.append(char(rand() % 256));
	}

	for(int id=0 ; id<256 ; ++id) {
		//if(id == 0x0F || id == 0x28) continue;
		QByteArray op;
		op.append((char)id);
		Opcode *opcode = Script::createOpcode(op + filledFF);
		if(opcode->id() != id) {
			qDebug() << "Opcode error id" << id << opcode->id();
		}
		QByteArray res = opcode->params();
		if(res.size() + 1 != Opcode::length[id]) {
			qDebug() << "Opcode error size" << id << (res.size() + 1) << Opcode::length[id];
		}
		if(res != filledFF.left(res.size())) {
			qDebug() << "Opcode error params" << id << res.toHex() << filledFF.left(res.size()).toHex();
		}
		delete opcode;
		opcode = Script::createOpcode(op + filled00);
		if(opcode->id() != id) {
			qDebug() << "Opcode error id" << id << opcode->id();
		}
		res = opcode->params();
		if(res.size() + 1 != Opcode::length[id]) {
			qDebug() << "Opcode error size" << id << (res.size() + 1) << Opcode::length[id];
		}
		if(res != filled00.left(res.size())) {
			qDebug() << "Opcode error params" << id << res.toHex() << filled00.left(res.size()).toHex();
		}
		delete opcode;
		opcode = Script::createOpcode(op + filledRandom);
		if(opcode->id() != id) {
			qDebug() << "Opcode error id" << id << opcode->id();
		}
		res = opcode->params();
		if(res.size() + 1 != Opcode::length[id]) {
			qDebug() << "Opcode error size" << id << (res.size() + 1) << Opcode::length[id];
		}
		if(res != filledRandom.left(res.size())) {
			qDebug() << "Opcode error params" << id << res.toHex() << filledRandom.left(res.size()).toHex();
		}
		delete opcode;
	}*/

//	Config::setValue("jp_txt", true);
//	bool jp = true/*Config::value("jp_txt", false).toBool()*/;
	/*for(int i=0 ; i<255 ; ++i) {
		for(int j=0 ; j<255 ; ++j) {
			for(int k=0 ; k<255 ; ++k) {
				qDebug() << i << j << k;
				for(int l=0 ; l<255 ; ++l) {
					QByteArray base = QByteArray().append((char)i).append((char)j).append((char)k).append((char)l);
					FF7Text t(base);
					QString text = t.getText(jp);
					t.setText(text, jp);
					if(base != t.getData()) {
						qDebug() << "Erreur" << base.toHex() << t.getData().toHex() << text;
						return app.exec();
					}
				}
			}
		}
	}*/

//	QFile f("C:/Users/vista/Documents/Makou_Reactor/data/WINDOW.BIN/windowPC.3.bin");
//	f.open(QIODevice::ReadOnly);
//	QByteArray data = f.readAll();
//	QString out;

//	int start = 231+210+231+210+210, len = 210;

//	for(int i=start ; i<start+256 ; ++i) {
//		if(i<start+len)
//			out.append(QString("%1, ").arg((int)data.at(i)));
//		else
//			out.append(QString("0, "));
//		if((i+1-start) % 16 == 0) {
//			out.append("\n");
//		}
//	}

//	qDebug() << out;

	return app.exec();
}
