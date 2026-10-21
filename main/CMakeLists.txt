idf_component_register(
    SRCS "main.c"
         "passport_nav.c"
         "passport_art.c"
         "passport_ui.c"
    INCLUDE_DIRS "."
    REQUIRES bsp bt esp_event esp_hw_support esp_timer esp_netif nvs_flash
)
