QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    lib/gui_lib.cpp \
    logic/message.cpp \
    logic/user.cpp \
    loginform.cpp \
    main.cpp \
    mainwindow.cpp \
    profilewindow.cpp \
    registerform.cpp

HEADERS += \
    lib/gui_lib.h \
    logic/message.h \
    logic/user.h \
    loginform.h \
    mainwindow.h \
    profilewindow.h \
    registerform.h

FORMS += \
    loginform.ui \
    mainwindow.ui \
    profilewindow.ui \
    registerform.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
