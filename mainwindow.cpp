#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :    QMainWindow(parent),    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    lblNextScan = new QLabel(this);
    ui->statusBar->addWidget(lblNextScan);

    lblReadWarning = new QLabel(this);
    ui->statusBar->addWidget(lblReadWarning);
    QPalette pal;
    pal = lblReadWarning->palette();
    pal.setColor(lblReadWarning->backgroundRole(),Qt::yellow);
    lblReadWarning->setPalette(pal);
    lblReadWarning->setAutoFillBackground(true);
    lblReadWarning->setText(tr("  ВЗЕМАНЕ НА ОТЧЕТИ...  "));
    lblReadWarning->setVisible(false);

    pbrReadingSensors = new QProgressBar(this);
    ui->statusBar->addWidget(pbrReadingSensors);
    pbrReadingSensors->setToolTip(tr("Прочитане на датчиците"));
    pbrReadingSensors->setVisible(false);

    lblComAlarm = new QLabel(this);
    ui->statusBar->addWidget(lblComAlarm);
    pal = lblComAlarm->palette();
    pal.setColor(lblComAlarm->backgroundRole(),Qt::red);
    lblComAlarm->setPalette(pal);
    lblComAlarm->setAutoFillBackground(true);
    lblComAlarm->setVisible(false);

    //диаграма
    plot = new QwtPlot();
    QHBoxLayout *layoutPlot = new QHBoxLayout(ui->frameDiagram);
    layoutPlot->setSpacing(0);
    layoutPlot->addWidget(plot,Qt::StretchTile);
    plot->setAutoReplot(true);
    //имена на осите
    plot->setAxisTitle(0,tr("Температура"));
    plot->setAxisTitle(2,tr("Време"));
    //Ляаявата вертикална ос за температура
    plot->setAxisScale(0,-40,40,10);
    plot->setAxisScale(2,200,0,10);
    // фон
    plot->setCanvasBackground(QBrush(Qt::darkBlue, Qt::SolidPattern));
    // grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->setMinorPen(QPen(Qt::darkGray, 0 , Qt::DotLine));
    grid->attach(plot);
    //picker
    picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,plot->canvas());
    picker->setTrackerPen(QColor(Qt::yellow));
    //легенди за кривите
    QwtLegend *legend = new QwtLegend();
    plot->insertLegend(legend,QwtPlot::RightLegend);
    //Криви
    curveA = new QwtPlotCurve("TEMP");
    //curveA->setRenderHint(QwtPlotItem::RenderAntialiased);
    curveA->setPen(QPen(Qt::red));
    curveA->setLegendAttribute(QwtPlotCurve::LegendShowLine);
    curveA->setYAxis(0);
    curveA->attach(plot);

    //Аналогов термометър
    gauge = new QtSvgDialGauge(this);
    gauge->setSkin("Thermometer");
    gauge->setShowOverlay(false);//Без стъклен капак
    gauge->setNeedleOrigin(0.456, 0.459);
    gauge->setMinimum(-30);
    gauge->setMaximum(50);
    gauge->setStartAngle(-90);
    gauge->setEndAngle(150);
    gauge->setValue(0);
    gauge->setMaximumSize(200, 200);
    ui->layoutAnalogThermometer->addWidget(gauge);

    ui->splitter->setStretchFactor(1,1);//това прибира сплитера максимално нагоре

    //отваряне на базата данни
    QFile dbFile("thermolog.db");
    if(!dbFile.exists()) QMessageBox::critical(this, tr("Грешка база данни"), tr("Файлът с БД не съществува"));
    else
    {   database = QSqlDatabase::addDatabase("QSQLITE"); //QSQLITE е за версия 3 и нагоре, QSQLITE2 e за версия 2
        database.setDatabaseName("thermolog.db");
        if(!database.open()) QMessageBox::critical(this, tr("Грешка база данни"), tr("Не мога да отворя БД"));
    }

    //Зареждане на текущите настройки от БД:
    QSqlQuery qry;
    bool ok;
    qry.prepare("SELECT * FROM tableSettings;");
    if(!qry.exec()) QMessageBox::critical(this, tr("Грешка база данни"), tr("Не мога да заредя настройките от БД!"));
    if(qry.next())
    {
        settings.nextScanHours   = qry.value(0).toInt(&ok);
        settings.nextScanMinutes = qry.value(1).toInt(&ok);
        settings.startH          = qry.value(2).toInt(&ok);
        settings.startM          = qry.value(3).toInt(&ok);
        settings.openOnFail      = qry.value(4).toInt(&ok);
        settings.maxFailRetry    = qry.value(5).toInt(&ok);
        settings.maxNodeNum      = qry.value(6).toInt(&ok);
        settings.commType        = qry.value(7).toInt(&ok);
        settings.serialPortName      = qry.value(8).toString();

    }else
    {
        QMessageBox::critical(this, tr("Грешка база данни"), tr("Не мога да заредя настройките от БД!"));
        this->close();
    }

    startTime.setHMS(settings.startH,settings.startM,0,0);
    //пресмятане за момента на следващия отчет nextScanTime
    nextScanTime.setDate(QDateTime::currentDateTime().date());
    nextScanTime.setTime(startTime);
    nextScanTime = nextScanTime.addDays(-1);
    while(nextScanTime < QDateTime::currentDateTime())
    {
        nextScanTime = nextScanTime.addSecs(settings.nextScanMinutes*60 + settings.nextScanHours*3600);
    }
    lblNextScan->setText(tr("Следващ отчет: ") + nextScanTime.toString(Qt::LocalDate) + "  ");
    channelSettingsUpdate(); //Актуализират се по БД channelsList и activeNodeList
    refreshData();    //таблицата със събраните данни се актуализира. Показват се данните за каналите

    //Зареждат се от файл  ширините на колоните
    QFile file("columns.txt");
    if(file.exists())
    {
        QTextStream s(&file);
        int w;
        bool ok;
        file.open(QIODevice::ReadOnly);
        for(int i=0 ; i != ui->tableWidgetData->columnCount() ; i++)
        {
            if(s.atEnd()) break;
            w = s.readLine().toInt(&ok);
            if(!ok) break;
            ui->tableWidgetData->setColumnWidth(i,w);
        }
        file.close();
    }

    //Текущо показвана температура на канал
    if(channelsList.count())
    {
        monitorChannelIndex = 0;
        ui->groupBoxCurrentChannel->setTitle(channelsList[0].name());
    }
    else
    {//Ако няма нито един канал конфигуриран
        monitorChannelIndex = -1;
        ui->groupBoxCurrentChannel->setTitle("---");
    }
    ui->lblTemp->setText("");

    connect(ui->tableWidgetData,SIGNAL(currentCellChanged(int,int,int,int)),this,SLOT(onTableDataSelect(int,int,int,int)));
    //Главно меню
    QMenu *settingsMenu = ui->menuBar->addMenu(tr("Настройки"));
    channelSettingsAction = settingsMenu->addAction(tr("Канали"));
    connect(channelSettingsAction,SIGNAL(triggered()),this,SLOT(channelSettings()));
    settingsAction = settingsMenu->addAction(tr("Работни настройки"));
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(onActionSettings()));
    settingsMenu->addSeparator();
    quitAction = settingsMenu->addAction(tr("Изход"));
    QMenu *reportMenu = ui->menuBar->addMenu(tr("Справки"));
    reportAction = reportMenu->addAction(tr("Изготвяне на спрвака"));
    connect(reportAction, SIGNAL(triggered()), this, SLOT(onActionReport()));
    readNowAction = reportMenu->addAction(tr("Вземи отчет сега"));
    connect(readNowAction, SIGNAL(triggered()), this, SLOT(onActionReadNow()));
    aboutDialogAction = ui->menuBar->addAction(tr("За програмата"));
    connect(aboutDialogAction, SIGNAL(triggered()), this, SLOT(onActionAboutDialog()));


    connect(quitAction,SIGNAL(triggered()),this,SLOT(onActionQuit()));
    exitFromMenu=false;

    //system tray икона и меню
    trayActionMinimize = new QAction(tr("Скриване"),this);
    trayActionRestore = new QAction(tr("Отваряне"),this);
    trayActionQuit = new QAction(tr("Изход"), this);
    connect(trayActionMinimize, SIGNAL(triggered()), this, SLOT(hide()));
    connect(trayActionRestore , SIGNAL(triggered()), this, SLOT(showNormal()));
    connect(trayActionQuit    , SIGNAL(triggered()), this, SLOT(onActionQuit()));
    trayMenu = new QMenu(this);
    trayMenu->addAction(trayActionMinimize);
    trayMenu->addAction(trayActionRestore);
    trayMenu->addSeparator();
    trayMenu->addAction(trayActionQuit);
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/ico/res/thermo.ico"));
    trayIcon->setToolTip(tr("Thermolog"));
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
    //икона на главния прозорец
    this->setWindowIcon(QIcon(":/ico/res/thermo.ico"));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this, SLOT(onTrayIconActivated(QSystemTrayIcon::ActivationReason)));

    state=STATE_IDLE;
    reader.setNodeList(activeNodeList);
    reader.stopReading=false;
    if(settings.commType) reader.setSerialPort(QSerialPort::Baud1200, settings.serialPortName); //RS-485
    else reader.setSerialPort(QSerialPort::Baud115200, settings.serialPortName); //RS-232
    reader.setupThread(txrxThread);
    reader.moveToThread(&txrxThread);
    connect(&txrxThread, SIGNAL(finished()), this, SLOT(onReadSensorsDone()));
    connect(&reader, SIGNAL(Progress(int)), this, SLOT(onReadProgress(int)));

    //таймер за обхождане
    doReading=false;
    readNow=false;
    stopScan=false;
    connect(&timer,SIGNAL(timeout()),this,SLOT(onTimerTick()));
    timer.setInterval(5000);
    timer.setSingleShot(true);
    timer.start();


}

