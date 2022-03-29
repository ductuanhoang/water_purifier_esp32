/**
 * @file WaterPurifierDP.cpp
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
#include "WaterPurifierDP.h"
#include "WaterPurifier_PacketMessage.h"
#include "TuyaDefs.h"
#include "TuyaWifi.h"
#include "user_uart.h"
#include "TuyaWifi.h"
#include "esp_log.h"
/***********************************************************************************************************************
 * Macro definitions
 ***********************************************************************************************************************/
#define TAG "WaterPurifierDP.cpp"
/***********************************************************************************************************************
 * Typedef definitions
 ***********************************************************************************************************************/
TuyaWifi tuya_wifi;
/* Stores all DPs and their types. PS: array[][0]:dpid, array[][1]:dp type.
 *                                     dp type(TuyaDefs.h) : DP_TYPE_RAW, DP_TYPE_BOOL, DP_TYPE_VALUE, DP_TYPE_STRING, DP_TYPE_ENUM, DP_TYPE_BITMAP
 */
unsigned char dp_array[][2] = {
    {DPID_TDS_OUT, DP_TYPE_VALUE},
    {DPID_PAC_FILTERTIM, DP_TYPE_VALUE},
    {DPID_RO_FILTERTIME, DP_TYPE_VALUE},
    {DPID_CF_FILTERTIME, DP_TYPE_VALUE},
    {DPID_CURRENT_TEMP, DP_TYPE_VALUE},
    {DPID_TOTAL_WATER, DP_TYPE_VALUE},
    {DPID_RESET_WATER, DP_TYPE_BOOL},
    {DPID_SWITCH_FLOW, DP_TYPE_BOOL},
    {DPID_CHILD_LOCK, DP_TYPE_BOOL},
    {DPID_STATUS, DP_TYPE_ENUM},
    {DPID_RO_RESET, DP_TYPE_BOOL},
    {DPID_PAC_RESET, DP_TYPE_BOOL},
    {DPID_CF_RESET, DP_TYPE_BOOL},
    {DPID_SWITCH, DP_TYPE_BOOL},
    {DPID_TEMP, DP_TYPE_ENUM},
    {DPID_FAULT, DP_TYPE_RAW},
    {DPID_SET_FLOW, DP_TYPE_ENUM},
    {DPID_PAC_DAYLEFT, DP_TYPE_VALUE},
    {DPID_RO_DAYLEFT, DP_TYPE_VALUE},
    {DPID_CF_DAYLEFT, DP_TYPE_VALUE},
    {DPID_AUTO_CLEAN, DP_TYPE_BOOL},
};
/***********************************************************************************************************************
 * Private global variables and functions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Exported global variables and functions (to be accessed by other files)
 ***********************************************************************************************************************/
extern "C"
{
    void main_recieve_callback(unsigned char *buf, unsigned short len);
    static void get_data_from_raw_data(unsigned char dpid, unsigned short index, const unsigned char value[], unsigned short len);
}

void main_recieve_callback(unsigned char *buf, unsigned short len)
{
    tuya_wifi.uart_service_2(buf, len);
}

/**
 * @brief
 *
 * @param dpid
 * @param index index of table data point
 * @param value
 * @param length
 * @return unsigned char
 */
unsigned char main_parser_data(unsigned char dpid, unsigned short index, const unsigned char value[], unsigned short length)
{
    ESP_LOGI(TAG, "dpid = %d", dpid);
    ESP_LOGI(TAG, "index = %d", index);
    get_data_from_raw_data(dpid, index, value, length);
    // ESP_LOGI(TAG, "value = %d");
    // ESP_LOGI(TAG, "value = %d");

    return 1;
}

/***********************************************************************************************************************
 * Imported global variables and functions (from other files)
 ***********************************************************************************************************************/

/**
 * @brief
 *
 * @param p
 * @return uint8_t
 */
void WaterPurifier_init(void)
{
    tuya_wifi.dp_parser_func_register(main_parser_data);
    uart_recieve_callback_init(main_recieve_callback);
    user_uart_init();
    tuya_wifi.set_dp_cmd_total(dp_array, 20);
}

void WaterPurifier_Process(void)
{
    tuya_wifi.ProcessTask();
}
/***********************************************************************************************************************
 * static functions
 ***********************************************************************************************************************/

//=============================================================================
// dp data point type
//=============================================================================

static void get_data_from_raw_data(unsigned char dpid, unsigned short index, const unsigned char value[], unsigned short len)
{
    ESP_LOGI(TAG, "get_data_from_raw_data");
    switch (dp_array[index][1])
    {
    case DP_TYPE_RAW:
        ESP_LOGI(TAG, "DP_TYPE_RAW");
        /* code */
        break;
    case DP_TYPE_BOOL:
        ESP_LOGI(TAG, "DP_TYPE_BOOL: %d",value[0] );
        water_purifier_packet_data_bool(dpid, value[0]);
        break;
    case DP_TYPE_VALUE:
        ESP_LOGI(TAG, "DP_TYPE_VALUE");
        water_purifier_packet_data_value(dpid, value);
        break;
    case DP_TYPE_STRING:
        ESP_LOGI(TAG, "DP_TYPE_STRING");
        break;
    case DP_TYPE_ENUM:
        water_purifier_packet_data_enum(dpid, value[0]);
        ESP_LOGI(TAG, "DP_TYPE_ENUM");
        break;
    case DP_TYPE_BITMAP:
        ESP_LOGI(TAG, "DP_TYPE_BITMAP");
        break;
    default:
        break;
    }
}
/***********************************************************************************************************************
 * End of file
 ***********************************************************************************************************************/