#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include <QDialog>
#include <QTime>

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

    void initValues(QTime t,int h, int m, bool oof, int mf, int ct, int mnn, QString sp);
protected:
    void changeEvent(QEvent *e);

private:
    Ui::DialogSettings *ui;

private slots:
    void onMinutesChanged(int m);
    void onHoursChanged(int h);
    void onOK();
};

#endif // DIALOGSETTINGS_H