MainWindow::~MainWindow()
{
    qDebug() << "~MainWindow()";
    if(database.isOpen()) database.close();
    if(reader.portState()) reader.closeSerialPort();
    if(txrxThread.isRunning()) txrxThread.exit();
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(exitFromMenu)
    {
        qDebug() << "Exit from menu. Event accepted...";
        if(reader.portState()) reader.closeSerialPort();
        event->accept();
    }
    else
    {
        if (trayIcon->isVisible()) {
            QMessageBox::information(this, tr("Thermolog "),
                                     tr("Програмата ще остане активна!"
                                        " Ако искате да я прекратите  "
                                        "изберете <b>Изход</b> от контекстното меню. "));
            hide();
            event->ignore();
        }
    }
}

void MainWindow::channelSettings()
{
    QSerialPort::BaudRate baud;

    if(txrxThread.isRunning()) reader.stopReading = true;
    txrxThread.wait(1500);
    reader.closeSerialPort();
    if(txrxThread.isRunning()) qDebug() << "Entering channel settings but reader thread is still running!";

    if(settings.commType) baud=QSerialPort::Baud1200;//RS-485
    else baud=QSerialPort::Baud115200;//USB

    DialogChannelSettings dlg(this, settings.serialPortName, baud,settings.maxNodeNum);
    dlg.exec();
    lblComAlarm->setText("");
    lblComAlarm->setVisible(false);//Изтрива се евентуално останало съобщение за грешка
    channelSettingsUpdate(); //Актуализират се по БД channelsList и activeNodeList
    refreshData();    //таблицата със събраните данни се актуализира. Показват се данните за каналите    
}

