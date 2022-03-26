/**
 * @file TuyaWifi.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-03-19
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef TUYA_WIFI_H_
#define TUYA_WIFI_H_

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
#define PID_LEN 16
#define VER_LEN 5

extern "C"
{
    typedef void (*tuya_callback_dp_update_all)(void);
    typedef unsigned char (*tuya_callback_dp_download)(unsigned char dpid, const unsigned char value[], unsigned short length);
    typedef unsigned char (*tuya_callback_dp_parser)(unsigned char dpid, const unsigned char value[], unsigned short length);
}

class TuyaWifi
{
public:
    typedef enum
    {
        E_TUYA_WIFI_REQUEST_HEAT_BEAT,
        E_TUYA_WIFI_REQUEST_HEAT_BEAT_AFTER_15SECOND,
        E_TUYA_WIFI_REQUEST_PRODUCT_INFO,
        E_TUYA_WIFI_SEND_WORKING_STATE,
        E_TUYA_WIFI_CONTROL_EXAMPLE,
        E_TUYA_WIFI_IDLE,
    } TuyaWifi_Process_t;
    TuyaWifi(void);
    void ProcessTask(void);
    unsigned char init(unsigned char *pid, unsigned char *mcu_ver);
    void uart_service(void);
    void uart_service_2(unsigned char *data, unsigned short length);
    void dp_process_func_register(tuya_callback_dp_download _func);
    void dp_update_all_func_register(tuya_callback_dp_update_all _func);
    void dp_parser_func_register(tuya_callback_dp_parser _func);
    void set_dp_cmd_total(unsigned char download_cmd_array[][2], unsigned char download_cmd_num);

    unsigned long mcu_get_dp_download_data(unsigned char dpid, const unsigned char value[], unsigned short len);

    /* char * */
    unsigned char mcu_dp_update(unsigned char dpid, const unsigned char value[], unsigned short len); /* char raw */
    /* unsigned long / long */
    unsigned char mcu_dp_update(unsigned char dpid, unsigned long value, unsigned short len);
    unsigned char mcu_dp_update(unsigned char dpid, long value, unsigned short len);
    /* unsigned char / char */
    unsigned char mcu_dp_update(unsigned char dpid, unsigned char value, unsigned short len);
    unsigned char mcu_dp_update(unsigned char dpid, char value, unsigned short len);
    /* unsigned int / int */
    unsigned char mcu_dp_update(unsigned char dpid, unsigned int value, unsigned short len);
    unsigned char mcu_dp_update(unsigned char dpid, int value, unsigned short len);

#if WIFI_CONTROL_SELF_MODE
    void set_state_pin(unsigned char led_pin, unsigned char key_pin);
#else
    unsigned char mcu_get_wifimode_flag(void);
    void mcu_set_wifi_mode(unsigned char mode);
    unsigned char mcu_get_reset_wifi_flag(void);
    void mcu_reset_wifi(void);
    unsigned char mcu_get_wifi_work_state(void);
#endif /* WIFI_CONTROL_SELF_MODE */

#if SUPPORT_GREEN_TIME
    char TuyaWifi::get_green_time(TUYA_WIFI_TIME *time, const unsigned int timeout);
#endif /* SUPPORT_GREEN_TIME */

#if SUPPORT_RTC_TIME
    char TuyaWifi::get_rtc_time(TUYA_WIFI_TIME *time, const unsigned int timeout);
#endif /* SUPPORT_GREEN_TIME */
    /**
     * @brief heat beat send from module wifi to MCU
     *
     */
    void heat_beat_send(void);
    /**
     * @brief query product information from wifi to mcu
     *
     */
    void product_info_query(void);
    /**
     * @brief query mode working mode of MCU
     *
     */
    void working_mode_query(void);

    void send_mcu_wifi_mode(unsigned char mode);
    void send_control_on_off(void);

private:
#if WIFI_CONTROL_SELF_MODE
    volatile unsigned char wifi_state_led = 16;
    volatile unsigned char wifi_reset_key = 28;
#else
    volatile unsigned char reset_wifi_flag;   // Reset wifi flag (TRUE: successful / FALSE: failed)
    volatile unsigned char set_wifimode_flag; // Set the WIFI working mode flag (TRUE: Success / FALSE: Failed)
    volatile unsigned char wifi_work_state;   // Wifi module current working status
#endif /* WIFI_CONTROL_SELF_MODE */

#if SUPPORT_GREEN_TIME
    volatile TUYA_WIFI_TIME _green_time;
#endif /* SUPPORT_GREEN_TIME */

#if SUPPORT_RTC_TIME
    volatile TUYA_WIFI_TIME _rtc_time;
#endif /* SUPPORT_RTC_TIME */
    unsigned char product_id[PID_LEN];
    unsigned char mcu_ver_value[VER_LEN];

    unsigned char download_dp_number;
    TuyaWifi_Process_t wifi_process_state = E_TUYA_WIFI_REQUEST_HEAT_BEAT;
    bool device_online_state = false;
    unsigned char (*download_cmd)[2];

    tuya_callback_dp_download dp_download_handle;
    tuya_callback_dp_update_all all_data_update;
    tuya_callback_dp_parser dp_parser_handle;
    void data_handle(unsigned short offset);
    void heat_beat_check(void);
    void product_info_update(void);
    void product_info_parse(unsigned char *message, unsigned short length);
    void get_mcu_wifi_mode(void);
    unsigned char data_point_handle(const unsigned char value[]);
    unsigned char data_point_parser_handle(const unsigned char value[]);
    unsigned char get_dowmload_dpid_index(unsigned char dpid);
};

/****************************************************************************/
/***         Exported global functions                                     ***/
/****************************************************************************/

#endif /* TUYA_WIFI_H_ */
