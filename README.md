# iRDash_Client v2
Displays live data of iRacing telemetry using an Arduino board plus a TFT display.

The purpose of this program is to display iRacing's live telemetry data on an Arduino TFT display.
As of now it can show:
- RPM
- Eight segment Shift Light Indicator
- Gear
- Fuel quantity
- Water temperature
- Speed
- Engine management lights

It supports multiple profiles to suit the different cars available in the sim. You can choose between them if you touch the middle 1/3 of the display.
Currently it has profile for:
- Skip Barber (Skippy)
- Cadillac CTS-V
- MX-5 (NC and ND)
- Formula Renault 2.0
- Dallara Formula 3
- Porsche 911 GT3 cup (992) // updated for 2022 season 4
- Toyota GR86
- Super Formula Lights
- BMW G82 M4

The program is developed on a Geekcreit ESP32 dev board and a 320x240 resolution touch sensitive SPI serial TFT display.
Project is moved to Arduino GFX library from Adafruit GFX library. It has much better performance.

To display the live data of iRacing it needs the "iRDash Server" program running on Windows host and connected to the Arduino board via USB.
- https://github.com/UmmonPwr/iRDash-Server

To compile the program you need the below libraries:
- Arduino GFX library https://github.com/moononournation/Arduino_GFX
- Touch sense library https://github.com/PaulStoffregen/XPT2046_Touchscreen