#include "dialogchannelsettings.h"
#include "ui_dialogchannelsettings.h"

DialogChannelSettings::DialogChannelSettings(QWidget *parent, QString portName , QSerialPort::BaudRate baud, int maxNodeNum) :  QDialog(parent),   ui(new Ui::DialogChannelSettings)
{
    ui->setupUi(this);

    serial.setPortName(portName);
    serial.open(QIODevice::ReadWrite);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setBaudRate(baud);

    _maxNodeNum = maxNodeNum;
    //Таблица с каналите
    ui->tableChannels->setColumnCount(5);
    QStringList headers;
    headers.append(tr("Име на канал"));
    headers.append(tr("Контролер"));
    headers.append(tr("Адрес"));
    headers.append(tr("Офсет"));
    headers.append(tr("Множител"));
    ui->tableChannels->setColumnWidth(0,140);//Име на канал
    ui->tableChannels->setColumnWidth(1,80); //контролер
    ui->tableChannels->setColumnWidth(2,120);//Адрес
    ui->tableChannels->setColumnWidth(1,70); //офсет
    ui->tableChannels->setColumnWidth(1,70); //множител
    ui->tableChannels->setHorizontalHeaderLabels(headers);    
    refreshChannelsTable();
    connect(ui->tableChannels,SIGNAL(cellChanged(int,int)),this,SLOT(onTabCellChanged(int,int)));
    //Относно обновяването на списъка с адресите
    connect(ui->btnRefreshMACList,SIGNAL(clicked()),this,SLOT(onRefreshMACList()));
    tmr.setSingleShot(true);
    connect(&tmr,SIGNAL(timeout()),this,SLOT(onTimer()));
    ui->progressBar->setVisible(false);
    cancel = false;
    connect(this,SIGNAL(rejected()),this,SLOT(onCancel()));

    connect(ui->btnNewChannel,SIGNAL(clicked()),this,SLOT(onNewChannel()));
    connect(ui->btnDeleteChannel,SIGNAL(clicked()),this,SLOT(onDeleteChannel()));
    connect(ui->btnExit, SIGNAL(clicked()), this, SLOT(accept()));

    connect(ui->listWidgetMAC,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(onMACListDoubleclick(QListWidgetItem*)));
}

DialogChannelSettings::~DialogChannelSettings()
{
    delete ui;
}

void DialogChannelSettings::onRefreshMACList()
{//Праща се запитване като се обхождат всички контролери от 0 до 15.
    QByteArray rxBuff, rxData;
    QString cmd;
    QStringList lst;
    int i,progress = 0;

    ui->listWidgetMAC->clear();
    ui->listWidgetMAC->setEnabled(false);
    ui->tableChannels->setEnabled(false);
    ui->btnRefreshMACList->setEnabled(false);
    ui->btnDeleteChannel->setEnabled(false);
    ui->btnNewChannel->setEnabled(false);
    ui->lineEdit_NewChannelName->setEnabled(false);
    ui->progressBar->setVisible(true);


    for(int node=0 ; node<_maxNodeNum ; node++)
    {
        for(int seg=0 ; seg<3 ;seg++)
        {
            if(cancel)
            {
                tmr.stop();
                return;
            }
            ui->progressBar->setValue(progress++);
            cmd = QString("tst %1 %2\r\n").arg(node).arg(seg);
            serial.clear(QSerialPort::Output);
            serial.write(cmd.toLatin1().data());
            serial.waitForBytesWritten(100);
            serial.clear(QSerialPort::Input);

            timeout=false;
            tmr.start(1000);

            while(true)
            {
                QCoreApplication::processEvents();

                rxData = serial.readAll();
                if(timeout) break; //Ако няма данни по серииния порт, настъпва таймаут.
                if(rxData.count() == 0) continue;

                tmr.stop();
                //ако има приети данни таймера се рестартира,
                //а данните се добавят в буфера до получаване на символ за край на ред.
                //Когато той пристигне данните се обработват като се извличат МАС адресите
                //и се добавят в списъка
                while(rxData.contains('\n'))
                {
                    rxData.remove(rxData.indexOf('\n'),1);
                }

                while(rxData.length())
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
                        //TODO: processRxBuff();
                        lst = QString(rxBuff).split(' ');
                        foreach(QString s, lst)
                        {
                            if(s.startsWith("t[") && s.contains("]="))
                                ui->listWidgetMAC->insertItem(ui->listWidgetMAC->count(), QString::number(node) + "-" + s.mid(2,MAC_LENGTH));
                        }
                        rxBuff.clear();
                        continue;
                    }

                    //Символите преди \r се добавят в буфера и се махат от масива с приетите данни.
                    //Обработката на буфера ще стане в следващия цикъл т.к. сега \r е първия в rxData
                    rxBuff.append(rxData.left(i));
                    rxData.remove(0,i);
                }

                tmr.start(1000);
            }//while(true)
        }
    }

    //Дачиците, чиито адреси са изведени в таблицата с каналите т.е. са използвани се оцветяват
    for(i=0; i!=ui->listWidgetMAC->count() ; i++)
    {
        bool res = checkMACisInUse(ui->listWidgetMAC->item(i)->text().right(8), -1);
        if(res) ui->listWidgetMAC->item(i)->setBackground(QBrush(Qt::lightGray,Qt::SolidPattern));
        else ui->listWidgetMAC->item(i)->setBackground(QBrush(Qt::white,Qt::SolidPattern));
    }

    ui->progressBar->setVisible(false);
    ui->listWidgetMAC->setEnabled(true);
    ui->tableChannels->setEnabled(true);
    ui->btnRefreshMACList->setEnabled(true);
    ui->btnDeleteChannel->setEnabled(true);
    ui->btnNewChannel->setEnabled(true);
    ui->lineEdit_NewChannelName->setEnabled(true);
}

