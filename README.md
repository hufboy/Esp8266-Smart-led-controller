
# Esp8266 Local web server for controlling smart stuff

Made this project to control the leds and the temperature in my room.

Hardware: 
- Nodemcu 1.0
- LED Strip 
- AM2320 Temp & Humidity Sens
- HC-S501 Motion Sensor
- IR transmitter / receiver

On power on it will run local web server on wifi (change wif SSID/password and ip conf).
Via website you can choose to turn on /off leds, select fastled predefied palette  and change color. 

There is motion sensor attached on it so on the webpage you can check for power on leds via mot.sens 

View temp & Humidity on the other tab.

Esp8266 will use LittleFS to read web page file so before uploading code make sure to build LittleFS image and upload image too. 




## Installation

Build using PlatformIO on VSCODE

    
## Roadmap

- Improve web request manage function 

- Improve Motion sensor 

- Add IR recv/send funcs (to control IR devices)

- General improvements