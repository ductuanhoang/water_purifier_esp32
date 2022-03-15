/**
 * @file softuart.h
 * @author your name (you@domain.com)
 * @brief base on Software Uart For Stm32
 * 		  https://github.com/liyanboy74
 * @version 0.1
 * @date 2022-01-19
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdint.h>
#include <stdio.h>

#define 	Number_Of_SoftUarts 	6 	// Max 8

#define 	SoftUartTxBufferSize	32
#define 	SoftUartRxBufferSize	64

typedef enum {
	SoftUart_OK,
	SoftUart_Error
}SoftUartState_E;

typedef struct{
	uint8_t			Tx[SoftUartTxBufferSize];
	uint8_t			Rx[SoftUartRxBufferSize];
}SoftUartBuffer_S;

/**
 * @brief tx_pin callback function
 * 
 */
typedef void (*tx_pin_callback)(uint8_t state);

/**
 * @brief rx_pin callback function
 * 
 */
typedef uint8_t (*rx_pin_callback)(void);

/**
 * @brief struct software UART
 * 
 */
typedef struct {
	volatile uint8_t 		TxNComplated;
	uint8_t			TxEnable;
	uint8_t			RxEnable;
	uint8_t 		TxBitShift,TxBitCounter;
	uint8_t 		RxBitShift,RxBitCounter;
	uint8_t			TxIndex,TxSize;
	uint8_t			RxIndex;
	SoftUartBuffer_S	*Buffer;
	// GPIO_TypeDef  		*TxPort;
	// uint16_t 		TxPin;
	// GPIO_TypeDef  		*RxPort;
	// uint16_t 		RxPin;
	rx_pin_callback rx_pin;
	tx_pin_callback tx_pin;
	uint8_t 		RxTimingFlag;
	uint8_t 		RxBitOffset;
	
} SoftUart_S;

//Call Every (0.2)*(1/9600) = 20.83 uS
void 		SoftUartHandler(void);

void 		SoftUartWaitUntilTxComplate(uint8_t SoftUartNumber);
uint8_t 	SoftUartRxAlavailable(uint8_t SoftUartNumber);
SoftUartState_E SoftUartPuts(uint8_t SoftUartNumber,uint8_t *Str,uint8_t Len);
SoftUartState_E SoftUartEnableRx(uint8_t SoftUartNumber);
SoftUartState_E SoftUartDisableRx(uint8_t SoftUartNumber);
SoftUartState_E SoftUartInit(uint8_t SoftUartNumber, tx_pin_callback tx_pin, rx_pin_callback rx_pin);
SoftUartState_E SoftUartReadRxBuffer(uint8_t SoftUartNumber,uint8_t *Buffer,uint8_t Len);
