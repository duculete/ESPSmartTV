# ESPSmartTV - control your TV via IR using MQTT 
...(and apple HomeKIT: https://github.com/duculete/homebridge-mq-tv )

## Hardware
The Wemos ESP8266 board containes 3 elements:
- IR reveiver
- IR Transmiter
- Contact sensor

## How it works
The Boards listens for commands on MQTT topic and based on that command will send IR code. Also using Contact sensor (**TV_STATUS_PIN**) it will read if TV is on or OFF and update the coresponding MQTT Topic. This feature is usefull if the TV has a USB port that sends power only when TV is ON (5V). 

## Extra features:
Since this project is created to work with Apple Homekit via Homebridge plugin some extra features are added:

- receives and handler Apple HomeKit TV/STB remore keys (UP, DOWN, MENU, VOLUME_UP, etc ...)
- has predefined few IR codes for IR Remote control and handles different "macro" action: switch to channel 32, switch to chanel 123, switch to HDMI (the logic depends on TV model and manufacturer).