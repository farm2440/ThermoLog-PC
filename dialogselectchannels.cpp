#include "dialogselectchannels.h"
#include "ui_dialogselectchannels.h"

DialogSelectChannels::DialogSelectChannels(QWidget *parent) :    QDialog(parent),    ui(new Ui::DialogSelectChannels)
{
    ui->setupUi(this);

    connect(ui->btnOK,SIGNAL(clicked()),this,SLOT(onOK()));
    connect(ui->btnCancel,SIGNAL(clicked()),this,SLOT(onCancel()));
}

DialogSelectChannels::~DialogSelectChannels()
{
    delete ui;
}

void DialogSelectChannels::onOK()
{
    this->accept();
}

void DialogSelectChannels::onCancel()
{
    this->reject();
}

void DialogSelectChannels::setList(QStringList *list)
{//Зарежда списъка в листа
    foreach(QString site, *list)
    {
        QListWidgetItem *itm;
        itm = new QListWidgetItem(site);
        itm->setCheckState(Qt::Unchecked);
        ui->listWidget->addItem(itm);
    }
}

void DialogSelectChannels::getList(QStringList *list)
{
    int i;
    list->clear();
    for(i=0 ; i<ui->listWidget->count() ; i++)
    {
        if(ui->listWidget->item(i)->checkState()) list->append(ui->listWidget->item(i)->text());
    }

}

QString DialogSelectChannels::getFamNam()
{
    return ui->lineEditFamNam->text();
}