void DialogChannelSettings::onCancel()
{
    cancel=true;
}

void DialogChannelSettings::onTimer()
{
    timeout = true;
}

void DialogChannelSettings::refreshChannelsTable()
{
    QTableWidgetItem *twiChannel, *twiNode, *twiMAC;//, *twiActive;
    QTableWidgetItem *twiOffset, *twiRatio;
    Channel *chan;
    //Запълване на таблицата с каналите с данни от БД
    QSqlQuery qry;
    bool ok;
    //временно разкачам слота за да не се отработват промените направени програмно
    disconnect(ui->tableChannels,SIGNAL(cellChanged(int,int)),this,SLOT(onTabCellChanged(int,int)));
    ui->tableChannels->setRowCount(0);
    channelsList.clear();

    qry.prepare("SELECT * FROM tableChannels;");
    if(!qry.exec())
    {
        QMessageBox::critical(this, tr("Грешка база данни"), tr("Не мога да изпълня SQL заявка SELECT"));
        connect(ui->tableChannels,SIGNAL(cellChanged(int,int)),this,SLOT(onTabCellChanged(int,int)));
        return;
    }
    int row=0;

    while(qry.next())
    {
        chan= new Channel();
        ui->tableChannels->insertRow(row);
        //ID в базата данни
        chan->setID(qry.value(0).toInt(&ok));
        //Име на канал
        twiChannel = new QTableWidgetItem(qry.value(1).toString(),0);
        chan->setName(qry.value(1).toString());
        //Адрес в RS-485
        int node = qry.value(2).toInt(&ok);
        chan->setNode(node);
        if(node==-1) twiNode = new QTableWidgetItem("",0);
        else twiNode = new QTableWidgetItem(QString("%1").arg(node),0);
        // 1-wire адрес
        twiMAC = new QTableWidgetItem(qry.value(3).toString(),3);
        chan->setAddress(qry.value(3).toString());
        //Активен - Check box е маркиран ако за този канал се събират данни.
        int chk = qry.value(4).toInt(&ok);
        chan->setActive(chk!=0);
        //twiActive = new QTableWidgetItem();
        //twiActive->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        //if(chk) twiActive->setCheckState(Qt::Checked);
        //else twiActive->setCheckState(Qt::Unchecked);

        //Offset
        twiOffset = new QTableWidgetItem(qry.value(5).toString(),0);
        chan->setOffset(qry.value(5).toDouble());
        //Ratio
        twiRatio = new QTableWidgetItem(qry.value(6).toString(),0);
        chan->setRatio(qry.value(6).toDouble());
        //запис на реда в таблицата
        ui->tableChannels->setItem(row,0,twiChannel);
        ui->tableChannels->setItem(row,1,twiNode);
        ui->tableChannels->setItem(row,2,twiMAC);        
        ui->tableChannels->setItem(row,3,twiOffset);
        ui->tableChannels->setItem(row,4,twiRatio);
        //ui->tableChannels->setItem(row,3,twiActive);
        channelsList.append(*chan);
        row++;
    }
    connect(ui->tableChannels,SIGNAL(cellChanged(int,int)),this,SLOT(onTabCellChanged(int,int)));
}

