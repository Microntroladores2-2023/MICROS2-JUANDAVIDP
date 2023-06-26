#include <Arduino.h>

// Define el tamaño maximo de la cola
#define QUEUE_SIZE 5

// Crear la cola

QueueHandle_t cola; 

// Tarea Productora 

void tareaProductora(void *parameter) {

   for (int i = 0; i < 30; i++ ) {

    // Enviar datos a la cola
    xQueueSend(cola, &i, portMAX_DELAY);
    //vTaskDelay(pdMS_TO_TICKS(1000)); // Retardo de 1 segundo
   }
   vTaskDelete(NULL);
}

// Tarea consumidora 

void tareaConsumidora(void *parameter) {
  int valor;
  while(1) {
    // Recibir datos de la cola
    if (xQueueReceive(cola, &valor, portMAX_DELAY)) {
      Serial.print("Valor recibido: ");
      Serial.println(valor);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Crear la cola con QUEUE_SIZE elementos, cada uno de tamaño sizeof(int)
  cola = xQueueCreate(QUEUE_SIZE , sizeof(int));

  // Crear las tareas
  xTaskCreate(tareaProductora,"Productor", 10000, NULL, 1, NULL);
  xTaskCreate(tareaConsumidora,"Consumidor", 10000, NULL, 1, NULL);

}

void loop() {
  // No hacer nada en el bucle principal
}