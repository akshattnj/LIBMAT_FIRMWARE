# _LiBMAT Project Battery Firmware_

## Brief Description of the project
LiBMAT stands for Lithium Battery Management And Tracking. The objective of this project is to track and manage battries using IoT. This specific project contains the firmware that is uploaded to the individual battries in this project.


Below is short explanation of the files in the project folder.

```
├── CMakeLists.txt
├── components
│   └─ esp-nimble-cpp              BLE CPP library
├── Makefile
├── sdkconfig
├── partitions.csv
├── main
│   ├── CMakeLists.txt
│   ├── src
│   │   ├── Commons                Contains various variables that are used across files
│   │   │  ├── Commons.h
│   │   │  └── Commons.cpp
│   │   ├── Networking             This contains all wireless networking related things
│   │   │   ├── BLE
│   │   │   │   ├── BLEHandler.cpp Implementation file for BLE
│   │   │   │   └── BLEHandler.h   Header file for BLE
│   │   │   ├── MQTT
│   │   │   │   ├── MQTT.cpp       Implementation file for MQTT
│   │   │   │   └── MQTT.h         Header file for MQTT
│   │   │   ├── WiFi
│   │   │   │   ├── WiFi.cpp       Implementation file for WiFi
│   │   │   │   └── WiFi.h         Header file for WiFi
│   │   │   └── WS
│   │   │       ├── WS.cpp         Implementation file for Websockets
│   │   │       └── WS.h           Header file for Web Sockets
│   │   ├── CANHandler.cpp         Implementation file for controlling the CAN Bus
│   │   ├── CANHandler.h           Header file for controlling the CAN Bus
│   │   ├── LEDHandler.cpp         Implementation file for controlling the WS2812 LED Strips
│   │   ├── LEDHandler.h           Header file for controlling the WS2812 LED Strips
│   │   └── definations.h          Contains constants that are used across files
│   └── main.cpp
└── README.md                      This is the file you are currently reading
```