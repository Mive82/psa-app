#include "psa_main_window.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFontDatabase>
#include <iostream>
#include <QSplashScreen>
#include <stdio.h>

int main(int argc, char *argv[])
{


    QApplication a(argc, argv);
    QSplashScreen splash(QPixmap(":/backgrounds/backgrounds/pezo2.png"));
    splash.show();

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "psa-app_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    QFontDatabase::addApplicationFont(":/fonts/fonts/dejavu_sans_mono.ttf");
    QFontDatabase::addApplicationFont(":/fonts/fonts/DroidSansMono.ttf");
    QFontDatabase::addApplicationFont(":/fonts/fonts/AzeretMono-BoldItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/fonts/DSEG7Classic-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/fonts/DSEG14Modern-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/fonts/DSEG14Classic-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/fonts/Dotsalfn.ttf");
    QFontDatabase::addApplicationFont(":/fonts/fonts/PSAGroupeHMISansCS-Bold.ttf");
    psa_main_window w;
    w.show();
    w.display_selected_window();
    a.processEvents();
    splash.finish(&w);
    w.display_selected_window();
    a.processEvents();
    return a.exec();
}
