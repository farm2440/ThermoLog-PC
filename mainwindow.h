#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTextCodec>
#include <QByteArray>
#include <QAction>
#include <QList>
#include <QFile>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QTextCodec>
#include <QTextStream>
#include <QSystemTrayIcon>
#include <QIcon>
#include <QCloseEvent>
#include <QPalette>
#include <QProgressBar>
#include <QSerialPort>
#include <QThread>


#include "definitions.h"
#include "dialogchannelsettings.h"
#include "dialogreport.h"
#include "dialogabout.h"
#include "dialogsettings.h"
#include "channel.h"
#include "sensorreader.h"

/*
    За да се ползва QWT трябва:
    1. В .pro файла трябва да се добави редовете:
    INCLUDEPATH += /home/svilen/work/QtPC/qwt-6.0.1/src

    LIBS += /home/svilen/work/QtPC/qwt-6.0.1/lib/libqwt.so.6.0.1
    LIBS += /home/svilen/work/QtPC/qwt-6.0.1/lib/libqwt.so.6.0
    LIBS += /home/svilen/work/QtPC/qwt-6.0.1/lib/libqwt.so.6
    LIBS += /home/svilen/work/QtPC/qwt-6.0.1/lib/libqwt.so

   2. Библиотеката libqwt.so.6.0.1 трябва да се копира при изпълнимия файл и да се направят връзки с кратките имена към нея

   */
#include <qwt_plot.h>
#include <qwt.h>
#include <qwt_text.h>
#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_picker.h>
#include <qwt_symbol.h>

//qt embedded widgets
//#include <QtBasicDialGauge>
//#include <QtScrollWheel>
#include <QtSvgDialGauge>
//#include <QtSvgToggleSwitch>

struct SettingsStruct
{
    int nextScanHours;
    int nextScanMinutes;
    int startH;
    int startM;
    bool openOnFail;
    int maxFailRetry;
    int maxNodeNum;
    int commType;  //0- rs232, 1- rs485
    QString serialPortName;
};

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:
    void closeEvent(QCloseEvent *event); // Този метод се ре-имплементира за да предупреждава при затваряне ,
                                         // че програмата се минимизира а не се затваря.
private:
    Ui::MainWindow *ui;
    QLabel *lblNextScan; //Етикет за съобщение в статус бара за дата/час на следващия запис
    QLabel *lblComAlarm; //Етикет за съобщение в статус бара за проблем с комуникацията
    QLabel *lblReadWarning;
    QProgressBar *pbrReadingSensors; //Този прогрес бар показва че в момента се четат датчиците
                                     //Става видим когато се извика readSensors(true)
//    QByteArray rxBuff;
    QSqlDatabase database;
    QTimer timer; //Този таймер вика onTimerTick

    QThread txrxThread;
    SensorReader reader;
    enum State {STATE_IDLE, STATE_WAITING_DATA, STATE_DATA_READY} state;
    int retry;

    //Събиране на данни от датчиците:
    int currentNodeIndex, currentLine; //текущите
    QList<Channel> channelsList; //списък с адресите на конфигурираните контролери. Всеки адрес е число от 0 до MAX_NODE_NUM<=15
    QList<int> activeNodeList; // записват се RS485 адресите на контролерите от които ще се събират данни
    bool cancelReadSensors;
    QTime startTime;
    QDateTime nextScanTime;
    bool openOnFail;
    bool stopScan; //този флаг се вдига при озлизане от програмата или при отваряне на диалог за конфигуриране.
    bool doReading;
    bool readNow;//Този флаг се вдига когато от главното меню се избере "Вземи отчет сега"

    int monitorChannelIndex; //това е индекса на канала за който температурата се показва диналично на термометъра горе в ляво

    //За главното меню
    QAction *channelSettingsAction, *settingsAction ,*quitAction, *aboutDialogAction, *reportAction, *readNowAction;
    bool exitFromMenu;

    //Меню на иконата в system tray
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QAction *trayActionQuit, *trayActionMinimize, *trayActionRestore;
    //Диаграма
    QwtPlot *plot;
    QwtPlotCurve *curveA, *curveB;
    QwtSymbol *symA,*symB;
    QwtPlotPicker *picker;
    QList<double> xData, yData;

    //Аналогов термометър
    QtSvgDialGauge *gauge;

    SettingsStruct settings;

public slots:
    void refreshData();
    void channelSettingsUpdate();//вика се в началото а също и след отваряне на диалога за настройки на каналите
    void onTimerTick();
    void onTableDataSelect(int row, int col, int prow, int pcol);
    void onReadProgress(int progress);

    void onReadSensorsDone();
    //Слотове  за менюто на system tray иконата
    void onActionQuit();
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason);

    //За главното меню:
    void channelSettings();
    void onActionAboutDialog();
    void onActionReport();
    void onActionReadNow();
    void onActionSettings();
};

#endif // MAINWINDOW_H