void MainWindow::refreshData()
{
    //Таблицата с отчетите се запълва наново
    QSqlQuery qry;
    QString strQuery;
    bool ok;
    QTableWidgetItem *itm;

    ui->tableWidgetData->clear();
    ui->tableWidgetData->setRowCount(0);
    xData.clear();
    yData.clear();
    //Оразмерява се броя колони. Той е броя канали + колона за дата/час
    ui->tableWidgetData->setColumnCount(channelsList.count()+1);
    //Поставят се имена на колоните
    QStringList headers;
    headers.append(tr("Време\nна отчет"));
    foreach(Channel chn , channelsList)  headers.append(chn.name());
    ui->tableWidgetData->setHorizontalHeaderLabels(headers);

    //Извличат се данните от БД и се нанасят в таблицата
    //Извличат се толкова отчета, че да се запълни MAX_DATA_DISPLAYED броя редове
    strQuery = QString("SELECT * FROM tableLog ORDER BY Timestamp DESC LIMIT %1;").arg(MAX_DATA_DISPLAYED * channelsList.count());
    qry.setForwardOnly(true); //Това значително ускорява работата и минимизира ползваната памет
    qry.prepare(strQuery);
    if(!qry.exec()) QMessageBox::critical(this, tr("Грешка база данни"), tr("Не мога да изпълня SQL заявка SELECT"));

    int row=0,chId;
    QString pTimestamp = "", timestamp;
    int nTmp;
    double dTemp;

    while(qry.next())
    {
        timestamp = qry.value(0).toString();
        if(pTimestamp != timestamp)
        {//Ако Това е нова стойност за дата час. Добавя се нов ред а датата/час се слага в първата колона.
            pTimestamp=timestamp;
            row=ui->tableWidgetData->rowCount();
            ui->tableWidgetData->insertRow(row);
            itm = new QTableWidgetItem( timestamp );
            ui->tableWidgetData->setItem(row,0,itm);
        }
        //извличаме температурата и я слагаме в колоната за съответния канал
        chId = qry.value(1).toInt(&ok);
        nTmp = qry.value(2).toInt(&ok);
        dTemp = (double)nTmp/100;
        itm = new QTableWidgetItem(QString::number(dTemp)); //температура
        itm->setTextAlignment(Qt::AlignCenter);
        //по ID на канала се разбира в коя колона да се сложи температурата
        for(int i=0; i!=channelsList.count() ; i++)
        {
            if(channelsList[i].id()==chId)
            {
                ui->tableWidgetData->setItem(row,i+1,itm);
                if( i == monitorChannelIndex )
                {
                    xData.append((double)row);
                    yData.append(dTemp);
                }
                break;
            }
        }
    }    
    //refreshPlot();
    curveA->setSamples(xData.toVector(), yData.toVector());
}

