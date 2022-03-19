/**
 * @file user_uart.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-03-19
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef USER_UART_H_
#define USER_UART_H_

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef void (*uart_write_callback_t)(uint8_t *, uint8_t);
typedef void (*uart_read_callback_t)(uint8_t *, uint8_t);

/****************************************************************************/
/***         Exported global functions                                     ***/
/****************************************************************************/

/**
 * @brief init ble connection for smartconfig
 * 
 */
void user_uart_init(void);


void uart_recieve_callback_init(uart_read_callback_t *callback);

void uart_send_callback(uint8_t *data, uint8_t length);

#endif /* USER_UART_H_ */