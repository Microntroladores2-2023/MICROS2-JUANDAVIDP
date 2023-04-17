#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t xHandle;
void Boton(void *pvParameters)
{
#define PUSH_BUTTON_PIN 0

int cont = 0;

gpio_set_direction(PUSH_BUTTON_PIN, GPIO_MODE_INPUT);

  while (1)
  {
    if(gpio_get_level(PUSH_BUTTON_PIN) == 1)
    {
        if(cont > 0) cont -- ;
        if(cont == 30) vTaskResume( xHandle );
    }
    else
    {
        if(cont == 0)
        {
            vTaskSuspend(xHandle);
            cont = 50;
        }
    }
     vTaskDelay(100);
  }
}
void Blink(void *pvParameters)
{
    #define LED_PIN 2

      gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

      int ON = 0;

      while(true)
      {
        ON=!ON;
        gpio_set_level(LED_PIN, ON);
        vTaskDelay(500 / portTICK_PERIOD_MS);

      }
}
void app_main() 
{
   xTaskCreatePinnedToCore(Boton, "Boton", 1024 * 2, NULL, 1, NULL, 0);
   xTaskCreatePinnedToCore(Blink, "Parpadeo", 1024 * 2, NULL, 1, &xHandle, 0);
}