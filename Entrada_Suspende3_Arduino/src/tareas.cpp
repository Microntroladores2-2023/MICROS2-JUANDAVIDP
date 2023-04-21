#include "tareas.h"

extern TaskHandle_t xHandle;

uint16_t tiempo_Led = 1000; // variable global

void Boton(void *pvParameters)
{
#define PUSH_BUTTON_PIN (gpio_num_t)0

  enum ESTADOS{
    velocidad_lenta,
    velocidad_rapida,
    suspensio_BLINK,
    resumen_BLINK,
    velocidad_media
  };
  esp_task_wdt_add(NULL);
  uint8_t estado_maquina = velocidad_rapida;

  gpio_set_direction(PUSH_BUTTON_PIN, GPIO_MODE_INPUT);

  while (1)
  {
    // presiona boton y suelta
    while (gpio_get_level(PUSH_BUTTON_PIN) == 0){
      vTaskDelay(10 / portTICK_PERIOD_MS); // boton presionado
      esp_task_wdt_reset();
    }
    while (gpio_get_level(PUSH_BUTTON_PIN) == 1){
      vTaskDelay(10 / portTICK_PERIOD_MS); // boton sin presionar
      esp_task_wdt_reset();
    }
    switch (estado_maquina)
    {
    case velocidad_rapida:
      tiempo_Led = 100;
      estado_maquina = velocidad_lenta;
      break;

    case velocidad_lenta:
      tiempo_Led = 2000;
      estado_maquina = suspensio_BLINK;
      break;

    case suspensio_BLINK:
      vTaskSuspend(xHandle);
      estado_maquina = resumen_BLINK;
      break;

    case resumen_BLINK:
      vTaskResume(xHandle);
      tiempo_Led = 50;
      estado_maquina = velocidad_media;
      break;

    case velocidad_media:
      tiempo_Led = 500;
      estado_maquina = velocidad_rapida;
      break;
    }
    esp_task_wdt_reset();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void Blink(void *pvParameters) // Esta es una tarea
{

#define LED_PIN (gpio_num_t)2

  gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

  int ON = 0;

  while (true) // Una tarea nunca regresará ni saldrá
  {
    ON = !ON;
    gpio_set_level(LED_PIN, ON);
    vTaskDelay(tiempo_Led / portTICK_PERIOD_MS);
  }
}
