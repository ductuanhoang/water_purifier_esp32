/**
 * @file TuyaUart.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-03-19
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef TUYA_UART_H_
#define TUYA_UART_H_

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/


/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/


/****************************************************************************/
/***         Exported global functions                                     ***/
/****************************************************************************/
#include "config.h"
#include "TuyaDefs.h"

class TuyaUart
{
public:
    volatile unsigned char wifi_uart_rx_buf[PROTOCOL_HEAD + WIFI_UART_RECV_BUF_LMT];     //Serial data processing buffer
    volatile unsigned char wifi_uart_tx_buf[PROTOCOL_HEAD + WIFIR_UART_SEND_BUF_LMT];    //Serial receive buffer
    volatile unsigned char wifi_data_process_buf[PROTOCOL_HEAD + WIFI_DATA_PROCESS_LMT]; //Serial port send buffer
    volatile unsigned short wifi_data_process_len;
    TuyaUart(void);
    ~TuyaUart(void);
    void wifi_protocol_init(void);

    unsigned char uart_receive_input(unsigned char data);
    void uart_receive_buff_input(unsigned char value[], unsigned short data_len);
    unsigned char take_byte_rxbuff(void);
    unsigned char with_data_rxbuff(void);

    void uart_transmit_output(unsigned char value);
    void wifi_uart_write_data(unsigned char *in, unsigned short len);
    void wifi_uart_write_frame(unsigned char fr_type, unsigned char fr_ver, unsigned short len);
    unsigned short set_wifi_uart_byte(unsigned short dest, unsigned char byte);
    unsigned short set_wifi_uart_buffer(unsigned short dest, const unsigned char *src, unsigned short len);

    void wifi_uart_write_frame_heartbeat(void);
    void wifi_uart_write_frame_QueryProductInfo(void);
    void wifi_uart_write_frame_QueryWorkingMode(void);
    void wifi_uart_write_frame_ReportWiFiStatus(void);
    void wifi_uart_write_frame_QueryDpStatus(void);
    /* serial set */
    bool _isHWSerial;

    // void set_serial(HardwareSerial *serial);
    // void set_serial(SoftwareSerial *serial);
    void begin(long baud_rate);
    char read(void);
    int write(char value);
    int available(void);

private:
    volatile unsigned char *rx_buf_in;
    volatile unsigned char *rx_buf_out;
};


#endif /* TUYA_UART_H_ */
