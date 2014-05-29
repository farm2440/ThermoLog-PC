#include "dialogsettings.h"
#include "ui_dialogsettings.h"

DialogSettings::DialogSettings(QWidget *parent) : QDialog(parent),    ui(new Ui::DialogSettings)
{
    ui->setupUi(this);

    connect(ui->spinHours,SIGNAL(valueChanged(int)),this,SLOT(onHoursChanged(int)));
    connect(ui->spinMinutes,SIGNAL(valueChanged(int)),this,SLOT(onMinutesChanged(int)));

    connect(ui->btnCancel,SIGNAL(clicked()),this,SLOT(reject()));
    connect(ui->btnOK,SIGNAL(clicked()),this,SLOT(onOK()));
}

DialogSettings::~DialogSettings()
{
    delete ui;
}

void DialogSettings::changeEvent(QEvent *e)
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

void DialogSettings::onMinutesChanged(int m)
{
    if(ui->spinHours->value()==0)
    {
        if(m < 15) ui->spinMinutes->setValue(15);
    }
    if(ui->spinMinutes->value()==60)
    {
        ui->spinHours->setValue(ui->spinHours->value()+1);
        ui->spinMinutes->setValue(0);
    }
}

void DialogSettings::onHoursChanged(int h)
{
    if(h==0)
    {
        if(ui->spinMinutes->value() < 15) ui->spinMinutes->setValue(15);
    }
}

void DialogSettings::onOK()
{
    startTime = ui->timeEdit->time();
    hours = ui->spinHours->value();
    minutes = ui->spinMinutes->value();
    openOnFail = ui->chkOpenOnFail->isChecked();
    maxFail=ui->spinMaxRetry->value();
    if(ui->rbRS485->isChecked()) commType=1;
    else commType=0;
    maxNodeNum = ui->spbMaxNodeNumber->value();
    serialPort = ui->lineEdit_SerialPort->text();
    this->accept();
}

void DialogSettings::initValues(QTime t, int h, int m, bool oof, int mf, int ct, int mnn, QString sp)
{
    ui->spinHours->setValue(h);
    ui->spinMinutes->setValue(m);
    ui->timeEdit->setTime(t);
    ui->chkOpenOnFail->setChecked(oof);
    ui->spinMaxRetry->setValue(mf);
    if(ct) ui->rbRS485->setChecked(true);
    else ui->rbRS232->setChecked(true);
    ui->spbMaxNodeNumber->setValue(mnn);
    ui->lineEdit_SerialPort->setText(sp);
}
