#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"

static QueueHandle_t uart0_queue;

void protocolo1Serial(uint8_t *ByteArray, uint16_t Length);

#define tamBUFFER 1024

//**************************************
//*************** TASKs ****************
//**************************************

void TareaEventosUART0(void *Parametro)
{
    uart_event_t evento;

    uint8_t *datoRX = (uint8_t *)malloc(tamBUFFER);

    for (;;)
    {
        if (xQueueReceive(uart0_queue, (void *)&evento, (TickType_t)portMAX_DELAY))
        {
            bzero(datoRX, tamBUFFER);
            if (evento.type == UART_DATA)
            {
                uart_read_bytes(UART_NUM_0, datoRX, evento.size, portMAX_DELAY);
                // modbusSerial(datoRX, evento.size);

                //protocolo1Serial(datoRX, evento.size);

                uart_write_bytes(UART_NUM_0, (const char*) datoRX, evento.size);

                // vTaskDelay(10 / portTICK_PERIOD_MS);
            }
        }
    }

    free(datoRX);
    datoRX = NULL;
    vTaskDelete(NULL);
}

//**************************************
//************* Init UARTs *************
//**************************************

// Función Para iniciar el UART0
void initUART0()
{
    uart_config_t configUART0 = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_param_config(UART_NUM_0, &configUART0);
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_0, tamBUFFER * 2, tamBUFFER * 2, 20, &uart0_queue, 0);

    xTaskCreatePinnedToCore(TareaEventosUART0, "Tarea_para_UART0", 1024 * 5, NULL, 12, NULL, 1);
}

void app_main()
{
    initUART0();
}

//**************************************
//************* Funciones **************
//**************************************

void protocolo1Serial(uint8_t *ByteArray, uint16_t Length)
{

    //uart_write_bytes(UART_NUM_0, (const char*) ByteArray, Length);


    uint8_t estado = ByteArray[0]; // maquina de estado

    switch (estado)
    {
    case 0:

        break;

    case 1:

        break;
    case 2:

        break;

    case 3:

        break;
    }
}