#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
extern "C"
{
#include "softuart.h"
#include "app_smart_config.h"
#include "user_uart.h"
#include "TuyaWifi.h"
#include "WaterPurifierDP.h"
}

#define TAG "main.c"

extern "C"
{
    void app_main(void);
}

static void WaterPurifier_Process_task(void *pvParameters)
{
    while (1)
    {
        WaterPurifier_Process();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // app_smart_config_init();
    // uart_recieve_callback_init(tuya_wifi.uart_service_2);
    WaterPurifier_init();
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    xTaskCreate(WaterPurifier_Process_task, "WaterPurifier_Process_task", 4096, NULL, 10, NULL);
    while (1)
    {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
