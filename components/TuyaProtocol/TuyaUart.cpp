/**
 * @file TuyaWifi.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-03-19
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
#include <stddef.h>
#include <stdio.h>
#include "TuyaWifi.h"
#include "TuyaUart.h"
#include "TuyaTools.h"
#include "user_uart.h"
/***********************************************************************************************************************
 * Macro definitions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Typedef definitions
 ***********************************************************************************************************************/
extern volatile unsigned char stop_update_flag; // ENABLE:Stop all data uploads   DISABLE:Restore all data uploads

/***********************************************************************************************************************
 * Private global variables and functions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Exported global variables and functions (to be accessed by other files)
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Imported global variables and functions (from other files)
 ***********************************************************************************************************************/

extern TuyaTools tuya_tools;

TuyaUart::TuyaUart(void)
{
    wifi_protocol_init();
}

TuyaUart::~TuyaUart(void)
{
}

void TuyaUart::wifi_protocol_init(void)
{
    rx_buf_in = (unsigned char *)wifi_uart_rx_buf;
    rx_buf_out = (unsigned char *)wifi_uart_rx_buf;

    stop_update_flag = TY_DISABLE;
}

void TuyaUart::uart_transmit_output(unsigned char value)
{
    write(value);
}

unsigned char TuyaUart::uart_receive_input(unsigned char data)
{
    if (1 == rx_buf_out - rx_buf_in)
    {
        // UART receive buffer is full
        return TY_ERROR;
    }
    else if ((rx_buf_in > rx_buf_out) && ((unsigned long)(rx_buf_in - rx_buf_out) >= sizeof(wifi_uart_rx_buf)))
    {
        // UART receive buffer is full
        return TY_ERROR;
    }
    else
    {
        // UART receive buffer is not full
        if (rx_buf_in >= (unsigned char *)(wifi_uart_rx_buf + sizeof(wifi_uart_rx_buf)))
        {
            rx_buf_in = (unsigned char *)(wifi_uart_rx_buf);
        }

        *rx_buf_in++ = data;
    }
    return TY_SUCCESS;
}

void TuyaUart::uart_receive_buff_input(unsigned char value[], unsigned short data_len)
{
    unsigned short i = 0;
    for (i = 0; i < data_len; i++)
    {
        uart_receive_input(value[i]);
    }
}

unsigned char TuyaUart::take_byte_rxbuff(void)
{
    unsigned char date = 0;

    if (rx_buf_out != rx_buf_in)
    {
        // With data
        if (rx_buf_out >= (unsigned char *)(wifi_uart_rx_buf + sizeof(wifi_uart_rx_buf)))
        {
            // The data has reached the end
            rx_buf_out = (unsigned char *)(wifi_uart_rx_buf);
        }

        date = *rx_buf_out++;
    }

    return date;
}

unsigned char TuyaUart::with_data_rxbuff(void)
{
    if (rx_buf_in != rx_buf_out)
        return 1;
    else
        return 0;
}

void TuyaUart::wifi_uart_write_data(unsigned char *in, unsigned short len)
{
    if ((NULL == in) || (0 == len))
    {
        return;
    }

    // while (len--)
    // {
    //     uart_transmit_output(*in);
    //     in++;
    // }
    uart_send_callback(in, len);
}

void TuyaUart::wifi_uart_write_frame(unsigned char fr_type, unsigned char fr_ver, unsigned short len)
{
    unsigned char check_sum = 0;

    wifi_uart_tx_buf[HEAD_FIRST] = 0x55;
    wifi_uart_tx_buf[HEAD_SECOND] = 0xaa;
    wifi_uart_tx_buf[PROTOCOL_VERSION] = fr_ver;
    wifi_uart_tx_buf[FRAME_TYPE] = fr_type;
    wifi_uart_tx_buf[LENGTH_HIGH] = len >> 8;
    wifi_uart_tx_buf[LENGTH_LOW] = len & 0xff;

    len += PROTOCOL_HEAD;
    check_sum = tuya_tools.get_check_sum((unsigned char *)wifi_uart_tx_buf, len - 1);
    wifi_uart_tx_buf[len - 1] = check_sum;

    wifi_uart_write_data((unsigned char *)wifi_uart_tx_buf, len);
}
/**
 * @brief 
 * 
 */
