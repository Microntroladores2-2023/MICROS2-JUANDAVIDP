#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "freertos/queue.h"
#include "DHT.h"
#include <Adafruit_Sensor.h>

TaskHandle_t xHandle_http_task = NULL;
TaskHandle_t xHandle_entrada_datos1 = NULL;
TaskHandle_t xHandle_entrada_datos2 = NULL;
TaskHandle_t xHandle_procesamiento_datos = NULL;

QueueHandle_t queue1, queue2;

#define DHT1_PIN 18 // Pin del sensor DHT11 1
#define DHT2_PIN 19 // Pin del sensor DHT11 2
#define DHT_TYPE DHT11 // Tipo de sensor DHT11

DHT dht1(DHT1_PIN, DHT_TYPE);
DHT dht2(DHT2_PIN, DHT_TYPE);

//**************************************
//*************** TASKs ****************
//**************************************

void TaskEntradaDatos1(void* pvParameters) {

  float txBuffer[2];
  queue1 = xQueueCreate(5, sizeof(txBuffer));
  if (queue1 == 0) {
    printf("Failed to create queue= %p\n", queue1);
  }

  while (1) {
    float temperature1 = dht1.readTemperature();
    float humidity1 = dht1.readHumidity();

    txBuffer[0] = temperature1;
    txBuffer[1] = humidity1;

    xQueueSend(queue1, (void*)txBuffer, (TickType_t)0);
    vTaskDelay(500 / portTICK_PERIOD_MS); // Delay para tomar una nueva lectura cada 0.5 segundos
  }
}

void TaskEntradaDatos2(void* pvParameters) {

  float txBuffer[2];
  queue2 = xQueueCreate(5, sizeof(txBuffer));
  if (queue2 == 0) {
    printf("Failed to create queue= %p\n", queue2);
  }

  while (1) {
    float temperature2 = dht2.readTemperature();
    float humidity2 = dht2.readHumidity();

    txBuffer[0] = temperature2;
    txBuffer[1] = humidity2;

    xQueueSend(queue2, (void*)txBuffer, (TickType_t)0);
    vTaskDelay(500 / portTICK_PERIOD_MS); // Delay para tomar una nueva lectura cada 0.5 segundos
  }
}

void TaskProcesamientoDatos(void* pvParameters) {

  // Scada Vemetris en Digital Ocean
  String ScadaVemetris = "http://137.184.178.17:21486/httpds?__device=MonitorJuan";

  float rxBuffer1[2], rxBuffer2[2];

  while (1) {

    if (xQueueReceive(queue1, &(rxBuffer1), (TickType_t)5) == pdTRUE && xQueueReceive(queue2, &(rxBuffer2), (TickType_t)5) == pdTRUE) {

      HTTPClient http;

      String dato1 = String(rxBuffer1[0]);

      String dato2 = String(rxBuffer1[1]);

      String dato3 = String(rxBuffer2[0]);

      String dato4 = String(rxBuffer2[1]);

      String Trama = ScadaVemetris + "&rssi=" + WiFi.RSSI() + "&dato1=" + dato1 + "&dato2=" + dato2 + "&dato3=" + dato3 + "&dato4=" + dato4;

      Serial.println(Trama);
      http.begin(Trama);          //Iniciar conexión
      int httpCode = http.GET();  // Realizar petición
      if (httpCode > 0) {
        String payload = http.getString();  // Obtener respuesta
        Serial.println(httpCode);           // Si el codigo es 200, se realizo bien
        Serial.println(payload);            // Mostrar respuesta por serial
      } else {
        Serial.println("Error enviando la trama");
      }
      http.end();  // Se libera el cliente


      vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay para enviar una nueva trama cada 5 segundos
    }
  }
}


void initWiFi(void) {

  const char* ssid = "JuanD";
  const char* password = "ingucv2207";

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.println("Conectando a la red WiFi...");
  }
  Serial.println("Conectado ala red WiFi!");
}


void setup() {

  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  initWiFi();

  dht1.begin();
  dht2.begin();

  xTaskCreatePinnedToCore(TaskEntradaDatos1, "EntradaDatos1", 4096, NULL, 2, &xHandle_entrada_datos1, 1);
  xTaskCreatePinnedToCore(TaskEntradaDatos2, "EntradaDatos2", 4096, NULL, 2, &xHandle_entrada_datos2, 1);
  xTaskCreatePinnedToCore(TaskProcesamientoDatos, "ProcesamientoDatos", 4096, NULL, 4, &xHandle_procesamiento_datos, 1);
}

void loop() {
}