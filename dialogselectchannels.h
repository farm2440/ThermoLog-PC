#ifndef DIALOGSELECTCHANNELS_H
#define DIALOGSELECTCHANNELS_H

#include <QDialog>

namespace Ui {
    class DialogSelectChannels;
}

class DialogSelectChannels : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSelectChannels(QWidget *parent = 0);
    ~DialogSelectChannels();

    void setList(QStringList *list);
    void getList(QStringList *list);
    QString getFamNam();

private:
    Ui::DialogSelectChannels *ui;
private slots:
    void onOK();
    void onCancel();
};

#endif // DIALOGSELECTCHANNELS_H
