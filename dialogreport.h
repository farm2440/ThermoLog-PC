#ifndef DIALOGREPORT_H
#define DIALOGREPORT_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QStringList>
#include <QListWidgetItem>
#include <QHash>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrintPreviewDialog>

#include <QTextDocument>
#include <QTextTable>
#include <QTextTableFormat>
#include <QTextTableCellFormat>
#include <QTextCursor>
#include <QTextCharFormat>

#include "dialogselectchannels.h"
#include "definitions.h"

namespace Ui {
    class DialogReport;
}

class DialogReport : public QDialog {
    Q_OBJECT
public:
    DialogReport(QWidget *parent = 0);
    ~DialogReport();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::DialogReport *ui;

    QPrinter *printer;
    QPrintPreviewDialog *preview;
    QStringList reportChannelsList; //В този списък се зареждат имената на станциите които да бъдат включени в протокола
    QHash<int, int> hashIdColumn; //Тук се пазят двойките id/номер на колона в таблицата
    QString famNam; //Име на изготвилия протокола

private slots:
    void onExit();

    void onSelectStartDate();
    void onSelectEndDate();

    void onPrintPreview();
    void print(QPrinter *prn);
};

#endif // DIALOGREPORT_H
