#-------------------------------------------------
#
# Project created by QtCreator 2014-04-30T17:40:15
#
#-------------------------------------------------

QT       += core gui serialport sql printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ThermoLog
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dialogsettings.cpp \
    channel.cpp \
    dialogabout.cpp \
    dialogchannelsettings.cpp \
    dialogreport.cpp \
    dialogselectchannels.cpp \
    sensorreader.cpp

HEADERS  += mainwindow.h \
    dialogsettings.h \
    channel.h \
    definitions.h \
    dialogabout.h \
    dialogchannelsettings.h \
    dialogreport.h \
    dialogselectchannels.h \
    sensorreader.h

FORMS    += mainwindow.ui \
    dialogsettings.ui \
    dialogabout.ui \
    dialogchannelsettings.ui \
    dialogreport.ui \
    dialogselectchannels.ui

#Икона на приложението - този файл е текстов и съдържа един ред в който се указва път към файла с иконата
RC_FILE = thermologIcon.rc
#---------------------------------------------------------------------------------
# За QWT
#---------------------------------------------------------------------------------
# Добавени за да се ползваа QWT
#Linux
#INCLUDEPATH += /home/svilen/work/QtPC/qwt-6.0.1/src
#Windows
INCLUDEPATH += C:\\Qt\\Qwt-6.1.0\\include


# Библиотеката libqwt.so.6.0.1 трябва да се копира при изпълнимия файл и да се направят връзки с кратките имена към нея
#LIBS += /home/svilen/work/QtPC/qwt-6.0.1/lib/libqwt.so.6.0.1
#LIBS += /home/svilen/work/QtPC/qwt-6.0.1/lib/libqwt.so.6.0
#LIBS += /home/svilen/work/QtPC/qwt-6.0.1/lib/libqwt.so.6
#LIBS += /home/svilen/work/QtPC/qwt-6.0.1/lib/libqwt.so

#Debug
#LIBS += C:\\Qt\\Qwt-6.1.0\\lib\\qwtd.dll
#Release
LIBS += C:\\Qt\\Qwt-6.1.0\\lib\\qwt.dll

#Маркера в диаграмата има 4 знака след запетаята. За да се промени броя им трябва във файла
# qwt_plot_picker.cpp да се промени функцията QwtText QwtPlotPicker::trackerTextF( const QPointF &pos ) const
#            //text.sprintf( "%.4f, %.4f", pos.x(), pos.y() );
#            text.sprintf( "%d, %.1f", (int)pos.x(), pos.y() );
#след промяната библиотеката трябва да се прекомпилира от командния ред на Qt
#
#   cd qwt-6.1.0
#    mingw32-make clean
#    mingw32-make
#    mingw32-make install
#



#---------------------------------------------------------------------------------
#   Za Qt Embedded Widgets
#---------------------------------------------------------------------------------
include(c:\\work\\Qt5\\embedded-widgets-1.1.0\\src\\common\\common.pri)
  #include(e:\\work\\QWT\\embedded-widgets-1.1.0\\src\\svgslideswitch\\svgslideswitch.pri)

  #include(e:\\work\\QWT\\embedded-widgets-1.1.0\\src\\svgbutton\\svgbutton.pri)
  #include(e:\\work\\QWT\\embedded-widgets-1.1.0\\src\\svgtoggleswitch\\svgtoggleswitch.pri)

  #include(e:\\work\\QWT\\embedded-widgets-1.1.0\\src\\5waybutton\\5waybutton.pri)
  #include(e:\\work\\QWT\\embedded-widgets-1.1.0\\src\\basicgraph\\basicgraph.pri)
  #include(e:\\work\\QWT\\embedded-widgets-1.1.0\\src\\multislider\\multislider.pri)
  #include(e:\\work\\QWT\\embedded-widgets-1.1.0\\src\\scrolldial\\scrolldial.pri)
  # omit scrollwheel, it's already included in scrolldial.pri
include(c:\\work\\Qt5\\embedded-widgets-1.1.0\\src\\svgdialgauge\\svgdialgauge.pri)

RESOURCES += \
    c:\\work\\Qt5\\embedded-widgets-1.1.0\\skins\\thermometer_svgdialgauge.qrc \
  #    e:\\work\\QWT\\embedded-widgets-1.1.0\\skins\\tachometer_svgdialgauge.qrc \
  #     e:\\work\\QWT\\embedded-widgets-1.1.0\\skins\\amperemeter_svgdialgauge.qrc \
  #   e:\\work\\QWT\\embedded-widgets-1.1.0\\skins\\beryl_5waybutton.qrc \
  #   e:\\work\\QWT\\embedded-widgets-1.1.0\\skins\\beryl_multislider.qrc \
  #   e:\\work\\QWT\\embedded-widgets-1.1.0\\skins\\beryl_svgslideswitch.qrc \
  #   e:\\work\\QWT\\embedded-widgets-1.1.0\\skins\\beryl_svgbutton.qrc \
  #   e:\\work\\QWT\\embedded-widgets-1.1.0\\skins\\beryl_svgtoggleswitch.qrc \
  #   e:\\work\\QWT\\embedded-widgets-1.1.0\\skins\\berylsquare_svgtoggleswitch.qrc \
  #   e:\\work\\QWT\\embedded-widgets-1.1.0\\skins\\berylsquare_svgbutton.qrc \
  #   e:\\work\\QWT\\embedded-widgets-1.1.0\\skins\\beryl_scrollwheel.qrc \
  #   e:\\work\\QWT\\embedded-widgets-1.1.0\\skins\\beryl_scrolldial.qrc \
  #   e:\\work\\QWT\\embedded-widgets-1.1.0\\skins\\metallicbrush_svgbutton.qrc \
  #   e:\\work\\QWT\\embedded-widgets-1.1.0\\skins\\metallicbrush_svgslideswitch.qrc
    resources.qrc
