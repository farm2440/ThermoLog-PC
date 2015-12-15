#include "dialogreport.h"
#include "ui_dialogreport.h"

DialogReport::DialogReport(QWidget *parent) : QDialog(parent), ui(new Ui::DialogReport)
{
    ui->setupUi(this);

    connect(ui->btnExit,SIGNAL(clicked()),this,SLOT(onExit()));

    connect(ui->calendarWidgetEnd,SIGNAL(selectionChanged()),this,SLOT(onSelectEndDate()));
    connect(ui->calendarWidgetStart,SIGNAL(selectionChanged()),this,SLOT(onSelectStartDate()));

    ui->lblStart->setText(tr("От:") + ui->calendarWidgetStart->selectedDate().toString("dd-MM-yyyy"));
    ui->lblEnd->setText(tr("До:") + ui->calendarWidgetEnd->selectedDate().toString("dd-MM-yyyy"));

    //Printing
    printer = new QPrinter(QPrinter::HighResolution);
    preview = new QPrintPreviewDialog(printer,this);
    connect(preview,SIGNAL(paintRequested(QPrinter*)),this,SLOT(print(QPrinter*)));
    connect(ui->btnPrintPreview,SIGNAL(clicked()),this,SLOT(onPrintPreview()));
}

DialogReport::~DialogReport()
{
    delete ui;
    delete printer;
    delete preview;
}

void DialogReport::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void DialogReport::onExit()
{
    this->accept();
}

void DialogReport::onSelectStartDate()
{
    ui->lblStart->setText(tr("От:") + ui->calendarWidgetStart->selectedDate().toString("dd-MM-yyyy"));
    ui->calendarWidgetEnd->setMinimumDate(ui->calendarWidgetStart->selectedDate());;
}

void DialogReport::onSelectEndDate()
{
    ui->lblEnd->setText(tr("До:") + ui->calendarWidgetEnd->selectedDate().toString("dd-MM-yyyy"));
    ui->calendarWidgetStart->setMaximumDate(ui->calendarWidgetEnd->selectedDate());
}

void DialogReport::onPrintPreview()
{

    QSqlQuery qrySites;
    QString strQrySites;
    DialogSelectChannels dlg;


    //Първо се извежда диалог за избор на каналите които да бъдат включени в протокола
    strQrySites = "SELECT * from tableChannels;";
    qrySites.prepare(strQrySites);
    if(!qrySites.exec())
    {
         QMessageBox::critical(this, tr("Грешка база данни"), tr("Не мога да изпълня SQL заявка SELECT заtableChannels"));
         return;
    }

    int id;
    bool ok;
    double offset, ratio, fam;
    QString MAC;
    reportChannelsList.clear();
    while(qrySites.next())
    {
        id = qrySites.value(0).toInt(&ok);
        reportChannelsList.append(qrySites.value(1).toString());
        hashNameID.insert(qrySites.value(1).toString(), id);

        offset = qrySites.value(5).toDouble();
        ratio = qrySites.value(6).toDouble();
        hashIdOffset.insert(id,offset);
        hashIdRatio.insert(id, ratio);

        MAC = qrySites.value(3).toString();
        fam=0;
        if(MAC.left(2)=="10") fam=10;
        if(MAC.left(2)=="28") fam=28;
        if(MAC.left(2)=="26") fam=26;
        hashIdFamCode.insert(id,fam);
    }
    //Избор на сайтовете за протокола
    dlg.setList(&reportChannelsList);
    if(dlg.exec()== QDialog::Rejected) return;
    dlg.getList(&reportChannelsList);
    //в hashIdColumn се запазва връзката кои ID на канал в коя колона в таблицата отива
    hashIdColumn.clear();
    for(int i=0 ; i != reportChannelsList.count() ; i++)
    {
        id = hashNameID[reportChannelsList[i]];
        hashIdColumn.insert(id,i);
    }

    famNam=dlg.getFamNam();
    if(famNam=="") famNam="_________________________________________";

    //Превю диалог:
    preview->setWindowState(Qt::WindowMaximized);
    preview->exec();
    this->accept();

}