void MainWindow::channelSettingsUpdate()
{   //Тази функция се вика в началото и след промяна в списъка на каналите.
    //Тя съгласува проемнливите със записите в БД в tableChannels
    QSqlQuery qry;
    bool ok;
    Channel chan;

    activeNodeList.clear();
    channelsList.clear();

    //1. В channelsList се записват настройките на каналите за които ще се събират данни
    qry.prepare("SELECT * FROM tableChannels;");
    if(!qry.exec())
    {
        QMessageBox::critical(this, tr("Грешка база данни"), tr("Не мога да изпълня SQL заявка SELECT"));
        return;
    }

    while(qry.next())
    {   //ID в базата данни
        chan.setID( qry.value(0).toInt(&ok));
        //Име на канал
        chan.setName(qry.value(1).toString());
        //Адрес в RS-485
        int node = qry.value(2).toInt(&ok);
        chan.setNode( node);
        // 1-wire адрес
        chan.setAddress(qry.value(3).toString());

        //Записваат се данни само за канали с коректен адрес и номер на контролер
        if((chan.node()<0) || (chan.node()>15)) continue;
        if(chan.address().length() !=8 ) continue;

        channelsList.append(chan);
    }

    //2. в activeNodeList се записват адресите на контролерите от които ще се събират данни
    qry.prepare("SELECT DISTINCT Node FROM tableChannels;");
    if(!qry.exec())
    {
        QMessageBox::critical(this, tr("Грешка база данни"), tr("Не мога да изпълня SQL заявка SELECT DISTINCT"));
        return;
    }

    while(qry.next())
    {
        int node = qry.value(0).toInt(&ok);
        if((node>=0) && (node<=15)) activeNodeList.append(node);
    }

    currentNodeIndex=0;
    currentLine=0;
    return;
}

void MainWindow::onActionSettings()
{
    stopScan=true;
    DialogSettings dlg(this);
    dlg.initValues(startTime,
                   settings.nextScanHours,
                   settings.nextScanMinutes,
                   settings.openOnFail,
                   settings.maxFailRetry,
                   settings.commType,
                   settings.maxNodeNum,
                   settings.serialPortName);
    if(dlg.exec())
    {
        startTime = dlg.startTime;
        settings.startH = startTime.hour();
        settings.startM = startTime.minute();
        settings.nextScanHours = dlg.hours;
        settings.nextScanMinutes = dlg.minutes;
        settings.openOnFail = dlg.openOnFail;
        settings.maxFailRetry = dlg.maxFail;
        settings.commType = dlg.commType;
        settings.maxNodeNum = dlg.maxNodeNum;
        settings.serialPortName = dlg.serialPort;

        //Новите настройки се записват в БД
        QSqlQuery qry;
        QString str = QString("UPDATE tableSettings SET startH=%1, startM=%2, nextScanHours=%3, nextScanMinutes=%4, openOnFail=%5, maxFailRetry=%6, commType=%7, maxNodeNum=%8, serialPort='%9';")
                      .arg(settings.startH).arg(settings.startM)
                      .arg(settings.nextScanHours).arg(settings.nextScanMinutes)
                      .arg(settings.openOnFail)
                      .arg(settings.maxFailRetry)
                      .arg(settings.commType)
                      .arg(settings.maxNodeNum)
                      .arg(settings.serialPortName);
        qry.prepare(str);
        if(!qry.exec()) QMessageBox::critical(this, tr("Грешка база данни"), tr("Не мога да запазя настройките от БД!"));

        //Новите настройки се вкарват в действие
        //Сериен порт

        startTime.setHMS(settings.startH,settings.startM,0,0);


        //пресмятане за момента на следващия отчет nextScanTime
        nextScanTime.setDate(QDateTime::currentDateTime().date());
        nextScanTime.setTime(startTime);
        nextScanTime = nextScanTime.addDays(-1);
        while(nextScanTime < QDateTime::currentDateTime())
        {
            nextScanTime = nextScanTime.addSecs(settings.nextScanMinutes*60 + settings.nextScanHours*3600);
        }
        lblNextScan->setText(tr("Следващ отчет: ") + nextScanTime.toString(Qt::LocalDate) + "  ");
    }
    stopScan=false;
}

