TEMPLATE = app
BINDIR = /usr/bin
QT += network \
    xml
include (../../config.pri)
DIALOGS += lastfm \
    medialibdialog \
    streamingdialog \
    firsttimewizard \
    preferencesdialog \
    aboutdialog \
    xmms2dpreferences \
    equalizer
TOOLS += grepshortcutkeydlg \
    globalshortcut
!win32 { 
    for(a, DIALOGS):COMPONENTS += ../dialogs/$$a/lib$${a}.a
    for(a, TOOLS):COMPONENTS += ../tools/$$a/lib$${a}.a
    COMPONENTS += ../lib/liblib.a \
        ../../data/libdata.a
}
win32 { 
    for(a, DIALOGS):COMPONENTS += ../dialogs/$$a/release/lib$${a}.a
    for(a, TOOLS):COMPONENTS += ../tools/$$a/release/lib$${a}.a
    COMPONENTS += ../lib/release/liblib.a \
        ../../data/release/libdata.a
}
LIBS += $$COMPONENTS
PRE_TARGETDEPS = $$COMPONENTS

# Target
TARGET = esperanza
win32 { 
    DESTDIR = $$PWD/../..
    RC_FILE = $$PWD/esperanza.rc
}
unix:!mac { 
    QMAKE_POST_LINK = rm \
        -f \
        ../../esperanza \
        ; \
        ln \
        -s \
        src/ui/esperanza \
        ../../esperanza
    target.path = $$BINDIR
    INSTALLS += target
}
macx:TARGET = Esperanza
SOURCES += fancyplaylistview.cpp \
    minimode.cpp \
    shortcutmanager.cpp \
    filedialog.cpp \
    systemtray.cpp \
    playerbutton.cpp \
    progressframe.cpp \
    volumebar.cpp \
    main.cpp \
    playerwidget.cpp \
    progressindicator.cpp \
    mdns.cpp \
    playlistview.cpp \
    serverdialog.cpp \
    ../../mainwindow.cpp
HEADERS += esperanza_plugin.h \
    fancyplaylistview.h \
    filedialog.h \
    mdns.h \
    minimode.h \
    playerbutton.h \
    playerwidget.h \
    playlistview.h \
    progressframe.h \
    progressindicator.h \
    serverdialog.h \
    shortcutmanager.h \
    systemtray.h \
    volumebar.h \
    ../../mainwindow.h
FORMS += ../../mainwindow.ui