void DialogReport::print(QPrinter *prn)
{//Този метод е слот който е свързан със сигнала paintRequested на preview (QPrintPreviewDialog)
    QTextDocument doc;    
    QTextCursor cursor(&doc);
    //------------------------------------------------
    //Извличане на данните и поставяне в таблици
    QSqlQuery  qryData;
    QString  strQryData;
    //------------------------------------------------
    // Заглавна част на протокола
    QTextCharFormat chrFormat18B, chrFormat10, chrFormatGrayBkg;
    QTextBlockFormat blkFormatCenter, blkFormatLeft, blkFormatRight;
    QFont fnt;
    //Подготовка на формати за шрифтове
    chrFormat18B = cursor.charFormat();
    fnt = chrFormat18B.font();
    fnt.setBold(true);
    fnt.setPointSize(18);
    chrFormat18B.setFont(fnt);

    chrFormat10 = cursor.charFormat();
    fnt = chrFormat10.font();
    fnt.setBold(false);
    fnt.setPointSize(10);
    chrFormat10.setFont(fnt);
    //Формати за блокове
    blkFormatCenter.setAlignment(Qt::AlignHCenter);
    blkFormatLeft.setAlignment(Qt::AlignLeft);
    blkFormatRight.setAlignment(Qt::AlignRight);
    //Tekst na protokola
    cursor.insertBlock(blkFormatCenter,chrFormat18B);
    cursor.insertText(tr("ПРОТОКОЛ\n"));

    cursor.setPosition(QTextCursor::End);
    cursor.insertBlock(blkFormatLeft,chrFormat10);
    cursor.insertText(tr("За отчетените стойности на температура за канали: \n"));
    for(int i = 0 ; i< reportChannelsList.count() ; i++)
    {
        cursor.insertText(reportChannelsList[i]);
        if(i== reportChannelsList.count()-1) cursor.insertText(".");
        else cursor.insertText(", ");
    }
    QString startDate,endDate;
    startDate = ui->calendarWidgetStart->selectedDate().toString("dd-MM-yyyy");
    endDate = ui->calendarWidgetEnd->selectedDate().toString("dd-MM-yyyy");
    if(startDate==endDate) cursor.insertText(tr("\nза ") + startDate + tr("г."));
    else cursor.insertText(QString(tr("\nза периода от %1г. до %2г.")).arg(startDate).arg(endDate));
    cursor.insertText(tr("\n\nДата:") + QDate::currentDate().toString("dd-MM-yyyy") + tr("г.\n\n"));
    cursor.insertText(QString(tr("Изготвил:  %1\n\nПодпис:____________________\n\n")).arg(famNam));

    QTextTable *pTable;
    QTextTableFormat tabFormat;
    QTextCursor tabCursor;
    int columns = 2 + reportChannelsList.count(); //Броят колони е броя канали за справка + 1 за дата/час + 1 за номер на отчет
    int rows = 3;
    //Вмъква се таблицата за данните
    tabFormat.setCellPadding(2); //отстояние от съдържанието до стените на клетката
    tabFormat.setCellSpacing(0); //отстояние между клетките
    tabFormat.setHeaderRowCount(2); //Ако таблицата се пренася в нова страница то хедъра се отпечатва отново
    tabFormat.setBorderStyle(QTextTableFormat::BorderStyle_Solid);
    pTable = cursor.insertTable(rows,columns,tabFormat);
    //Фонът е сив
    chrFormatGrayBkg.setBackground(QBrush(Qt::lightGray));
    for(int i=0 ; i != columns ; i++)
    {
        pTable->cellAt(1,i).setFormat(chrFormatGrayBkg);
        pTable->cellAt(0,i).setFormat(chrFormatGrayBkg);
    }
    //Първия ред е име на програмата. Клетките на таблицата са слети
    pTable->mergeCells(0,0,1,columns);
    tabCursor = pTable->cellAt(0,0).firstCursorPosition();
    tabCursor.insertText(STRING_SOFT_VERSION);
    tabCursor.insertText("\t\twww.stiv.bg");
    //Вторият ред е с имената на колоните.
    tabCursor = pTable->cellAt(1,1).firstCursorPosition();
    tabCursor.insertText(tr("Дата/час"));
    for(int i=0 ; i != reportChannelsList.count() ; i++)
    {
        tabCursor = pTable->cellAt(1,i+2).firstCursorPosition();
        tabCursor.insertText(reportChannelsList[i]);
        //Третия ред е за мерни единици - ако MAC почва 10,28 -C, 26 -%
        int id = hashNameID[reportChannelsList[i]];
        tabCursor = pTable->cellAt(2,i+2).firstCursorPosition();
        QString unit;
        if((hashIdFamCode[id] == 10)||(hashIdFamCode[id] == 28)) unit="[ºC]";
        if(hashIdFamCode[id] == 26) unit="[%]";
        tabCursor.insertText(unit);
    }



    //Извличане на данните от БД и нанасяне в таблицата
    //Извличане на данните от БД и запис в таблицата
    strQryData = QString("SELECT * FROM tableLog WHERE Timestamp>='%1' AND Timestamp<='%2' ORDER BY Timestamp DESC;")
                    .arg(ui->calendarWidgetStart->selectedDate().toString("yyyy-MM-dd") + " 00:00:00")
                    .arg(ui->calendarWidgetEnd->selectedDate().toString("yyyy-MM-dd") + " 24:00:00");
    qryData.prepare(strQryData);

    if(!qryData.exec())
    {    QMessageBox::critical(this, tr("Грешка база данни"), tr("Не мога да изпълня SQL заявка SELECT"));
         return;
    }

    QString timestamp,pTimestamp="";
    int nTemp,id;
    double dTemp, offset, ratio;
    bool ok;
    int n=1;
    while(qryData.next())
    {
        timestamp = qryData.value(0).toString();
        if(pTimestamp != timestamp)
        {//Ако Това е нова стойност за дата час. Добавя се нов ред а датата/час се слага в първата колона.
            pTimestamp=timestamp;
            pTable->insertRows(rows,1);
            tabCursor = pTable->cellAt(rows,0).firstCursorPosition();
            tabCursor.insertText(QString::number(n++) + "  ");
            tabCursor.movePosition(QTextCursor::NextCell);
            tabCursor.insertText(timestamp + "  ");
            rows++;
        }
        //извличаме температурата и я слагаме в колоната за съответния канал
        id = qryData.value(1).toInt(&ok);
        if(!hashIdColumn.contains(id)) continue;

        offset = hashIdOffset[id];
        ratio = hashIdRatio[id];
        nTemp = qryData.value(2).toInt(&ok);
        dTemp = (double)nTemp/100;
        dTemp *= ratio;
        dTemp += offset;
        tabCursor = pTable->cellAt(rows-1, hashIdColumn[id]+2).firstCursorPosition();
        tabCursor.insertText(QString::number(dTemp,'f',1));
    }

    //Край на извличането на данните и на поставянето им в таблици
    //--------------------------------------------------------------
    doc.print(printer);

}