void DialogChannelSettings::onNewChannel()
{
    QString name,sql;
    QRegExp rxName(tr("^[a-zA-Zа-яА-Я0-9 ]{1,30}$"));
    QRegExpValidator vName(rxName,this);
    bool ok;
    int i,pos=0;
    QSqlQuery qry;

    name = ui->lineEdit_NewChannelName->text();
    ok = vName.validate(name,pos);
    if((name=="") || (!ok))
    {//Проверка за празно име
        QMessageBox::critical(this,tr("ГРЕШКА!"),tr("Не е въведено валидно име за канал!\nПозволени са само букви и цифри до 30 символа.\nИмето не може да бъде празно."));
        return;
    }

    for(i=0 ; i!=ui->tableChannels->rowCount() ; i++)
    {//Проверка за повтарящо се име
        if(ui->tableChannels->item(i,0)->text() == name)
        {
            QMessageBox::critical(this,tr("ГРЕШКА!"),tr("Вече има канал с такова име!"));
            return;
        }
    }

    sql = QString(tr("INSERT INTO tableChannels VALUES (NULL,'%1',0,'',1,0.0,1.0);").arg(name));
    qry.prepare(sql);
    if(!qry.exec())
    {
        QMessageBox::critical(this, tr("Грешка база данни"), tr("Не мога да изпълня SQL заявка INSERT"));
        refreshChannelsTable(); //слота тук ще бъде закачен
        return;
    }
    refreshChannelsTable();
}

void DialogChannelSettings::onDeleteChannel()
{//Текущо избрания канал се изтрива от базата данни
    int i;
    int row = ui->tableChannels->currentRow();
    if(row==-1)
    {
        QMessageBox::critical(this,tr("ГРЕШКА"),tr("Няма избран канал в таблицата!"));
        return;
    }

    int id = channelsList[row].id();

    int res = QMessageBox::warning(this,tr("ВНИМАНИЕ"),
                         QString(tr("Моля, потвърдете каналът '%1' да бъде изтрит!\nВсички записи в БД за този канал също ще бъдат изтрити!").arg(channelsList[row].name())),
                         QMessageBox::Ok,
                         QMessageBox::Cancel);
    if(res==QMessageBox::Cancel) return;

    //Изтрива се канала от таблицата tableChannels
    QSqlQuery qry;
    QString sql = QString("DELETE FROM tableChannels WHERE id=%1;").arg(id);
    qry.prepare(sql);
    if(!qry.exec())
    {
        QMessageBox::critical(this, tr("Грешка база данни"), tr("Не мога да изпълня SQL заявка DELETE за tableChannels"));
        return;
    }
    //А също се изтриват и всички отчети за температура за този канал от tableLog
    sql = QString("DELETE FROM tableLog WHERE ChannelId=%1;").arg(id);
    qry.prepare(sql);
    if(!qry.exec())
    {
        QMessageBox::critical(this, tr("Грешка база данни"), tr("Не мога да изпълня SQL заявка DELETE за tableLog"));
        return;
    }

    refreshChannelsTable();

    //Дачиците, чиито адреси са изведени в таблицата с каналите т.е. са използвани се оцветяват
    for(i=0; i!=ui->listWidgetMAC->count() ; i++)
    {
        bool res = checkMACisInUse(ui->listWidgetMAC->item(i)->text().right(8), -1);
        if(res) ui->listWidgetMAC->item(i)->setBackground(QBrush(Qt::lightGray,Qt::SolidPattern));
        else ui->listWidgetMAC->item(i)->setBackground(QBrush(Qt::white,Qt::SolidPattern));
    }
}