void MainWindow::onTimerTick()
{
    if(exitFromMenu) return;

    switch(state)
    {
    case STATE_IDLE:
        if(channelsList.count()==0) break; //ако няма конфигурирани канали нищо не се случва

        //Проверява се дали е дошло време за запис в БД на
        if(QDateTime::currentDateTime() >=  nextScanTime)
        {//Ако е, то се прочитат датчиците и се записват в БД с еднакъв дата/час
            while(nextScanTime < QDateTime::currentDateTime())
            { nextScanTime = nextScanTime.addSecs(settings.nextScanMinutes*60 + settings.nextScanHours*3600); }

            retry = settings.maxFailRetry;
            if(retry<1) retry=1;
            if(retry>15) retry=15;
            doReading=true;
        }

        if(readNow)
        {
            //Етикета се скрива и се показва прогресбара.
            lblReadWarning->setVisible(false);
            pbrReadingSensors->setVisible(true);
            pbrReadingSensors->setValue(0);
            pbrReadingSensors->setMaximum(activeNodeList.count() * 3 + 1);

            readNow=false;
            retry = settings.maxFailRetry;
            if(retry<1) retry=1;
            if(retry>15) retry=15;
            doReading=true;
        }

        //При прочитане на температура този флаг става true.
        //По него се установява дали всички датчици са прочетени.
        for(int i=0 ; i!=channelsList.count() ; i++) channelsList[i].setUpdateFlag(false);

        //Подготовка за следващо прочитане
        reader.setNodeList(activeNodeList);
        reader.stopReading=false;
        if(settings.commType) reader.setSerialPort(QSerialPort::Baud1200, settings.serialPortName); //RS-485
        else reader.setSerialPort(QSerialPort::Baud115200, settings.serialPortName); //RS-232

        txrxThread.start();
        state = STATE_WAITING_DATA;
        break;
    case STATE_WAITING_DATA:
        break;
    case STATE_DATA_READY:
        //Има върнати данни за температура в reader.temperatures. channelList се актуализира.
        for(int i=0 ; i!=channelsList.count() ; i++)
        {
            QString addr = QString::number(channelsList[i].node()) + " " + channelsList[i].address();
            if(reader.temperatures.contains(addr))
            {
                channelsList[i].setTemperature(reader.temperatures[addr]);
                channelsList[i].setUpdateFlag(true); //При прочитане на температура този флаг става true
            }
        }

        //за текущо избрания канал от таблицата се показва температурата
        if(monitorChannelIndex!=-1)
        {
            double dTemp = (double) channelsList[monitorChannelIndex].temperature() / 100;
            ui->lblTemp->setText(QString::number(dTemp));
            gauge->setValue((int)dTemp);
        }

        //проверка дали има данни от всички канали
        //ако всички канали са актуализирани то allReady ще остане true
        bool allReady=true;
        foreach(Channel chn, channelsList)  allReady = allReady && chn.updateFlag();

        //Съобщение за грешка при проблем с комуникацията
        if(allReady)
        {//всички канали са прочетени. Ако е имало съобщение за грешка, то ще се изтрие
            lblComAlarm->setText("");
            lblComAlarm->setVisible(false);
        }
        else
        {
            lblComAlarm->setText(tr("  ГРЕШКА: Проблем с комуникацията:  "));
            foreach(Channel chn, channelsList)
            {
                if(!chn.updateFlag())
                {//Ако канала не е актуализиран, името му се добавя в съобщението за грешка
                    lblComAlarm->setText(lblComAlarm->text() + chn.name() + "  ");
                    continue;
                }
            }
            lblComAlarm->setVisible(true);
            //Показване на прозореца при проблем с комуникацията
            if(settings.openOnFail)
            {
                if(!isVisible())
                {
                    showFullScreen();
                    showMaximized();
                }
            }
        }

        //ако е вдигнат флага за четене, то се проверява дали данните за всички канали са актуални.
        //Ако са, то данните се записват в БД. Ако не са, то retry се намаля с 1.
        //Като стане 0 наличните данни се записват.
        if(doReading)
        {
            if(allReady || (retry==0))
            {//Ако всички канали са актуални или повече няма право на повторен опит за четене
                doReading=false;
                lblNextScan->setText(tr("Следващ отчет: ") + nextScanTime.toString(Qt::LocalDate) + "  ");

                //Запис в БД
                QDateTime dt = QDateTime::currentDateTime();
                QSqlQuery qry;
                QString str;
                QString qryStr = QString("INSERT INTO tableLog VALUES ('%1 %2' , ")
                                 .arg(dt.date().toString("yyyy-MM-dd")) //Timestamp //1
                                 .arg(dt.time().toString("hh:mm:ss"));            //2

                foreach(Channel chn, channelsList)
                {
                    if(!chn.updateFlag()) continue;
                    str = qryStr + QString::number(chn.id()) + "," + QString::number(chn.temperature()) + ");";
                    qry.prepare(str);
                    qry.exec();
                }                
                refreshData();//Опреснява се таблицата с данните:
                pbrReadingSensors->setVisible(false);//и се скрива прогресбара
            } else retry--;
        }

        state = STATE_IDLE;
        break;
    }

    timer.start(100);
}

