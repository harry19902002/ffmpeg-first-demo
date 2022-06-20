QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

##ffmpeg
FFMPEG_LIB = /usr/local/ffmpeg/lib
FFMPEG_INCLUDE = /usr/local/ffmpeg/include

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += $$FFMPEG_INCLUDE \

LIBS += $$FFMPEG_LIB/libavcodec.so \
        $$FFMPEG_LIB/libavdevice.so \
        $$FFMPEG_LIB/libavfilter.so \
        $$FFMPEG_LIB/libavformat.so \
        $$FFMPEG_LIB/libavutil.so \
        $$FFMPEG_LIB/libswresample.so \
        $$FFMPEG_LIB/libswscale.so \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
