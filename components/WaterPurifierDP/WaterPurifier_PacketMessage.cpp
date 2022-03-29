/**
 * @file WaterPurifier_PacketMessage.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-03-26
 *
 * @copyright Copyright (c) 2022
 *
 */

/***********************************************************************************************************************
 * Pragma directive
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes <System Includes>
 ***********************************************************************************************************************/
#include "WaterPurifier_PacketMessage.h"
/* sprintf example */
#include <stdio.h>
#include "esp_log.h"
/***********************************************************************************************************************
 * Macro definitions
 ***********************************************************************************************************************/

#define TAG "water_purifier_packet"
/***********************************************************************************************************************
 * Typedef definitions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Private global variables and functions
 ***********************************************************************************************************************/
typedef unsigned char (*water_purifier_report_dpi_t)(const char *data_report);
water_purifier_report_dpi_t water_purifier_report_dpi = NULL;
/***********************************************************************************************************************
 * Exported global variables and functions (to be accessed by other files)
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Imported global variables and functions (from other files)
 ***********************************************************************************************************************/

// void water_purifier_report_callback_register(water_purifier_report_dpi _cb)
// {
//     water_purifier_report_dpi = _cb;
// }
/**
 * @brief
 *
 * @param dpid
 * @param value
 */
void water_purifier_packet_data_bool(unsigned char dpid, unsigned short value)
{
    // "1": ,
    ESP_LOGI(TAG, "dpid = %d --- value = %d", dpid, value);
    char buffer_report[20];
    if (value)
        sprintf(buffer_report, "{\"%d\": %s}", dpid, "true");
    else
        sprintf(buffer_report, "{\"%d\": %s}", dpid, "false");

    // water_purifier_report_dpi(buffer_report);
    ESP_LOGI(TAG, "send data = %s", buffer_report);
    if (water_purifier_report_dpi != NULL)
        water_purifier_report_dpi(buffer_report);
    else
        ESP_LOGI(TAG, "not registering function");
}

void water_purifier_packet_data_enum(unsigned char dpid, unsigned short value)
{
    ESP_LOGI(TAG, "dpid = %d --- value = %d", dpid, value);
    char buffer_report[20];
    sprintf(buffer_report, "{\"%d\": %d}", dpid, value);
    ESP_LOGI(TAG, "send data = %s", buffer_report);
}

void water_purifier_packet_data_value(unsigned char dpid, const unsigned char value[])
{
    uint32_t _value = 0;
    _value = (value[0] << 24) + (value[1] << 16) + (value[2] << 8) + value[3];
    char buffer_report[20];
    sprintf(buffer_report, "{\"%d\": %d}", dpid, _value);
    ESP_LOGI(TAG, "send data = %s", buffer_report);
    // ESP_LOGI(TAG, "dpid = %d --- value = %d", dpid, value[0]);
    // ESP_LOGI(TAG, "dpid = %d --- value = %d", dpid, value[1]);
    // ESP_LOGI(TAG, "dpid = %d --- value = %d", dpid, value[2]);
    // ESP_LOGI(TAG, "dpid = %d --- value = %d", dpid, value[3]);
    // send_len = tuya_uart.set_wifi_uart_byte(send_len, value >> 24);
    // send_len = tuya_uart.set_wifi_uart_byte(send_len, value >> 16);
    // send_len = tuya_uart.set_wifi_uart_byte(send_len, value >> 8);
    // send_len = tuya_uart.set_wifi_uart_byte(send_len, value & 0xff);
}
/***********************************************************************************************************************
 * static functions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * End of file
 ***********************************************************************************************************************/