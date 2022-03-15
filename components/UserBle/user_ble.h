/**
 * @file user_ble.h
 * @author tuanhd1 (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-03-15
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef _USER_BLE_SERVER_H_
#define _USER_BLE_SERVER_H_

typedef void (*ble_server_write_callback_t)(char *, int);
typedef void (*ble_server_read_callback_t)(void);
bool ble_server_send_response_data(char *data, int len);
void ble_send_indicate_data(uint8_t *data, uint16_t data_len);
void ble_server_set_write_callback(void *callback);
void ble_server_set_read_callback(void *callback);
void ble_server_stop(void);
void ble_server_start(void);
bool is_ble_connected(void);

#endif
