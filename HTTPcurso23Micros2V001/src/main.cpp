#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "freertos/queue.h"

TaskHandle_t xHandle_http_task = NULL;
TaskHandle_t xHandle_entrada_datos = NULL;

QueueHandle_t queue;


//**************************************
//*************** TASKs ****************
//**************************************

void TaskEntradaDatos(void* pvParameters) {

  float txBuffer[10];
  queue = xQueueCreate(5, sizeof(txBuffer));
  if (queue == 0) {
    printf("Failed to create queue= %p\n", queue);
  }
  txBuffer[2] = -20;
  txBuffer[3] = -30;

  while (1) {
    txBuffer[0] = random(100);
    txBuffer[1] = random(100);
    txBuffer[2]++;
    txBuffer[3]++;

    if (txBuffer[2] == 20) {
      txBuffer[2] = -20;
      txBuffer[3] = -30;
    }

    xQueueSend(queue, (void*)txBuffer, (TickType_t)0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void TaskHTTP(void* pvParameters) {

  // Scada Vemetris en Digital Ocean
  String ScadaVemetris = "http://137.184.178.17:21486/httpds?__device=TestJose";

  float rxBuffer[10];

  while (1) {

    if (xQueueReceive(queue, &(rxBuffer), (TickType_t)5)) {
      
      HTTPClient http;

      String dato1 = String(rxBuffer[0]);

      String dato2 = String(rxBuffer[1]);

      String dato3 = String(rxBuffer[2]);

      String dato4 = String(rxBuffer[3]);

      String Trama = ScadaVemetris + "&rssi=" + WiFi.RSSI() + "&dato1=" + dato1 + "&dato2=" + dato2 + "&dato3=" + dato3 + "&dato4=" + dato4;

      Serial.println(Trama);
      http.begin(Trama);          //Iniciar conexión
      int httpCode = http.GET();  // Realizar petición
      if (httpCode > 0) {
        String payload = http.getString();  // Obtener respuesta
        Serial.println(httpCode);           // Si el codigo es 200, se realizo bien
        Serial.println(payload);            // Mostrar respuesta por serial
      } else {
        Serial.println("Error enviadno la trama");
      }
      http.end();  // Se libera el cliente


      vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
  }
}


void initWiFi(void) {

  const char* ssid = "HOLETB 2.4_EXT";
  const char* password = "primitiva";

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.println("Configurando Red Wi-Fi");
  }
}


void setup() {

  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  initWiFi();

  xTaskCreatePinnedToCore(TaskEntradaDatos, "EntradaDatos", 4096, NULL, 2, &xHandle_entrada_datos, 1);

  xTaskCreatePinnedToCore(TaskHTTP, "HTTPcliente", 4096, NULL, 4, &xHandle_http_task, 1);
}

void loop() {
}
