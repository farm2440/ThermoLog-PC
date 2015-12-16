#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include <QDialog>
#include <QTime>
#include <QFileDialog>
#include <QMessageBox>

namespace Ui {
    class DialogSettings;
}

class DialogSettings : public QDialog {
    Q_OBJECT
public:
    DialogSettings(QWidget *parent = 0);
    ~DialogSettings();

    QTime startTime;
    int hours;
    int minutes;
    bool openOnFail;
    int maxFail;
    int commType;
    int maxNodeNum;
    QString serialPort;
    QString dbFileName;

    void initValues(QTime t, int h, int m, bool oof, int mf, int ct, int mnn, QString sp, QString dbf);
protected:
    void changeEvent(QEvent *e);

private:
    Ui::DialogSettings *ui;

private slots:
    void onMinutesChanged(int m);
    void onHoursChanged(int h);
    void onOK();
    void on_btnSelectFileDB_clicked();
};

#endif // DIALOGSETTINGS_H
