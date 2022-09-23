QT       += core gui multimedia dbus multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 console

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    psa_badapple_window.cpp \
    psa_main_window.cpp \
    psa_music_window.cpp \
    psa_radio_window.cpp \
    psa_van_receiver.cpp \
    tachometer.cpp \
    tachometer_window.cpp \
    os_helper.cpp \
    libpsa/psa_receive.c

HEADERS += \
    psa_badapple_window.h \
    psa_main_window.h \
    psa_music_window.h \
    psa_radio_window.h \
    psa_van_receiver.h \
    tachometer.h \
    tachometer_window.h \
    os_helper.h \
    libpsa/header/psa/msp_common.h \
    libpsa/header/psa/psa_common.h \
    libpsa/header/psa/psa_packet_defs.h \
    libpsa/header/psa/psa_receive.h \
    libpsa/header/psa/van_packet_iden.h \
    libpsa/header/common/psa_argument_defs.h

LIBS += -ltag

FORMS += \
    psa_badapple_window.ui \
    psa_main_window.ui \
    psa_music_window.ui \
    psa_radio_window.ui \
    tachometer_window.ui

TRANSLATIONS += \
    psa-app_hr_HR.ts
CONFIG += lrelease
CONFIG += embed_translations

QMAKE_CFLAGS += -mthumb -mfpu=neon-vfpv4 -mfloat-abi=hard -mcpu=cortex-a7
QMAKE_CXXFLAGS += -mthumb -mfpu=neon-vfpv4 -mfloat-abi=hard -mcpu=cortex-a7

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    demo-resources.qrc \
    psa-resources.qrc

# DEFINES += PSA_SIMULATE

DISTFILES += \
    README.md
