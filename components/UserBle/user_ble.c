#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "sdkconfig.h"
#include "user_ble_config.h"
#include "user_ble.h"
static void ble_server_write_callback_default(char *buf, int len);
static ble_server_write_callback_t ble_server_write_callback = ble_server_write_callback_default;
static void ble_server_read_callback_default(void);
static ble_server_read_callback_t ble_server_read_callback = ble_server_read_callback_default;
static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

#define TAG "ble_server"

static bool is_connect = false;
static char DEVICE_NAME[30];
static uint8_t MAC_BLE[6];

#define GATTS_SERVICE_UUID_A 0x00FF // Service
#define GATTS_CHAR_UUID_A 0xFF01    // Read/Write Character
#define GATTS_NUM_HANDLE_A 4

#define GATTS_DEMO_CHAR_VAL_LEN_MAX 0x40
#define PREPARE_BUF_MAX_SIZE 1024
static uint16_t gatts_mtu = 23;
static uint8_t char1_str[] = {0x11, 0x22, 0x33};
static esp_gatt_char_prop_t a_property = 0;
/// Declare the static function

static esp_attr_value_t gatts_demo_char1_val =
    {
        .attr_max_len = GATTS_DEMO_CHAR_VAL_LEN_MAX,
        .attr_len = sizeof(char1_str),
        .attr_value = char1_str,
};

static uint8_t adv_config_done = 0;
#define adv_config_flag (1 << 0)
#define scan_rsp_config_flag (1 << 1)

static uint8_t adv_service_uuid128[32] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    // first uuid, 16bit, [12],[13] is the value
    0xfb,
    0x34,
    0x9b,
    0x5f,
    0x80,
    0x00,
    0x00,
    0x80,
    0x00,
    0x10,
    0x00,
    0x00,
    0xEE,
    0x00,
    0x00,
    0x00,
    // second uuid, 32bit, [12], [13], [14], [15] is the value
    0xfb,
    0x34,
    0x9b,
    0x5f,
    0x80,
    0x00,
    0x00,
    0x80,
    0x00,
    0x10,
    0x00,
    0x00,
    0xFF,
    0x00,
    0x00,
    0x00,
};

// The length of adv data must be less than 31 bytes
// adv data
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006, // slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x000C, // slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = 0x00,
    .manufacturer_len = 0,       // TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};
// scan response data
static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006,
    .max_interval = 0x000C,
    .appearance = 0x00,
    .manufacturer_len = 0,       // TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    //.peer_addr            =
    //.peer_addr_type       =
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

#define PROFILE_NUM 1
#define PROFILE_A_APP_ID 0

struct gatts_profile_inst
{
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
static struct gatts_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_A_APP_ID] = {
        .gatts_cb = gatts_profile_a_event_handler,
        .gatts_if = ESP_GATT_IF_NONE, /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
};
typedef struct
{
    esp_gatt_if_t gatts_if;
    esp_ble_gatts_cb_param_t *param;
} ble_server_t;
static ble_server_t ble_server;
typedef struct
{
    uint8_t *buf;
    int len;
    uint16_t handle;
} prepare_type_env_t;

static prepare_type_env_t write_env;
static prepare_type_env_t read_env;

bool is_ble_connected(void)
{
    return is_connect;
}

// static uint8_t MAC_addr[6] = { 0 };
static void ble_get_mac_name(char *mac_name, size_t max)
{
    ESP_ERROR_CHECK(esp_read_mac(MAC_BLE, ESP_MAC_BT));
    snprintf(mac_name, max, "%s%02X%02X%02X%02X%02X%02X", DEVICE_NAME_PREFIX, MAC_BLE[0], MAC_BLE[1], MAC_BLE[2], MAC_BLE[3], MAC_BLE[4], MAC_BLE[5]);
}

void ble_server_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {

    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~adv_config_flag);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~scan_rsp_config_flag);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;

    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        // advertising start complete event to indicate advertising start successfully or failed
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG, "Advertising start failed\n");
        }
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG, "Advertising stop failed\n");
        }
        else
        {
            ESP_LOGI(TAG, "Stop adv successfully\n");
        }
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        ESP_LOGI(TAG, "update connetion params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                 param->update_conn_params.status,
                 param->update_conn_params.min_int,
                 param->update_conn_params.max_int,
                 param->update_conn_params.conn_int,
                 param->update_conn_params.latency,
                 param->update_conn_params.timeout);
        break;
    default:
        break;
    }
}

