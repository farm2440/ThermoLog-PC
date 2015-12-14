#include "channel.h"

Channel::Channel()
{
    _id=-1;
    _node=-1;
    _name="";
    _address="";
    _active=false;
    _updateFlag=false;
    _value=-300;
    _offset = 0.0;
    _ratio = 1.0;
}

void Channel::setName(QString nm) { _name=nm; }
void Channel::setID(int i) { _id=i; }
void Channel::setAddress(QString mac) { _address=mac; }
void Channel::setValue(int val)
{
    _updateFlag=true;
    _value=val;
}
void Channel::setUpdateFlag(bool flag) { _updateFlag=flag; }
void Channel::setNode(int n) { _node=n; }
void Channel::setActive(bool a) { _active=a; }
void Channel::setOffset(double ofs){ _offset=ofs; }
void Channel::setRatio(double rto) { _ratio=rto; }

int Channel::id() { return _id; }
QString Channel::name() { return _name; }
int Channel::node() { return _node; }
QString Channel::address() { return _address; }
bool Channel::active() { return _active; }
int Channel::value() { return _value; }
bool Channel::updateFlag() { return _updateFlag; }
double Channel::offset() { return _offset; }
double Channel::ratio() { return _ratio; }
