#include <Arduino.h>

void Blink( void *pvParameters)
{
  #define LED_PIN GPIO_NUM_2 

  gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
  
  int ON = 0;

  while (true)
  {
    ON = !ON;
    gpio_set_level(LED_PIN, ON);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup() {

  xTaskCreatePinnedToCore(Blink, "Parpadeo",1024 * 2, NULL, 1, NULL, 0);
}
void loop(){

}