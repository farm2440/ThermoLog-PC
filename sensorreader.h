#ifndef SENSORREADER_H
#define SENSORREADER_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QSerialPort>
#include <QList>
#include <QByteArray>
#include <QTime>
#include <QElapsedTimer>
#include <QDateTime>
#include <QHash>

#include "definitions.h"

class SensorReader : public QObject
{
    Q_OBJECT
public:
    explicit SensorReader(QObject *parent = 0);


public:
    void setupThread(QThread &rxThread);
    void setSerialPort(QSerialPort::BaudRate baudrate,  QString portName);
    void setNodeList(QList<int> &list);

    bool stopReading;
    QHash<QString,int> values;
private:
    QList<int> nodeList; //списък с адресите на нодовете които да бъдат прочетени
    QThread *thr;
    QSerialPort::BaudRate _baudrate;
    QString _portName;

    void processSensorData(QString data);
signals:
    void Progress(int);
    void ReadingDone();
public slots:
    void readSensors();
};

#endif // SENSORREADER_H
