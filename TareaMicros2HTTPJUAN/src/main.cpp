#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "freertos/queue.h"
#include "DHT.h"
#include <Adafruit_Sensor.h>

TaskHandle_t xHandle_http_task = NULL;
TaskHandle_t xHandle_entrada_datos = NULL;

QueueHandle_t queue;

#define DHT1_PIN 18 // Pin del sensor DHT11 1
#define DHT2_PIN 19 // Pin del sensor DHT11 2
#define DHT_TYPE DHT11 // Tipo de sensor DHT11

DHT dht1(DHT1_PIN, DHT_TYPE);
DHT dht2(DHT2_PIN, DHT_TYPE);

//**************************************
//*************** TASKs ****************
//**************************************

void TaskEntradaDatos(void* pvParameters) {

  float txBuffer[10];
  queue = xQueueCreate(5, sizeof(txBuffer));
  if (queue == 0) {
    printf("Failed to create queue= %p\n", queue);
  }

  while (1) {
    float temperature1 = dht1.readTemperature();
    float humidity1 = dht1.readHumidity();
    float temperature2 = dht2.readTemperature();
    float humidity2 = dht2.readHumidity();

    txBuffer[0] = temperature1;
    txBuffer[1] = humidity1;
    txBuffer[2] = temperature2;
    txBuffer[3] = humidity2;

    xQueueSend(queue, (void*)txBuffer, (TickType_t)0);
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay para tomar una nueva lectura cada 2 segundos
  }
}

void TaskHTTP(void* pvParameters) {

  // Scada Vemetris en Digital Ocean
  String ScadaVemetris = "http://137.184.178.17:21486/httpds?__device=MonitorJuan";

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
        Serial.println("Error enviando la trama");
      }
      http.end();  // Se libera el cliente


      vTaskDelay(5000 / portTICK_PERIOD_MS); // Delay para enviar una nueva trama cada 5 segundos
    }
  }
}


void initWiFi(void) {

  const char* ssid = "JuanD";
  const char* password = "ingucv2207";

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

  dht1.begin();
  dht2.begin();

  xTaskCreatePinnedToCore(TaskEntradaDatos, "EntradaDatos", 4096, NULL, 2, &xHandle_entrada_datos, 1);

  xTaskCreatePinnedToCore(TaskHTTP, "HTTPcliente", 4096, NULL, 4, &xHandle_http_task, 1);
}

void loop() {
}