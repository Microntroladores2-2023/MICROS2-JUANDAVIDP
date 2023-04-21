//#include <stdio.h>
//#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"


void TaskADC1(void *pvParameters) // Esta es una tarea
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);// GPIO 32

    while (1)
    {
        int16_t adc_value = adc1_get_raw(ADC1_CHANNEL_4); 
        printf("AD 4 (data cruda): %d\n\r", adc_value);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main()
{

    xTaskCreatePinnedToCore(TaskADC1, "Task1", 1024 * 2, NULL, 1, NULL, 0);
}