void TuyaUart::wifi_uart_write_frame_heartbeat(void)
{
    unsigned short len = 0;
    // 55 aa 00 00 00 00 ff
    unsigned char check_sum = 0xff;

    wifi_uart_tx_buf[HEAD_FIRST] = 0x55;
    wifi_uart_tx_buf[HEAD_SECOND] = 0xaa;
    wifi_uart_tx_buf[PROTOCOL_VERSION] = 0x00;
    wifi_uart_tx_buf[FRAME_TYPE] = 0x00;
    wifi_uart_tx_buf[LENGTH_HIGH] = 0x00 >> 8;
    wifi_uart_tx_buf[LENGTH_LOW] = 0x00 & 0xff;

    len += PROTOCOL_HEAD;
    wifi_uart_tx_buf[len - 1] = check_sum;
    wifi_uart_write_data((unsigned char *)wifi_uart_tx_buf, len - 1);
}
/**
 * @brief 
 * 
 */
void TuyaUart::wifi_uart_write_frame_QueryProductInfo(void)
{
    // 55 aa 00 01 00 00 00
    unsigned short len = 0;
    unsigned char check_sum = 0x00;

    wifi_uart_tx_buf[HEAD_FIRST] = 0x55;
    wifi_uart_tx_buf[HEAD_SECOND] = 0xaa;
    wifi_uart_tx_buf[PROTOCOL_VERSION] = 0x00;
    wifi_uart_tx_buf[FRAME_TYPE] = 0x01;
    wifi_uart_tx_buf[LENGTH_HIGH] = 0x00 >> 8;
    wifi_uart_tx_buf[LENGTH_LOW] = 0x00 & 0xff;

    len += PROTOCOL_HEAD;
    wifi_uart_tx_buf[len - 1] = check_sum;
    wifi_uart_write_data((unsigned char *)wifi_uart_tx_buf, len - 1);
}
/**
 * @brief 
 * 
 */
void TuyaUart::wifi_uart_write_frame_QueryWorkingMode(void)
{
    // 55 aa 00 02 00 00 01
    unsigned short len = 0;
    unsigned char check_sum = 0x01;

    wifi_uart_tx_buf[HEAD_FIRST] = 0x55;
    wifi_uart_tx_buf[HEAD_SECOND] = 0xaa;
    wifi_uart_tx_buf[PROTOCOL_VERSION] = 0x00;
    wifi_uart_tx_buf[FRAME_TYPE] = 0x02;
    wifi_uart_tx_buf[LENGTH_HIGH] = 0x00 >> 8;
    wifi_uart_tx_buf[LENGTH_LOW] = 0x00 & 0xff;

    len += PROTOCOL_HEAD;
    wifi_uart_tx_buf[len - 1] = check_sum;
    wifi_uart_write_data((unsigned char *)wifi_uart_tx_buf, len - 1);
}

void TuyaUart::wifi_uart_write_frame_ReportWiFiStatus(void)
{
    // TODO
}

unsigned short TuyaUart::set_wifi_uart_byte(unsigned short dest, unsigned char byte)
{
    unsigned char *obj = (unsigned char *)wifi_uart_tx_buf + DATA_START + dest;

    *obj = byte;
    dest += 1;

    return dest;
}

unsigned short TuyaUart::set_wifi_uart_buffer(unsigned short dest, const unsigned char *src, unsigned short len)
{
    unsigned char *obj = (unsigned char *)wifi_uart_tx_buf + DATA_START + dest;

    tuya_tools.my_memcpy(obj, src, len);

    dest += len;
    return dest;
}

void TuyaUart::begin(long baud_rate)
{
}

char TuyaUart::read(void)
{
    return 1;
}

int TuyaUart::write(char value)
{
    return 1;
}

int TuyaUart::available(void)
{
    return 1;
}

/***********************************************************************************************************************
 * static functions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * End of file
 ***********************************************************************************************************************/