#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/uart.h"

static const char *tag = "UART";
#define LedR 18
#define LedG 19
#define LedB 21

#define UART_NUM UART_NUM_0
#define BUF_SIZE 1024
#define TASK_MEMORY 1024 * 2

static void uart_task(void *pvParameters)
{
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    while (1)
    {
        bzero(data, BUF_SIZE);

        int len = uart_read_bytes(UART_NUM, data, BUF_SIZE, pdMS_TO_TICKS(100));

        if (len == 0)
        {
            continue;
        }

        uart_write_bytes(UART_NUM, (const char *)data, len);

        //ESP_LOGI(tag, "Data received: %s", data);

        for (size_t i = 0; i < len - 2; i++)
        {
            char value = data[i];

            switch (value)
            {
            case 'R':
                gpio_set_level(LedR, 1);
                gpio_set_level(LedG, 0);
                gpio_set_level(LedB, 0);
                break;
            case 'G':
                gpio_set_level(LedR, 0);
                gpio_set_level(LedG, 1);
                gpio_set_level(LedB, 0);
                break;
            case 'B':
                gpio_set_level(LedR, 0);
                gpio_set_level(LedG, 0);
                gpio_set_level(LedB, 1);
                break;
            default:
                gpio_set_level(LedR, 0);
                gpio_set_level(LedG, 0);
                gpio_set_level(LedB, 0);
                
                break;
            }
        }
    }
}

static void init_uart(void)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_param_config(UART_NUM, &uart_config);

    uart_set_pin(UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_driver_install(UART_NUM, BUF_SIZE , BUF_SIZE , 0, NULL, 0);

    xTaskCreate(uart_task, "uart_task", TASK_MEMORY, NULL, 5, NULL);

    ESP_LOGI(tag, "init uart completed!");
}

static void init_led(void)
{
    gpio_reset_pin(LedR);
    gpio_set_direction(LedR, GPIO_MODE_OUTPUT);
    gpio_reset_pin(LedG);
    gpio_set_direction(LedG, GPIO_MODE_OUTPUT);
    gpio_reset_pin(LedB);
    gpio_set_direction(LedB, GPIO_MODE_OUTPUT);
    ESP_LOGI(tag, "init led completed!");
}

void app_main(void)
{
    init_led();
    init_uart();
}
