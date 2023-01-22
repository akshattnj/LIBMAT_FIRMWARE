set(srcs "main.cpp"
        "src/nimble.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ".")
