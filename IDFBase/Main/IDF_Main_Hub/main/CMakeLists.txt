idf_component_register(SRCS "main.cpp"
                            "src/Wireless/WiFi_Hub/WiFiHub.cpp"
                            "src/Wireless/WiFi_Hub/Websocket.cpp"
                            "src/Wireless/Telemetry/TelemetryHandler.cpp"
                            "src/Wireless/Telemetry/EC20Handler.cpp"
                            "src/Wired/GPIOHandler.cpp"
                            "src/Wired/CANHandler.cpp"
                    INCLUDE_DIRS ".")
