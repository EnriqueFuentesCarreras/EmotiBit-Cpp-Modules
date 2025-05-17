QT      += core gui network widgets printsupport  charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    channelfrequencies.cpp \
    doublebuffer.cpp \
    emotibitcontroller.cpp \
    emotibitwifirobotea.cpp \
    formplot.cpp \
    formvistaemotibit.cpp \
    main.cpp \
    mainwindow.cpp \
    qemotibitpacket.cpp

HEADERS += \
    channelfrequencies.h \
    doublebuffer.h \
    emotiBitComms.h \
    emotibitcontroller.h \
    emotibitwifirobotea.h \
    formplot.h \
    formvistaemotibit.h \
    mainwindow.h \
    qemotibitpacket.h

FORMS += \
    formplot.ui \
    formvistaemotibit.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
