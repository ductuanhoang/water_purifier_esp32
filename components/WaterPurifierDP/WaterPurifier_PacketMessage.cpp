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
}

void water_purifier_packet_data_value(unsigned char dpid, const unsigned char value[])
{
    ESP_LOGI(TAG, "dpid = %d --- value = %d", dpid, value[0]);
}
/***********************************************************************************************************************
 * static functions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * End of file
 ***********************************************************************************************************************/