#ifndef DIALOGCHANNELSETTINGS_H
#define DIALOGCHANNELSETTINGS_H

#include <QDialog>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QStringList>
#include <QTextCodec>
#include <QMessageBox>
#include <QTimer>
#include <QList>
#include <QRegExp>
#include <QRegExpValidator>
#include <QListWidgetItem>
#include <QSerialPort>
#include <QDebug>

#include "definitions.h"
#include "channel.h"


namespace Ui {
    class DialogChannelSettings;
}

class DialogChannelSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogChannelSettings(QWidget *parent = 0, QString portName="", QSerialPort::BaudRate baud = QSerialPort::Baud1200, int maxNodeNum=16);
    ~DialogChannelSettings();

private:
    Ui::DialogChannelSettings *ui;
    QSerialPort serial;
    bool timeout, cancel;
    int _maxNodeNum;
    QTimer tmr;
    QList<Channel> channelsList;
    QString currentCellText; //При всяко избиране на клетка за редактиране тук се запазва първоначалното и съдържание
    bool checkMACisInUse(QString mac, int skipRow);
private slots:
    void onRefreshMACList();
    void refreshChannelsTable();
    void onCancel();
    void onTimer();
    void onNewChannel();
    void onDeleteChannel();
    void onTabCellChanged(int row, int column);
    void onMACListDoubleclick(QListWidgetItem *itm); //При doubleclick върху ред МАС адреса се присвоява на избрания от таблицата канал
};

#endif // DIALOGCHANNELSETTINGS_H
