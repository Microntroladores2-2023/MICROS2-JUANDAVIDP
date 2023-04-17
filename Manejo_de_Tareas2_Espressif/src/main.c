#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t xHandle; 

uint16_t tiempo_Led = 1000; // variable global

void Boton(void *pvParameters)
{
#define PUSH_BUTTON_PIN 0

    uint8_t estado_maquina = 0;

    gpio_set_direction(PUSH_BUTTON_PIN, GPIO_MODE_INPUT);

    while (1)
    {
        // presiona boton y suelta
        while (gpio_get_level(PUSH_BUTTON_PIN) == 0)
        vTaskDelay(10/portTICK_PERIOD_MS); // boton presionado

        while (gpio_get_level(PUSH_BUTTON_PIN) == 1)
           vTaskDelay(10/portTICK_PERIOD_MS); // boton sin presionar

        switch (estado_maquina)
        {
        case 0:
            tiempo_Led = 100;
            estado_maquina = 1;
            break;

        case 1:
            tiempo_Led = 2000;
            estado_maquina = 2;
            break;

        case 2:
            vTaskSuspend(xHandle);
            estado_maquina = 3;
            break;

        case 3:
            vTaskResume(xHandle);
            tiempo_Led = 50;
            estado_maquina = 4;
            break;

        case 4:
            tiempo_Led = 500;
            estado_maquina = 0;
            break;

        }

        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

void Blink(void *pvParameters) // Esta es una tarea
{

#define LED_PIN 2

    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    int ON = 0;

    while (true) // Una tarea nunca regresará ni saldrá
    {
        ON = !ON;
        gpio_set_level(LED_PIN, ON);
        vTaskDelay(tiempo_Led / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{

    xTaskCreatePinnedToCore(Boton, "Boton", 1024 * 2, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(Blink, "Parpadeo", 1024 * 2, NULL, 1, &xHandle, 0);
}