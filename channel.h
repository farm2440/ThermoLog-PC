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
    void setOffset(double ofs);
    void setRatio(double rto);

    int id();
    QString name();
    int node();
    QString address();
    bool active();
    int temperature();
    bool updateFlag();
    double offset();
    double ratio();

private:
    int _id;
    QString _name; //име зададено от потребителя
    int _node;       //Номер на контролера в RS-485
    QString _address;//MAC адрес на сензора
    bool _active;
    int _temperature;
    bool _updateFlag;
    double _offset;
    double _ratio;
};

#endif // CHANNEL_H