void DialogChannelSettings::onTabCellChanged(int row, int column)
{

    QString s;
    QString currentCellText, sql;
    QSqlQuery qry;
    QRegExp rxName(tr("^[a-zA-Zа-яА-Я0-9 ]{1,30}$"));
    QRegExp rxMAC(tr("^[A-F0-9]{16}$"));
    QRegExp rxReal(tr("(^-?[0-9][0-9]*[.]?[0-9][0-9]*$)")); // "-?" Опция започва с -, "[0-9]" една цифра 0-9, "[0-9]*" може една или повече цифри, "[.]?  - опция точка
    QRegExpValidator vName(rxName,this);
    QRegExpValidator vMAC(rxMAC,this);
    QRegExpValidator vReal(rxReal,this);
    bool ok;
    int i,pos=0;

    s = ui->tableChannels->item(row,column)->text();
    disconnect(ui->tableChannels,SIGNAL(cellChanged(int,int)),this,SLOT(onTabCellChanged(int,int)));
    switch(column)
    {
    case 0://Сменено е име на канал
        currentCellText = channelsList[row].name();
        ok = vName.validate(s,pos);
        if((s=="") || (!ok))
        {//Проверка за празно име
            QMessageBox::critical(this,tr("ГРЕШКА!"),tr("Не е въведено валидно име за канал!\nПозволени са само букви и цифри до 30 символа.\nИмето не може да бъде празно."));
            ui->tableChannels->item(row,column)->setText(currentCellText); //Възстановява се старото съдържание
            connect(ui->tableChannels,SIGNAL(cellChanged(int,int)),this,SLOT(onTabCellChanged(int,int)));
            return;
        }
        for(i=0 ; i!=ui->tableChannels->rowCount() ; i++)
        {//Проверка за повтарящо се име
            if(i==row) continue;
            if(ui->tableChannels->item(i,0)->text() == s)
            {
                QMessageBox::critical(this,tr("ГРЕШКА!"),tr("Вече има канал с такова име!"));
                ui->tableChannels->item(row,column)->setText(currentCellText); //Възстановява се старото съдържание
                connect(ui->tableChannels,SIGNAL(cellChanged(int,int)),this,SLOT(onTabCellChanged(int,int)));
                return;
            }
        }

        break;
    case 1://Променен е номер на контролер (RS-485 адрес)
        currentCellText = QString::number(channelsList[row].node());
        i = s.toInt(&ok);
        if( (!ok) || (i<0) || (i>_maxNodeNum-1) )
        {
            QMessageBox::critical(this,tr("ГРЕШКА!"),tr("Невалиден номер на контролер!"));
            ui->tableChannels->item(row,column)->setText(currentCellText); //Възстановява се старото съдържание
            connect(ui->tableChannels,SIGNAL(cellChanged(int,int)),this,SLOT(onTabCellChanged(int,int)));
            return;
        }
        break;
    case 2://Променен е МАС адрес
        currentCellText = channelsList[row].address();
        ok = vMAC.validate(s,pos);
        if((s.length()!= MAC_LENGTH) || (!ok))
        {
            QMessageBox::critical(this,tr("ГРЕШКА!"),tr("Невалиден адрес!\nДопустими са символи A-F(главни),0-9.\nДължина точно 16 символа."));
            ui->tableChannels->item(row,column)->setText(currentCellText); //Възстановява се старото съдържание
            connect(ui->tableChannels,SIGNAL(cellChanged(int,int)),this,SLOT(onTabCellChanged(int,int)));
            return;
        }
        if(checkMACisInUse(s,row))
        {
            QMessageBox::critical(this,tr("ГРЕШКА!"),tr("Датчик въведения адрес е вече използван!"));
            ui->tableChannels->item(row,column)->setText(currentCellText); //Възстановява се старото съдържание
            connect(ui->tableChannels,SIGNAL(cellChanged(int,int)),this,SLOT(onTabCellChanged(int,int)));
            return;
        }
        break;
    case 3://Променен е офсет
        currentCellText =QString::number(channelsList[row].offset());
        ok = vReal.validate(s,pos);
        if((s.length()==0) || (!ok))
        {
            QMessageBox::critical(this,tr("ГРЕШКА"),tr("Невалидна стойност!"));
            ui->tableChannels->item(row,column)->setText(currentCellText);
            connect(ui->tableChannels,SIGNAL(cellChanged(int,int)),this,SLOT(onTabCellChanged(int,int)));
            return;
        }
        break;
    case 4://Променен е множител
        currentCellText =QString::number(channelsList[row].ratio());
        ok = vReal.validate(s,pos);
        if((s.length()==0) || (!ok))
        {
            QMessageBox::critical(this,tr("ГРЕШКА"),tr("Невалидна стойност!"));
            ui->tableChannels->item(row,column)->setText(currentCellText);
            connect(ui->tableChannels,SIGNAL(cellChanged(int,int)),this,SLOT(onTabCellChanged(int,int)));
            return;
        }
        if(ui->tableChannels->item(row,4)->text().toDouble() == 0.0)
        {
            QMessageBox::critical(this,tr("ГРЕШКА"),tr("Множителя не може да бъде 0!"));
            ui->tableChannels->item(row,column)->setText(currentCellText);
            connect(ui->tableChannels,SIGNAL(cellChanged(int,int)),this,SLOT(onTabCellChanged(int,int)));
            return;
        }
        break;
    }
    //QMessageBox::information(this,"",QString("row=%1 , col=%2 data=%3 curCell=%4").arg(row).arg(column).arg(s).arg(currentCellText));

    sql = QString("UPDATE tableChannels SET Channel='%1',Node=%2, MAC='%3', Offset='%4', Ratio='%5' WHERE ID=%6;")
                .arg(ui->tableChannels->item(row,0)->text())
                .arg(ui->tableChannels->item(row,1)->text())
                .arg(ui->tableChannels->item(row,2)->text())
                .arg(ui->tableChannels->item(row,3)->text())
                .arg(ui->tableChannels->item(row,4)->text())
                .arg(channelsList[row].id());
    qry.prepare(sql);
    if(!qry.exec())
    {
        QMessageBox::critical(this, tr("Грешка база данни"), tr("Не мога да изпълня SQL заявка UPDATE"));
        refreshChannelsTable(); //слота тук ще бъде закачен
        return;
    }

    //Дачиците, чиито адреси са изведени в таблицата с каналите т.е. са използвани се оцветяват
    for(i=0; i!=ui->listWidgetMAC->count() ; i++)
    {
        bool res = checkMACisInUse(ui->listWidgetMAC->item(i)->text().right(8), -1);
        if(res) ui->listWidgetMAC->item(i)->setBackground(QBrush(Qt::lightGray,Qt::SolidPattern));
        else ui->listWidgetMAC->item(i)->setBackground(QBrush(Qt::white,Qt::SolidPattern));
    }

    channelsList[row].setName( ui->tableChannels->item(row,0)->text());
    channelsList[row].setNode( ui->tableChannels->item(row,1)->text().toInt(&ok));
    channelsList[row].setAddress( ui->tableChannels->item(row,2)->text());
    channelsList[row].setOffset(ui->tableChannels->item(row,3)->text().toDouble());
    channelsList[row].setRatio( ui->tableChannels->item(row,4)->text().toDouble());
    connect(ui->tableChannels,SIGNAL(cellChanged(int,int)),this,SLOT(onTabCellChanged(int,int)));
}

bool DialogChannelSettings::checkMACisInUse(QString mac, int skipRow)
{
    //Проверява дали в таблицата с каналите има запис с такъв МАС адрес.
    //Ако има връща true. не се проверява реда skipRow
    int j;
    for(j=0 ; j!=ui->tableChannels->rowCount() ; j++)
    {
        if(j==skipRow) continue;
        if(ui->tableChannels->item(j,2)->text() == mac) return true;
    }
    return false;
}

void DialogChannelSettings::onMACListDoubleclick(QListWidgetItem *itm)
{
    int chnRow = ui->tableChannels->currentRow();
    if(itm==NULL) return;
    if(chnRow<0) return;

    QStringList sl = itm->text().split('-');
    if(sl.count()!=2) return;
    ui->tableChannels->item(chnRow,1)->setText(sl[0]);
    ui->tableChannels->item(chnRow,2)->setText(sl[1]);
}