void MainWindow::onTableDataSelect(int row, int col, int prow, int pcol)
{
    bool ok;
    if((col<1) || (col>channelsList.count())) return;
    ui->groupBoxCurrentChannel->setTitle(channelsList[col-1].name());

    monitorChannelIndex = col-1;
    double dTemp = (double) channelsList[col-1].temperature() / 100;
    ui->lblTemp->setText(QString::number(dTemp));
    gauge->setValue((int)dTemp);

    //Опресняване на диаграмата - данните се извличат от таблицата
    xData.clear();
    yData.clear();
    for( int i=0 ; i<ui->tableWidgetData->rowCount() ; i++)
    {
        if(ui->tableWidgetData->item(i,col) == NULL) continue;
        dTemp = ui->tableWidgetData->item(i,col)->text().toDouble(&ok);
        if(ok)
        {
            xData.append((double)i);
            yData.append(dTemp);
        }
    }
    curveA->setSamples(xData.toVector(), yData.toVector());
}

void MainWindow::onActionQuit()
{
    stopScan = true;
    exitFromMenu = true;

    //Запис във файл на ширините на колоните
    QFile file("columns.txt");
    file.open(QIODevice::WriteOnly);
    QTextStream s(&file);
    for(int i =0 ; i < ui->tableWidgetData->columnCount() ; i++)
    {
        s << QString::number(ui->tableWidgetData->columnWidth(i)) <<"\r\n";
    }
    file.close();

    txrxThread.wait(3500);
    qDebug() << "onActionQuit";
    this->close();
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::DoubleClick:
        case QSystemTrayIcon::MiddleClick:
            showNormal();
            break;
        default:
            break;
    }

}

void MainWindow::onActionAboutDialog()
{
    stopScan=true;
    DialogAbout dlg(this);
    dlg.exec();
    stopScan=false;
}

void MainWindow::onActionReport()
{//Генериране на справка и печат
    stopScan=true;
    DialogReport dlg(this);
    dlg.exec();
    stopScan=false;
}

void MainWindow::onActionReadNow()
{//Прочитат се сензорите и се създава запис в БД незабавно
    readNow=true;
    lblReadWarning->setVisible(true);
}

void MainWindow::onReadSensorsDone()
{
    int n = reader.temperatures.count();
    reader.closeSerialPort();
    qDebug() << n << "sensor red.";
    state = STATE_DATA_READY;
}

void MainWindow::onReadProgress(int progress)
{
    pbrReadingSensors->setValue(progress);
}
