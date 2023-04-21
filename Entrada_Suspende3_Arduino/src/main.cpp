#include "tareas.h"
#define WDT_TIMEOUT 3000
TaskHandle_t xHandle;



void setup()
{
esp_task_wdt_init(WDT_TIMEOUT, true);

  xTaskCreatePinnedToCore(Boton, "Boton", 1024 * 2, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(Blink, "Parpadeo", 1024 * 2, NULL, 1, &xHandle, 0);
}

void loop()
{
}