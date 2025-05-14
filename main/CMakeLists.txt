idf_component_register(
    SRCS
    "main.c"
    "app.c"
    REQUIRES
    nvs_flash
    touch_element
    esp_adc
    esp_timer

    INCLUDE_DIRS
    "."
)
