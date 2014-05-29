#ifndef CHANNEL_H
#define CHANNEL_H

#include <QString>

class Channel
{
public:
    Channel();
    void setName(QString nm);
    void setID(int i);
    void setAddress(QString mac);
    void setTemperature(int temp);
    void setUpdateFlag(bool flag);
    void setNode(int n);
    void setActive(bool a);

    int id();
    QString name();
    int node();
    QString address();
    bool active();
    int temperature();
    bool updateFlag();

private:
    int _id;
    QString _name; //име зададено от потребителя
    int _node;       //Номер на контролера в RS-485
    QString _address;//MAC адрес на сензора
    bool _active;
    int _temperature;
    bool _updateFlag;
};

#endif // CHANNEL_H
