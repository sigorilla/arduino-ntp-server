# Arduino NTP Server

Проект по курсу [Защита информации МФТИ](https://ru.wikipedia.org/wiki/Проект:Защита_информации/2015). Реализация [NTP](https://ru.wikipedia.org/wiki/NTP) сервера на Arduino, используя в качестве эталона времени — часы со спутника.

## Плата

Arduino Leonardo ETH

## Модули

* GPS — `GY-NEO6MV2`

## Конфигурация

### GPS
```
GPS <-> Arduino
-----------------
GND --> GND
Tx  --> PIN8 (Rx)
Rx  --> PIN9 (Tx)
VCC --> +3.3V
```

## Отладка

Для отладки создан скетч `UCenterConfig`, который перенапрявляет все сообщения из стандартного Serial в UART GPS модуля. Таким образом, можно настроить GPS модуль, используя программу u-center от u-blox.

## Ссылки

* [u-center](https://www.u-blox.com/en/product/u-center-windows)
* [Instructables about u-blox](http://www.instructables.com/id/Arduino-Ublox-GPS/)
* [TinyGPS++](https://github.com/mikalhart/TinyGPSPlus)

## Участники

* Игорь Степанов — 213 группа
* Павел Лысенко — 216 группа
