#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "softuart.h"
#include "app_smart_config.h"
#include "user_uart.h"

#define TAG "main.c"
void main_recieve_callback(uint8_t *buf, uint8_t len)
{
    ESP_LOGI(TAG, "recieve = %s\r\n", buf);
}

void app_main(void)
{
    app_smart_config_init();
    uart_recieve_callback_init(main_recieve_callback);
    user_uart_init();
    uint8_t ret = 0;
    int i = 0;
    while (1)
    {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