void ble_server_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param)
{
    esp_gatt_status_t status = ESP_GATT_OK;
    if (param->write.need_rsp)
    {
        if (param->write.is_prep)
        {
            if (prepare_write_env->buf == NULL)
            {
                prepare_write_env->buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE * sizeof(uint8_t));
                prepare_write_env->len = 0;
                if (prepare_write_env->buf == NULL)
                {
                    ESP_LOGE(TAG, "Gatt_server prep no mem\n");
                    status = ESP_GATT_NO_RESOURCES;
                }
            }
            else
            {
                if (param->write.offset > PREPARE_BUF_MAX_SIZE || prepare_write_env->len > param->write.offset)
                {
                    status = ESP_GATT_INVALID_OFFSET;
                }
                else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE)
                {
                    status = ESP_GATT_INVALID_ATTR_LEN;
                }
            }

            esp_gatt_rsp_t *gatt_rsp = (esp_gatt_rsp_t *)malloc(sizeof(esp_gatt_rsp_t));
            gatt_rsp->attr_value.len = param->write.len;
            gatt_rsp->attr_value.handle = param->write.handle;
            gatt_rsp->attr_value.offset = param->write.offset;
            gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
            memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
            esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp);

            if (response_err != ESP_OK)
            {
                ESP_LOGE(TAG, "Send response error\n");
            }
            free(gatt_rsp);
            if (status != ESP_GATT_OK)
            {
                return;
            }
            memcpy(prepare_write_env->buf + param->write.offset,
                   param->write.value,
                   param->write.len);
            prepare_write_env->len += param->write.len;
        }
        else
        {
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, NULL);
        }
    }
}
void ble_server_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param)
{
    if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC)
    {
        ble_server_write_callback((char *)prepare_write_env->buf, prepare_write_env->len);
    }
    else
    {
        ESP_LOGI(TAG, "ESP_GATT_PREP_WRITE_CANCEL");
    }
    if (prepare_write_env->buf)
    {
        free(prepare_write_env->buf);
        prepare_write_env->buf = NULL;
    }
    prepare_write_env->len = 0;
}
void gatts_proc_read(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_read_env, esp_ble_gatts_cb_param_t *param, uint8_t *p_rsp_v, uint16_t v_len)
{
    if (!param->read.need_rsp)
    {
        return;
    }

    uint16_t value_len = gatts_mtu - 1;
    if (v_len - param->read.offset < (gatts_mtu - 1))
    { // read response will fit in one MTU?
        value_len = v_len - param->read.offset;
    }
    else if (param->read.offset == 0) // it's the start of a long read  (could also use param->read.is_long here?)
    {
        if (v_len > PREPARE_BUF_MAX_SIZE)
        {
            ESP_LOGE(TAG, "long read too long");
            return;
        }
        if (prepare_read_env->buf != NULL)
        {
            ESP_LOGW(TAG, "long read buffer not free");
            free(prepare_read_env->buf);
            prepare_read_env->buf = NULL;
        }

        prepare_read_env->buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE * sizeof(uint8_t));
        prepare_read_env->len = 0;
        if (prepare_read_env->buf == NULL)
        {
            ESP_LOGE(TAG, "long read no mem");
            return;
        }
        memcpy(prepare_read_env->buf, p_rsp_v, v_len);
        prepare_read_env->len = v_len;
        prepare_read_env->handle = param->read.handle;
    }
    esp_gatt_rsp_t rsp;
    memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
    rsp.attr_value.handle = param->read.handle;
    rsp.attr_value.len = value_len;
    rsp.attr_value.offset = param->read.offset;
    memcpy(rsp.attr_value.value, &p_rsp_v[param->read.offset], value_len);
    esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &rsp);
}

bool gatts_proc_long_read(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_read_env, esp_ble_gatts_cb_param_t *param)
{
    bool done = false;
    if (prepare_read_env->buf && (prepare_read_env->handle == param->read.handle)) // something buffered?
    {
        gatts_proc_read(gatts_if, prepare_read_env, param, prepare_read_env->buf, prepare_read_env->len);
        if (prepare_read_env->len - param->read.offset < (gatts_mtu - 1)) // last read?
        {
            free(prepare_read_env->buf);
            prepare_read_env->buf = NULL;
            prepare_read_env->len = 0;
            ESP_LOGI(TAG, "long_read ended");
            done = true;
        }
    }
    else
    {
        ESP_LOGE(TAG, "long_read not buffered");
    }
    return done;
}

