#include "sensorreader.h"

SensorReader::SensorReader(QObject *parent) :  QObject(parent)
{
    nodeList.clear();
}

void SensorReader::setupThread(QThread &rxThread)
{
    //Този обект се пуска в друга нишка без да наследява QThread
    connect(&rxThread,SIGNAL(started()),this,SLOT(readSensors()));
    thr= &rxThread;
}

void SensorReader::readSensors()
{
    QMutex mutex;
    QString req;
    QByteArray rxData, rxBuff;
    QTime time;
    bool dataProcessed;
    int progress = 0;
    int  i;

    rxBuff.clear();
    mutex.lock();
        temperatures.clear();
    mutex.unlock();

    qDebug() << "SensorReader : readSensors()";
    if(nodeList.count()==0) goto exitThread;
    if(!sp.isOpen()) goto exitThread;

    foreach(int n, nodeList)
    {//Обхождат се контролерите като се пращат запитвания само към адреси записани в
        for(int l = 0 ; l != 3 ; l++)
        {//Изпраща се питане за всеки от 3-те 1-wire интерфейса
            progress++;
            emit Progress(progress);
            mutex.lock();
                if(stopReading)  goto exitThread;
            mutex.unlock();
            dataProcessed=false;
            req = QString("tst %1 %2\r\n").arg(n).arg(l);
            qDebug() << QString("TX: ").toLatin1().data() << req.toLatin1().data();
            sp.clear(QSerialPort::Output);
            sp.write(req.toLatin1().data());
            sp.waitForBytesWritten(100);
            sp.clear(QSerialPort::Input);
            time.start();
            while(!dataProcessed)
            {
                mutex.lock();
                    if(stopReading) goto exitThread;
                mutex.unlock();

                if(time.elapsed() > 1500)
                {
                    qDebug() << QString("ГРЕШКА: Няма връзка с контролер ") << n;                    
                    l=2;
                    break; //Ако няма данни по серииния порт, настъпва таймаут.
                }

                if(sp.bytesAvailable()) rxData = sp.readAll();
                else
                {
                    thr->msleep(20);
                    continue;
                }

                //а данните се добавят в буфера до получаване на символ за край на ред.
                //Когато той пристигне данните се обработват като се извличат МАС адресите
                //и се добавят в списъка
                while(rxData.contains('\n'))
                {
                    rxData.remove(rxData.indexOf('\n'),1);
                }

                while(rxData.count())
                {
                    i=rxData.indexOf('\r');
                    if(i == -1)
                    {//В приетите данни няма символ за край на ред. Само се добавят към буфера.
                        rxBuff.append(rxData);
                        rxData.clear();
                        break;
                    }

                    if(i==0)
                    {//Първия приет символ е \r. махам го и обработвам буфера
                        rxData = rxData.remove(0,1);
                        qDebug() << QString("SensorReader : rxBuf=").toLatin1().data() << QString(rxBuff).toLatin1().data();
                        processSensorData(QString(rxBuff));
                        rxBuff.clear();
                        dataProcessed=true;
                        break;
                    }
                    //Символите преди \r се добавят в буфера и се махат от масива с приетите данни.
                    //Обработката на буфера ще стане в следващия цикъл т.к. сега \r е първия в rxData
                    rxBuff.append(rxData.left(i));
                    rxData.remove(0,i);
                }

                // има приети данни - таймера се рестартира,
                time.start();
            }//while
        }//for(int l = 0 ; l != 3 ; l++)
    }//foreach(int n, activeNodeList)
exitThread:
    thr->exit();
}

void SensorReader::processSensorData(QString data)
{
    //Получава върнатия от контролера низ и извлича от него данните за температурата.
    //Извлечените данни се вкарват в хеша temperatures
    bool ok;
    QString mac,token;
    int node,temp;
    QMutex mutex;

    QStringList tokenList = data.split(' ');

    if(tokenList.count() == 0) return;

    if(tokenList[0]!="nod") return;
    //Имаме данни от датчици
    //отговор от контролера с данни за температура трябва да е по-дълъг от 7
    //отговора е от типа:
    //nod 3 seg 1 dev 4 t[D1B08502]=2600  t[1BA28502]=2600  t[1B5B8700]=2600  t[C7C78502]=2600
    if(tokenList.count()<7) return; //няма данни за температура
    token = tokenList[1];
    node = token.toInt(&ok);
    if(!ok) return; //грешка при извличане на адреса на контролера
    for( int i=6 ; i != tokenList.count() ; i++)
    {
        token = tokenList[i];
        if(!token.startsWith("t[")) continue;
        mac = QString::number(node) + " " + token.mid(2,8);
        temp = token.mid(12).toInt(&ok);
        if(!ok) continue;//грешка при извличане на температурата
        mutex.lock();
            temperatures.insert(mac,temp);
        mutex.unlock();
    }
}

void SensorReader::setSerialPort(QSerialPort::BaudRate baudrate, QString portName)
{
    if(sp.isOpen()) sp.close();
    sp.setPortName(portName);
    sp.open(QIODevice::ReadWrite);
    sp.setBaudRate(baudrate);
    sp.setDataBits(QSerialPort::Data8);
    sp.setParity(QSerialPort::NoParity);
    sp.setStopBits(QSerialPort::OneStop);
    sp.setFlowControl(QSerialPort::NoFlowControl);
}

bool SensorReader::portState() { return sp.isOpen(); }

void SensorReader::closeSerialPort()
{
    if(sp.isOpen()) sp.close();
}

void SensorReader::setNodeList(QList<int> &list)
{
    QMutex mutex;

    if(list.count()==0) return;

    nodeList.clear();
    mutex.lock();
    nodeList.append(list);
    mutex.unlock();
}
