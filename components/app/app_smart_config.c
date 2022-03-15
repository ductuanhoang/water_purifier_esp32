/**
 * @file app_smart_config.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-03-15
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
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <stddef.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_log.h>
#include <esp_event.h>

#include "app_smart_config.h"
#include "user_ble.h"
#include "json_bluetooth.h"
/***********************************************************************************************************************
 * Macro definitions
 ***********************************************************************************************************************/
const char *TAG = "app_config";
/***********************************************************************************************************************
 * Typedef definitions
 ***********************************************************************************************************************/
typedef struct
{
    char ssid[32];
    char pwd[64];
} app_config_t;

/***********************************************************************************************************************
 * Private global variables and functions
 ***********************************************************************************************************************/
static void ble_recv_cb(char *buf, int len);
static void ble_read_cb(void);
static uint32_t gen_random_token(void);
static void app_smartconfig_get_mac_name(char *mac_name, size_t max);

static app_config_t app_config = {0};
static char TOKEN[5];
static uint8_t MAC_BLE[6];
static char DEVICE_NAME[30];
/***********************************************************************************************************************
 * Exported global variables and functions (to be accessed by other files)
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Imported global variables and functions (from other files)
 ***********************************************************************************************************************/

/**
 * @brief init ble connection for smartconfig
 *
 */
void app_smart_config_init(void)
{
    ble_server_set_write_callback(ble_recv_cb);
    ble_server_set_read_callback(ble_read_cb);
    ble_server_start();
    uint32_t rand = gen_random_token();
    sprintf(TOKEN, "%04d", rand);
    ESP_LOGW(TAG, "Gen Token: %s", TOKEN);
    app_smartconfig_get_mac_name(DEVICE_NAME, 30);
    // create new task
}

/***********************************************************************************************************************
 * static functions
 ***********************************************************************************************************************/
static uint32_t gen_random_token(void)
{
    uint32_t random = (uint32_t)rand();
    random = random % 10000;
    if (random < 1000)
        random += 1000;
    return random;
}

/**
 * @brief     BLE write command callback the data may pare by json: {"ssid":"Phenikaa Smart Device","pwd":"1234567891011"}
 * @param     buf  data receiver from smart phone
 * @param     len  Lenght of buffer buf
 * @return    None
 */
static void ble_recv_cb(char *buf, int len)
{
    ESP_LOGI(TAG, "BLE:%.*s", len, buf);
    if (buf[0] == ':')
    {
        static bool recv_ssid = false;
        static bool recv_pass = false;
        static uint8_t frm_ssid_old[30];
        static uint8_t frm_pass_old[30];
        static uint8_t frm_new[30];
        static bool new_ssid = true;
        static bool new_pass = true;
        if (buf[1] == 's')
        {
            new_ssid = true;
            memset(app_config.ssid, 0, sizeof(app_config.ssid));
            memcpy(app_config.ssid, &buf[2], len - 2);
            ESP_LOGI(TAG, "[BLE]SSID:%s", app_config.ssid);
            recv_ssid = true;
            memset(frm_new, 0, sizeof(frm_new));
            memcpy(frm_new, buf, len);
            if (memcmp(frm_ssid_old, frm_new, 30) == 0)
            {
                new_ssid = false;
                ESP_LOGI(TAG, "Old ssid");
            }
            memset(frm_ssid_old, 0, sizeof(frm_ssid_old));
            memcpy(frm_ssid_old, buf, len);
        }
        else if (buf[1] == 'p')
        {
            new_pass = true;
            memset(app_config.pwd, 0, sizeof(app_config.pwd));
            memcpy(app_config.pwd, &buf[2], len - 2);
            ESP_LOGI(TAG, "[BLE]PASS:%s", app_config.pwd);
            recv_pass = true;
            memset(frm_new, 0, sizeof(frm_new));
            memcpy(frm_new, buf, len);
            if (memcmp(frm_pass_old, frm_new, 30) == 0)
            {
                new_pass = false;
                ESP_LOGI(TAG, "Old pass");
            }
            memset(frm_pass_old, 0, sizeof(frm_pass_old));
            memcpy(frm_pass_old, buf, len);
        }

        if (recv_ssid && recv_pass && (new_ssid || new_pass))
        {
            recv_ssid = false;
            recv_pass = false;
            // if (config_done_event)
            //     xEventGroupSetBits(config_done_event, BLE_GOT_SSID_PWD_EVENT);
        }
    }
    else
    {
        if (strstr(buf, "indentify"))
        {
            // set_indentify();
        }
    }
}

/**
 * @brief     ble read command callback
 * @param     None
 * @return    None
 */
static void ble_read_cb(void)
{
    char message_packet[255];
    json_bluetooth_packet(&message_packet, TOKEN, true, (const char *)DEVICE_NAME);
    ESP_LOGI(TAG, "message_packet = %s", message_packet);
    ble_server_send_response_data(message_packet, strlen(message_packet));
}

/**
 * @brief 
 * 
 * @param mac_name 
 * @param max 
 */
static void app_smartconfig_get_mac_name(char *mac_name, size_t max)
{
    ESP_ERROR_CHECK(esp_read_mac(MAC_BLE, ESP_MAC_BT));
    snprintf(mac_name, max, "%02X%02X%02X%02X%02X%02X", MAC_BLE[0], MAC_BLE[1], MAC_BLE[2], MAC_BLE[3], MAC_BLE[4], MAC_BLE[5]);
}

/***********************************************************************************************************************
 * End of file
 ***********************************************************************************************************************/
