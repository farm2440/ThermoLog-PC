Qt5.2
commit 29MAY2014
Версия  "Thermolog 1.3" - Стабилна.  
Четенето на контролерите става с SensorReader в отделна нишка.


v2 - firmware-v3-compatible branch 

v2.0.1

2.0.1 -
- Преминава към ползване на целия МАС с 16 символа
- В БД tableChannels са добавени колони REAL - Offset и Ratio
dialogchannelsettings  е преработен да приема и стойности от/за новите
колони

2.0.2 -
- В таблицата tableLog името на колоната Temp е сменено на Value и типа е променен от INT на REAL

14DEC2015
Имах проблем - при преминаване към release програмата отказваше да се стартира. Проблема идваше от библиотеката за графиката Qwt.
След прекомпилирането и и промени в ThermoLog.pro файла проблема отпадна. Във версия 1.3 qwt е 6.0.1. От версия 2 ползвам qwt 6.1.2

2.0.3 -
15DEC2015

Премахат е проблема с изпускането на приети от сериен порт данни.
Когато сериен порт се ползва в нишка различна от главната трябва да се ползва с указател и да се създаде с new в неговата си нишка.
Иначе неговия родител става главната нишка а не нишшката в която работи порта. Когато има проблем при първия опит за ползване на порта
излиза дебъг съобщение
QObject: Cannot create children for a parent that is in a different thread.
(Parent is QSerialPort(0x28fd90), parent's thread is QThread(0x176ae230), current thread is QThread(0x28fd78)

След като порта се създава в неговата си нишка спря прииемането на данни. За да приема преди да се вика sp->bytesAvailable() трябва
да се извика sp->waitForReadyRead(1000);

2.0.4 -
16DEC2015
Стабилна

2.0.5
17DEC2015
Добавена е възможност да се избира файла с БД от главното меню "Настройки-Работни настройки"
Ако потребителя избере файл, то пълния път се записва във файла settings.txt. 
При стартиране ако settings.txt съществува то името на файла с БД се зарежда от там.
Ако не, то се ползва thermolog.db който трябва да е в същата директория като ThermoLog.exe