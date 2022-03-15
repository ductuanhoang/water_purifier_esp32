#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "softuart.h"
#include "app_smart_config.h"



void app_main(void)
{
    app_smart_config_init();
    uint8_t ret = 0;
    int i = 0;
    while (1)
    {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