void ble_send_indicate_data(uint8_t *data, uint16_t data_len)
{
    esp_ble_gatts_send_indicate(ble_server.gatts_if, ble_server.param->write.conn_id, gl_profile_tab[PROFILE_A_APP_ID].char_handle,
                                data_len, data, false);
}

static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTS_REG_EVT:
        ESP_LOGI(TAG, "REGISTER_APP_EVT, status %d, app_id %d", param->reg.status, param->reg.app_id);
        gl_profile_tab[PROFILE_A_APP_ID].service_id.is_primary = true;
        gl_profile_tab[PROFILE_A_APP_ID].service_id.id.inst_id = 0x00;
        gl_profile_tab[PROFILE_A_APP_ID].service_id.id.uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_A_APP_ID].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_A;
        gl_profile_tab[PROFILE_A_APP_ID].gatts_if = gatts_if;

        ble_get_mac_name(DEVICE_NAME, 30);
        esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(DEVICE_NAME);
        if (set_dev_name_ret)
        {
            ESP_LOGE(TAG, "set device name failed, error code = %x", set_dev_name_ret);
        }

        // config adv data
        esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
        if (ret)
        {
            ESP_LOGE(TAG, "config adv data failed, error code = %x", ret);
        }
        adv_config_done |= adv_config_flag;
        // config scan response data
        ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
        if (ret)
        {
            ESP_LOGE(TAG, "config scan response data failed, error code = %x", ret);
        }
        adv_config_done |= scan_rsp_config_flag;

        esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_A_APP_ID].service_id, GATTS_NUM_HANDLE_A);
        break;
    case ESP_GATTS_READ_EVT:
        ESP_LOGI(TAG, "GATT_READ_EVT, conn_id %d, trans_id %d, handle %d offset %d", param->read.conn_id, param->read.trans_id, param->read.handle, param->read.offset);
        ble_server.param = param;
        ble_server.gatts_if = gatts_if;
        ble_server_read_callback();
        break;
    case ESP_GATTS_WRITE_EVT:
        ESP_LOGI(TAG, "GATT_WRITE_EVT");
        if (!param->write.is_prep)
        {
            ble_server_write_callback((char *)param->write.value, param->write.len);
            if (gl_profile_tab[PROFILE_A_APP_ID].descr_handle == param->write.handle && param->write.len == 2)
            {
                uint16_t descr_value = param->write.value[1] << 8 | param->write.value[0];
                if (descr_value == 0x0001)
                {
                    if (a_property & ESP_GATT_CHAR_PROP_BIT_NOTIFY)
                    {

                        ESP_LOGI(TAG, "notify enable");
                        ble_server.param = param;
                        ble_server.gatts_if = gatts_if;
                    }
                }
                else if (descr_value == 0x0002)
                {
                    if (a_property & ESP_GATT_CHAR_PROP_BIT_INDICATE)
                    {
                        ESP_LOGI(TAG, "indicate enable");
                        uint8_t indicate_data[600];
                        for (int i = 0; i < sizeof(indicate_data); ++i)
                        {
                            indicate_data[i] = i % 0xff;
                        }

                        for (int j = 0; j < 1000; j++)
                        {
                            // the size of indicate_data[] need less than MTU size
                            esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[PROFILE_A_APP_ID].char_handle,
                                                        sizeof(indicate_data), indicate_data, true);
                        }
                    }
                }
                else if (descr_value == 0x0000)
                {
                    a_property = 0;
                    ESP_LOGI(TAG, "notify/indicate disable ");
                }
                else
                {
                    ESP_LOGE(TAG, "unknown descr value");
                    ble_server_write_callback((char *)param->write.value, param->write.len);
                }
            }
        }
        ble_server_write_event_env(gatts_if, &write_env, param);
        break;

    case ESP_GATTS_EXEC_WRITE_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_EXEC_WRITE_EVT");

        esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        ble_server_exec_write_event_env(&write_env, param);
        break;
    case ESP_GATTS_MTU_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
        break;
    case ESP_GATTS_UNREG_EVT:
        break;
    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d", param->create.status, param->create.service_handle);
        gl_profile_tab[PROFILE_A_APP_ID].service_handle = param->create.service_handle;
        gl_profile_tab[PROFILE_A_APP_ID].char_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_A_APP_ID].char_uuid.uuid.uuid16 = GATTS_CHAR_UUID_A;

        esp_ble_gatts_start_service(gl_profile_tab[PROFILE_A_APP_ID].service_handle);
        a_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
        esp_err_t add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PROFILE_A_APP_ID].service_handle, &gl_profile_tab[PROFILE_A_APP_ID].char_uuid,
                                                        ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                                        a_property,
                                                        &gatts_demo_char1_val, NULL);
        if (add_char_ret)
        {
            ESP_LOGE(TAG, "add char failed, error code = %x", add_char_ret);
        }
        break;
    case ESP_GATTS_ADD_INCL_SRVC_EVT:
        break;
    case ESP_GATTS_ADD_CHAR_EVT:
    {
        uint16_t length = 0;
        const uint8_t *prf_char;

        ESP_LOGI(TAG, "ADD_CHAR_EVT, status %d,  attr_handle %d, service_handle %d",
                 param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
        gl_profile_tab[PROFILE_A_APP_ID].char_handle = param->add_char.attr_handle;
        gl_profile_tab[PROFILE_A_APP_ID].descr_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_A_APP_ID].descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
        esp_err_t get_attr_ret = esp_ble_gatts_get_attr_value(param->add_char.attr_handle, &length, &prf_char);
        if (get_attr_ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "ILLEGAL HANDLE");
        }

        ESP_LOGI(TAG, "the gatts demo char length = %x", length);
        for (int i = 0; i < length; i++)
        {
            ESP_LOGI(TAG, "prf_char[%x] = %x", i, prf_char[i]);
        }
        esp_err_t add_descr_ret = esp_ble_gatts_add_char_descr(gl_profile_tab[PROFILE_A_APP_ID].service_handle, &gl_profile_tab[PROFILE_A_APP_ID].descr_uuid,
                                                               ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, NULL, NULL);
        if (add_descr_ret)
        {
            ESP_LOGE(TAG, "add char descr failed, error code =%x", add_descr_ret);
        }
        break;
    }
    case ESP_GATTS_ADD_CHAR_DESCR_EVT:

        gl_profile_tab[PROFILE_A_APP_ID].descr_handle = param->add_char_descr.attr_handle;
        ESP_LOGI(TAG, "ADD_DESCR_EVT, status %d, attr_handle %d, service_handle %d",
                 param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
        break;
    case ESP_GATTS_DELETE_EVT:
        break;
    case ESP_GATTS_START_EVT:
        ESP_LOGI(TAG, "SERVICE_START_EVT, status %d, service_handle %d",
                 param->start.status, param->start.service_handle);
        break;
    case ESP_GATTS_STOP_EVT:
        break;
    case ESP_GATTS_CONNECT_EVT:
    {
        is_connect = true;
        esp_ble_conn_update_params_t conn_params = {0};
        memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
        /* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
        conn_params.latency = 0;
        conn_params.max_int = 0x20; // max_int = 0x20*1.25ms = 40ms
        conn_params.min_int = 0x10; // min_int = 0x10*1.25ms = 20ms
        conn_params.timeout = 400;  // timeout = 400*10ms = 4000ms
        ESP_LOGI(TAG, "ESP_GATTS_CONNECT_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x:",
                 param->connect.conn_id,
                 param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
                 param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5]);
        gl_profile_tab[PROFILE_A_APP_ID].conn_id = param->connect.conn_id;
        // start sent the update connection parameters to the peer device.
        // esp_ble_gap_update_conn_params(&conn_params);
        break;
    }
    case ESP_GATTS_DISCONNECT_EVT:
        is_connect = false;
        ESP_LOGI(TAG, "ESP_GATTS_DISCONNECT_EVT");
        esp_ble_gap_start_advertising(&adv_params);
        break;
    case ESP_GATTS_CONF_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_CONF_EVT, status %d", param->conf.status);

        break;
    case ESP_GATTS_OPEN_EVT:
    case ESP_GATTS_CANCEL_OPEN_EVT:
    case ESP_GATTS_CLOSE_EVT:
    case ESP_GATTS_LISTEN_EVT:
        break;
    case ESP_GATTS_CONGEST_EVT:
        break;
    default:
        break;
    }
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT)
    {
        if (param->reg.status == ESP_GATT_OK)
        {
            gl_profile_tab[param->reg.app_id].gatts_if = gatts_if;
        }
        else
        {
            ESP_LOGI(TAG, "Reg app failed, app_id %04x, status %d",
                     param->reg.app_id,
                     param->reg.status);
            return;
        }
    }

    /* If the gatts_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    do
    {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++)
        {
            if (gatts_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
                gatts_if == gl_profile_tab[idx].gatts_if)
            {
                if (gl_profile_tab[idx].gatts_cb)
                {
                    gl_profile_tab[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}
/**
 * @bref Callback default for read data client(ex: Smartphone)
 * pram: None
 * return: None
 */
static void ble_server_read_callback_default(void)
{
}
/**m
 * @bref Callback for read data from client(ex: Smartphone)
 * pram: None
 * return: None
 */

void ble_server_set_read_callback(void *callback)
{
    if (callback)
    {
        ble_server_read_callback = callback;
    }
}
/**
 * @bref Callback default for receive data from client(ex: Smartphone)
 * pram: None
 * return: None
 */
static void ble_server_write_callback_default(char *buf, int len)
{
    esp_log_buffer_hex(TAG, buf, len);
    printf("data = %s\r\n", buf);
}
/**
 * @bref Callback for receive data from client(ex: Smartphone)
 * pram: None
 * return: None
 */

void ble_server_set_write_callback(void *callback)
{
    if (callback)
    {
        ble_server_write_callback = callback;
    }
}
/**
 * @brief     Send with read character
 * @param     data  data pointer for sending
 * @param     len   data length
 * @return    true or false
 */
bool ble_server_send_response_data(char *data, int len)
{
    bool done = false;
    if (!ble_server.param->read.is_long)
    {
        esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.handle = ble_server.param->read.handle;
        rsp.attr_value.len = len;
        memcpy(rsp.attr_value.value, data, rsp.attr_value.len);
        gatts_proc_read(ble_server.gatts_if, &read_env, ble_server.param, rsp.attr_value.value, rsp.attr_value.len);
    }
    else
    {
        done = gatts_proc_long_read(ble_server.gatts_if, &read_env, ble_server.param);
    }
    return done;
}
/**
 * @bref Initializing Bluetooth server
 * pram: None
 * return: None
 */
void ble_server_start(void)
{
    esp_err_t ret;
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret)
    {
        ESP_LOGE(TAG, "%s initialize controller failed", __func__);
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret)
    {
        ESP_LOGE(TAG, "%s enable controller failed", __func__);
        return;
    }
    ret = esp_bluedroid_init();
    if (ret)
    {
        ESP_LOGE(TAG, "%s init bluetooth failed", __func__);
        return;
    }
    ret = esp_bluedroid_enable();
    if (ret)
    {
        ESP_LOGE(TAG, "%s enable bluetooth failed", __func__);
        return;
    }

    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret)
    {
        ESP_LOGE(TAG, "gatts register error, error code = %x", ret);
        return;
    }
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret)
    {
        ESP_LOGE(TAG, "gap register error, error code = %x", ret);
        return;
    }
    ret = esp_ble_gatts_app_register(PROFILE_A_APP_ID);
    if (ret)
    {
        ESP_LOGE(TAG, "gatts app register error, error code = %x", ret);
        return;
    }

    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(517);
    if (local_mtu_ret)
    {
        ESP_LOGE(TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
    }
    return;
}
/**
 * @bref Stop Bluetooth server
 * pram: None
 * return: None
 */
void ble_server_stop(void)
{
    uint8_t idx = 0;
    for (idx = 0; idx < PROFILE_NUM; idx++)
    {
        esp_ble_gatts_app_unregister(gl_profile_tab[idx].gatts_if);
    }
    if (esp_bluedroid_get_status() == ESP_BLUEDROID_STATUS_ENABLED)
        esp_bluedroid_disable();
    if (esp_bluedroid_get_status() == ESP_BLUEDROID_STATUS_INITIALIZED)
        esp_bluedroid_deinit();
    if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_ENABLED)
        esp_bt_controller_disable();
    if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_INITED)
        esp_bt_controller_deinit();
}
