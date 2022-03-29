/* UART Events Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#define UART_FULL_THRESH_DEFAULT (360)
#include "user_uart.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define USER_UART_TX GPIO_NUM_17
#define USER_UART_RX GPIO_NUM_16

static const char *TAG = "user_uart";
static uart_write_callback_t uart_write_callback = NULL;
static uart_read_callback_t uart_read_callback = NULL;
/**
 * This example shows how to use the UART driver to handle special UART events.
 *
 * It also reads data from UART0 directly, and echoes it to console.
 *
 * - Port: UART0
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: on
 * - Pin assignment: TxD (default), RxD (default)
 */

#define EX_UART_NUM UART_NUM_2
#define UART_QUEUE_NUM uart2_queue

#define PATTERN_CHR_NUM (2) /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/

#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)
static QueueHandle_t UART_QUEUE_NUM;
#define CONFIG_UART_FIFO
static void UserUart_2_Init(void);
static int UserUart_ReadData(unsigned char *buff);
#ifdef CONFIG_UART_FIFO
static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    size_t buffered_size;
    uint8_t *dtmp = (uint8_t *)malloc(RD_BUF_SIZE);
    ESP_LOGI(TAG, "uart_event_task call");
    for (;;)
    {
        // Waiting for UART event.
        if (xQueueReceive(UART_QUEUE_NUM, (void *)&event, (TickType_t)portMAX_DELAY))
        {
            bzero(dtmp, RD_BUF_SIZE);
            ESP_LOGI(TAG, "uart[%d] event:", EX_UART_NUM);
            switch (event.type)
            {
            // Event of UART receving data
            /*We'd better handler data event fast, there would be much more data events than
            other types of events. If we take too much time on data event, the queue might
            be full.*/
            case UART_DATA:
                ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);
                ESP_LOGI(TAG, "[DATA EVT]:");
                // uart_write_bytes(EX_UART_NUM, (const char *)dtmp, event.size);
                // uart_read_callback((unsigned char *)dtmp, (unsigned short)event.size);
                break;
            // Event of HW FIFO overflow detected
            case UART_FIFO_OVF:
                ESP_LOGI(TAG, "hw fifo overflow");
                // If fifo overflow happened, you should consider adding flow control for your application.
                // The ISR has already reset the rx FIFO,
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(EX_UART_NUM);
                xQueueReset(UART_QUEUE_NUM);
                break;
            // Event of UART ring buffer full
            case UART_BUFFER_FULL:
                ESP_LOGI(TAG, "ring buffer full");
                // If buffer full happened, you should consider encreasing your buffer size
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(EX_UART_NUM);
                xQueueReset(UART_QUEUE_NUM);
                break;
            // Event of UART RX break detected
            case UART_BREAK:
                ESP_LOGI(TAG, "uart rx break");
                break;
            // Event of UART parity check error
            case UART_PARITY_ERR:
                ESP_LOGI(TAG, "uart parity error");
                break;
            // Event of UART frame error
            case UART_FRAME_ERR:
                ESP_LOGI(TAG, "uart frame error");
                break;
            // UART_PATTERN_DET
            case UART_PATTERN_DET:
                uart_get_buffered_data_len(EX_UART_NUM, &buffered_size);
                int pos = uart_pattern_pop_pos(EX_UART_NUM);
                ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                if (pos == -1)
                {
                    // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
                    // record the position. We should set a larger queue size.
                    // As an example, we directly flush the rx buffer here.
                    uart_flush_input(EX_UART_NUM);
                }
                else
                {
                    uart_read_bytes(EX_UART_NUM, dtmp, pos, 150 / portTICK_PERIOD_MS);
                    uint8_t pat[PATTERN_CHR_NUM + 1];
                    memset(pat, 0, sizeof(pat));
                    uart_read_bytes(EX_UART_NUM, pat, PATTERN_CHR_NUM, 150 / portTICK_PERIOD_MS);
                    ESP_LOGI(TAG, "read data: %s", dtmp);
                    ESP_LOGI(TAG, "read pat : %s", pat);
                }
                break;
            case UART_DATA_BREAK:
                ESP_LOGI(TAG, "UART_DATA_BREAK");
                break;
            // Others
            default:
                ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}
#else
static void uart_event_task(void *pvParameters)
{
    uint8_t data[512] = {0};
    unsigned char Rx_len = 0;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("\r\n---- Uart_Task ");
    while (1)
    {
        if ((Rx_len = UserUart_ReadData(data)) > 0)
        {
            ESP_LOGI(TAG, "---- Uart_Task = %d", Rx_len);
            // uart_read_callback((unsigned char *)data, (unsigned short)Rx_len);
            memset(data, 0x00, sizeof(data));
            Rx_len = 0;
        }
        // vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
#endif

void user_uart_init(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
#ifdef CONFIG_UART_FIFO
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // Install UART driver, and get the queue.
    uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &UART_QUEUE_NUM, 0);
    uart_param_config(EX_UART_NUM, &uart_config);

    // Set UART log level
    esp_log_level_set(TAG, ESP_LOG_INFO);
    // Set UART pins (using UART0 default pins ie no changes.)
    // uart_set_pin(EX_UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_set_pin(UART_NUM_2, USER_UART_TX, USER_UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Set uart pattern detect function.
    uart_enable_pattern_det_baud_intr(EX_UART_NUM, '+', PATTERN_CHR_NUM, 20, 0, 0);
    // Reset the pattern queue length to record at most 20 pattern positions.
    uart_pattern_queue_reset(EX_UART_NUM, 20);
#else  //
    UserUart_2_Init();
#endif //

    // Create a task to handler UART event from ISR
    xTaskCreate(uart_event_task, "uart_event_task", 4096, NULL, 3, NULL);
}

void uart_recieve_callback_init(uart_read_callback_t callback)
{
    if (callback)
    {
        uart_read_callback = callback;
    }
    else
    {
        ESP_LOGE(TAG, "uart_recieve_callback register error");
        return;
    }
}

void uart_send_callback(uint8_t *data, unsigned short length)
{
    uart_write_bytes(EX_UART_NUM, (const char *)data, length);
}

static void UserUart_2_Init(void)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
    uart_param_config(UART_NUM_2, &uart_config);
    uart_set_pin(UART_NUM_2, GPIO_NUM_17, GPIO_NUM_16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_2, 1024 * 2, BUF_SIZE * 2, 20, NULL, 0);
}

static int UserUart_ReadData(unsigned char *buff)
{
    int len = 0;
    len = uart_read_bytes(UART_NUM_2, buff, BUF_SIZE, 20 / portTICK_PERIOD_MS); // @suppress("Symbol is not resolved")
    return len;
}