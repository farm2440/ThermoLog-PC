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
#include <QHash>

class SensorReader : public QObject
{
    Q_OBJECT
public:
    explicit SensorReader(QObject *parent = 0);


public:
    void setupThread(QThread &rxThread);
    void setSerialPort(QSerialPort::BaudRate baudrate,  QString portName);
    void closeSerialPort();
    void setNodeList(QList<int> &list);

    bool stopReading;
    QHash<QString,int> temperatures;
private:
    QSerialPort sp;
    QList<int> nodeList; //списък с адресите на нодовете които да бъдат прочетени
    QThread *thr;

    void processSensorData(QString data);
signals:
    void Progress(int);
    void ReadingDone();
public slots:
    void readSensors();
    bool portState();
};

#endif // SENSORREADER_H
