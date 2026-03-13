QT       += core gui network websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    abstractplayer.cpp \
    aiplayer.cpp \
    aithinkworker.cpp \
    boarddata.cpp \
    gamesession.cpp \
    gamewidget.cpp \
    humanplayer.cpp \
    main.cpp \
    mainwindow.cpp \
    networkmanager.cpp \
    offlinechoicewidget.cpp \
    onlinechoicewidget.cpp \
    onlineplayer.cpp \
    onlineroomwidget.cpp \
    pagemanager.cpp \
    startwidget.cpp

HEADERS += \
    SharedType.h \
    abstractplayer.h \
    aiplayer.h \
    aithinkworker.h \
    boarddata.h \
    gamesession.h \
    gamewidget.h \
    humanplayer.h \
    mainwindow.h \
    networkmanager.h \
    offlinechoicewidget.h \
    onlinechoicewidget.h \
    onlineplayer.h \
    onlineroomwidget.h \
    pagemanager.h \
    startwidget.h

FORMS += \
    gamewidget.ui \
    mainwindow.ui \
    offlinechoicewidget.ui \
    onlinechoicewidget.ui \
    onlineroomwidget.ui \
    startwidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

DISTFILES += \
    res/picture/black_chess.png \
    res/picture/game_background.bmp \
    res/picture/white_chess.png
