set(srcs "main.cpp"
        "src/NimBLE/nimble.c"
        "src/WiFiClient/WiFiAP.c"
        "src/WSClient/WSClient.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ".")
