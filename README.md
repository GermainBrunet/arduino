# Arduino as a Sensor and Control Platform

This is a simple project that uses the [Arduino Nano](https://www.arduino.cc/en/Main/ArduinoBoardNano) as a sensor and control platform.  By combining multiple sensors together one can gather data over time on ambient temperature, water or ground temperature, relative temperature, humidity, pressure, light (lumens).  With this information, it is possible to control different devices (heater, fans, lights) and optimize the environment for an optimum growth.

The information gathered by the "Nano" is then communicated to a computer (targetting the Raspberry Pi II model B) that run a Spring Boot web application that captures and display information to mobile devices.  The intent is to allow one or more "Nano" devices to communicate data over USB (for short distances) or the network (for longer distances).  This project is intended as the author's playground as he experiments with aquaponics.

Due to the lack of inexpensive options, this project looks to provide as much capabilities while minimizing costs.

###Bill of Material
- Arduino Nano $4 - [ebay](http://www.ebay.ca/itm/MINI-USB-Nano-V3-0-ATmega328P-CH340G-5V-16M-Micro-controller-board-Arduino-T1-/181846906547?hash=item2a56eb96b3:g:Ir8AAOSwBahVL6BH)
- Pressure Sensor BMP183 - $10 - [Adafruit](https://www.adafruit.com/products/1900)
- Light Sensor TLS2561 $6- [Adafruit](https://www.adafruit.com/products/439)
- Humidity Sensor AM2302 $15 [Adafruit](https://www.adafruit.com/products/393)
- Thermister 3950NTC $10 [Adafruit](https://www.adafruit.com/products/372)
- 470 ohm resistor $1 for 50 ebay
- Red LED $3 for 50 on ebay
- Mini USB Cable $3 on ebay

Total cost of hardware $52 + shipping charges.  Price could be reduced through bulk buying or by picking alternative parts on [ebay](www.ebay.com) or [DHgate](www.dhgate.com).  Parts were available at the time of this article.

###Communication
The arduino communicates to the server using json.  This allows for flexibility in defining different sensors and switches.

> {"nodeUID":"A199827", "measurements":[{"sensor":"humidity", "value":34.70, "unit":"percent"},{"sensor":"temperature", "value":27.50, "unit":"celcius"},{"sensor":"heat index", "value":26.96, "unit":"celcius"},{"sensor":"temperature probe", "value":26.46, "unit":"celcius"},{"sensor":"pressure", "value":997.54, "unit":"hPa"},{"sensor":"light", "value":142.00, "unit":"lux"},{"sensor":"temperature from pressure sensor", "value":25.32, "unit":"celcius"},{"switch":"switch1", "value":0, "charOn":"y", "charOff":"z"}]}



