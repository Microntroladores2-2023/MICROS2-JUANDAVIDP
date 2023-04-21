

#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

static esp_adc_cal_characteristics_t adc1_chars;

void Tarea1(void *pvParameters) // Esta es una tarea
{
#define numeroMuestras 128

    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);

    adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);

    int muestras[numeroMuestras], promedio;
    int i = 0;

    int adc_value = adc1_get_raw(ADC1_CHANNEL_4);
    for (int j = 0; j < numeroMuestras; j++)
        muestras[j] = adc_value;

    uint32_t voltage;

    while (1)
    {
        adc_value = adc1_get_raw(ADC1_CHANNEL_4);

        printf("ADC Value raw: %d \n", adc_value);
        muestras[i++] = adc_value;
        promedio = 0;
        for (int j = 0; j < numeroMuestras; j++)
            promedio = promedio + muestras[j];

        printf("ADC Value promedio: %d \n", promedio / numeroMuestras);

        if (i >= numeroMuestras)
            i = 0;

        voltage = esp_adc_cal_raw_to_voltage(adc_value, &adc1_chars);
        printf("Voltage: %d mV",(int)voltage);
        printf("\n");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
void app_main() {

 xTaskCreatePinnedToCore(Tarea1, "Task1", 1024 * 10, NULL, 1, NULL, 0);

}