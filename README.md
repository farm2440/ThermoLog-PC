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