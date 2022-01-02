/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
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

#ifdef MR_CONSOLE
#include <QCoreApplication>
#include "CLI.h"
#else
#include <QApplication>
#include "Window.h"
#include "core/Var.h"
#include "Data.h"
//#include "core/FF7Font.h"
#endif
#include "core/Config.h"

int main(int argc, char *argv[])
{
#ifdef MR_CONSOLE
	QCoreApplication app(argc, argv);
	QCoreApplication::setApplicationName(MAKOU_REACTOR_NAME);
	QCoreApplication::setApplicationVersion(MAKOU_REACTOR_VERSION);
#ifdef Q_OS_WIN
	// QTextCodec::setCodecForLocale(QTextCodec::codecForName("IBM 850"));
#endif
	Config::set();
	CLI::exec();

	QTimer::singleShot(0, &app, &QCoreApplication::quit);
#else

	QApplication app(argc, argv);
	app.setWindowIcon(QIcon(":/images/logo-shinra.png"));

	QSurfaceFormat format;
#ifdef QT_DEBUG
	format.setOption(QSurfaceFormat::DebugContext);
#endif
	QSurfaceFormat::setDefaultFormat(format);

	Config::set();

	QString lang = QLocale::system().name();
	lang = Config::value("lang", lang.left(lang.indexOf("_")).toLower()).toString();
	QTranslator translator1;
	if (translator1.load("qt_" % lang, Config::programLanguagesDir()) || translator1.load("qt_" % lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
		app.installTranslator(&translator1);
	}
	QTranslator translator2;
	if (translator2.load("Makou_Reactor_" % lang, Config::programLanguagesDir()) || translator2.load("Makou_Reactor_" % lang)) {
		app.installTranslator(&translator2);
		Config::setValue("lang", lang);
	} else {
		Config::setValue("lang", QVariant());
	}

	if (Config::value("dark_theme", false).toBool()) {
#ifndef Q_OS_DARWIN
		qApp->setStyle(QStyleFactory::create("Fusion"));
		QPalette darkPalette;
		QColor disabledColor = QColor(127, 127, 127);
		darkPalette.setColor(QPalette::Window, QColor(0x19, 0x19, 0x19));
		darkPalette.setColor(QPalette::WindowText, Qt::white);
		darkPalette.setColor(QPalette::Base, QColor(0x20, 0x20, 0x20));
		darkPalette.setColor(QPalette::AlternateBase, QColor(0x27, 0x27, 0x27));
		darkPalette.setColor(QPalette::ToolTipBase, QColor(0xFE, 0xFE, 0xFE));
		darkPalette.setColor(QPalette::ToolTipText, QColor(0xFE, 0xFE, 0xFE));
		darkPalette.setColor(QPalette::Text, QColor(0xFE, 0xFE, 0xFE));
		darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
		darkPalette.setColor(QPalette::Button, QColor(0x20, 0x20, 0x20));
		darkPalette.setColor(QPalette::ButtonText, Qt::white);
		darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
		darkPalette.setColor(QPalette::BrightText, Qt::red);
		darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
		darkPalette.setColor(QPalette::LinkVisited, QColor(42, 130, 218));

		darkPalette.setColor(QPalette::Highlight, QColor(0x77, 0x77, 0x77));
		darkPalette.setColor(QPalette::HighlightedText, Qt::white);
		darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);

		darkPalette.setColor(QPalette::Light, QColor(0x34, 0x34, 0x34));
		darkPalette.setColor(QPalette::Midlight, QColor(0x27, 0x27, 0x27));
		darkPalette.setColor(QPalette::Dark, QColor(0x7, 0x7, 0x7));
		darkPalette.setColor(QPalette::Mid, QColor(0x14, 0x14, 0x14));
		darkPalette.setColor(QPalette::Shadow, Qt::black);

		darkPalette.setColor(QPalette::PlaceholderText, QColor(0x53, 0x53, 0x53));

		qApp->setPalette(darkPalette);

		qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
#endif
	}

	if (!Var::load()) {
		QMessageBox::warning(nullptr, QApplication::translate("main", "Error"),
		                     QApplication::translate("main", "The file 'var.cfg' could not be loaded.\n"
		                                                     "Make sure it is valid or delete it."));
	}

	if (!Data::load()) {
		qWarning() << "Error loading data!";
	}

	/* if (!FF7Font::listFonts()) {
		QMessageBox::critical(nullptr, QObject::translate("main", "Data loading"),
			QObject::translate("main", "Fonts couldn't be loaded!"));
		return -1;
	} */

	Window *window = new Window;
	window->show();
	if (argc > 1) {
		window->openFile(app.arguments().at(1));
	}
#endif

	return app.exec();
}
