// PROYECTO FINAL MICROS 2 JUAN PEÑA //
#include <Arduino.h>
#include "SPIFFS.h"
#include <WiFiClientSecure.h>
#include <Wire.h>
#include "freertos/queue.h"
#include "freertos/task.h"
#include <PubSubClient.h>
#include "DHT.h"
#include <Adafruit_Sensor.h>

// Credenciales de red Wifi
const char *ssid = "JuanD";
const char *password = "ingucv2207";

// Servidor MQTT
//Se definen los detalles del servidor MQTT al que el dispositivo se conectará, incluyendo la dirección del servidor y el número de puerto.
const char *mqtt_server = "a3s0rr8g20kk3a-ats.iot.us-east-2.amazonaws.com";
const int mqtt_port = 8883;
const int ADC_PIN = 32 ;
//Se definen variables de tipo String que se utilizarán para almacenar 
//el contenido de los archivos de certificados (rootCA, cert y private key)
// almacenados en la memoria SPIFFS del ESP32
String Read_rootca;
String Read_cert;
String Read_privatekey;
//********************************
#define BUFFER_LEN 256
long lastMsg = 0;  //es una variable que guarda el tiempo del último mensaje publicado
char msg[BUFFER_LEN];  // define el tamaño del búfer para almacenar los mensajes MQTT
int value = 0;
byte mac[6];
char mac_Id[18];
int count = 1;
int lectura;
//********************************

// Configuración de cliente MQTT
//Se crean objetos WiFiClientSecure y PubSubClient que se utilizarán para la conexión segura a través de TLS/SSL con el servidor MQTT.
WiFiClientSecure espClient;
PubSubClient client(espClient);

// Configuración de DHTXX
#define DHT1_PIN 18 // Pin del sensor DHT11 1
#define DHT2_PIN 19 // Pin del sensor DHT11 2
#define DHT_TYPE DHT11
#define samples 128
DHT dht1(DHT1_PIN, DHT11); // Modelo
DHT dht2(DHT2_PIN, DHT11); // Modelo

// Conectar a red Wifi
void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Conectando.. ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());
}

// Callback
//Esta función es el callback que se ejecuta cuando llega un mensaje desde el servidor MQTT al que el cliente se ha suscrito
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Conectar a broker MQTT
// Código para intentar la conexión al servidor MQTT 
void reconnect()
{

  // Loop para reconección
  //implementa un bucle de reconexión al servidor MQTT en caso de que la conexión inicial falle o se pierda
  while (!client.connected())
  {
    Serial.print("Estableciendo conexion MQTT ..."); 

    // Creando un ID como ramdon
    String clientId = "ESP32-";
    clientId += String(random(0xffff), HEX);

    // Intentando conectarse
    if (client.connect(clientId.c_str()))  //intenta establecer una conexión con el servidor MQTT utilizando el ID de cliente generado
    {
      Serial.println("conectada");

      // Conectado, publicando un payload...
      client.publish("ei_out", "hello world");

      // ... y suscribiendo
      client.subscribe("ei_in");
    }
    else
    {
      Serial.print("failed, rc=");   //Si falla la conexion.Muestra un mensaje 
                                     //en la interfaz de monitoreo serial para indicar que la conexión falló
      Serial.print(client.state());
      Serial.println(" Esperando 5 segundos");  //esperará 5 segundos antes de intentar nuevamente la conexión.

      // Tiempo muerto de 5 segundos
      delay(5000);
    }
  }
}

// Colas para comunicarse con las tareas
QueueHandle_t temperature1Queue;
QueueHandle_t humidity1Queue;
QueueHandle_t temperature2Queue;
QueueHandle_t humidity2Queue;
QueueHandle_t adcValueQueue;

// Tareas para leer sensores y ADC
//se encarga de leer el sensor DHT11 1 y enviar los datos a la cola correspondiente.
void taskReadDHT1(void *pvParameters)
{
  for (;;)
  {
    float temperature1 = dht1.readTemperature();
    float humidity1 = dht1.readHumidity();
    xQueueSend(temperature1Queue, &temperature1, portMAX_DELAY);
    xQueueSend(humidity1Queue, &humidity1, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(5000)); // Leer cada 5 segundos
  }
}
//se encarga de leer el sensor DHT11 2 y enviar los datos a la cola correspondiente.
void taskReadDHT2(void *pvParameters)
{
  for (;;)
  {
    float temperature2 = dht2.readTemperature();
    float humidity2 = dht2.readHumidity();
    xQueueSend(temperature2Queue, &temperature2, portMAX_DELAY);
    xQueueSend(humidity2Queue, &humidity2, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(5000)); // Leer cada 5 segundos
  }
}
//Esta función es otra tarea que se ejecutará en un núcleo del ESP32 y se encarga de leer el ADC y enviar los datos a la cola correspondiente.
void taskReadADC(void *pvParameters)
{
  for (;;)
  {
    int val_pot = 0;
    int smooth_val = 0;
    for (int i = 0; i < samples; i++)
    {
      smooth_val += val_pot = analogRead(ADC_PIN); // Lee el valor del ADC en el pin ADC_PIN
    }
    smooth_val /= samples;
    if (smooth_val > 4095)
    {
      smooth_val = 4095;
    }
    int adcValue = smooth_val;
    xQueueSend(adcValueQueue, &adcValue, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(5000)); // Leer cada 5 segundos
  }
}

void setup()
{
  Serial.begin(115200);  //Inicia la comunicación serial
  dht1.begin();   //Inicializa los objetos dht1 y dht2
  dht2.begin();
  //pinMode(ADC_PIN, INPUT);

  Serial.setDebugOutput(true);  //permite enviar mensajes de depuración a través del puerto serie

  // Inicializa con el PIN led2.
  pinMode(2, OUTPUT);
  setup_wifi();
  delay(1000);

  //****************
  if (!SPIFFS.begin(true))
  {
    Serial.println("Se ha producido un error al montar SPIFFS");
    return;
  }
  //**********************
  // Root CA leer archivo.
  File file2 = SPIFFS.open("/AmazonRootCA1.pem", "r"); // Contiene el certificado de la Autoridad de Certificación Raíz de Amazon Web Services (AWS).
  if (!file2)
  {
    Serial.println("No se pudo abrir el archivo para leerlo");
    return;
  }
  Serial.println("Root CA File Content:");
  while (file2.available())
  {
    Read_rootca = file2.readString();
    Serial.println(Read_rootca);
  }
  //*****************************
  // Cert leer archivo
  File file4 = SPIFFS.open("/69803ea3-certificate.pem.crt", "r"); //Contiene el certificado del cliente MQTT utilizado para la autenticación en el servidor AWS IoT.
  if (!file4)
  {
    Serial.println("No se pudo abrir el archivo para leerlo");
    return;
  }
  Serial.println("Cert File Content:");
  while (file4.available())
  {
    Read_cert = file4.readString();
    Serial.println(Read_cert);
  }

  
  //***************************************
  // Privatekey leer archivo
  File file6 = SPIFFS.open("/69803ea3-private.pem.key", "r");  //Contiene la clave privada asociada al certificado del cliente MQTT.
  if (!file6)
  {
    Serial.println("No se pudo abrir el archivo para leerlo");
    return;
  }
  Serial.println("privateKey contenido:");
  while (file6.available())
  {
    Read_privatekey = file6.readString();
    Serial.println(Read_privatekey);
  }
  //=====================================================
  //Cada archivo se lee y se almacena en las variables Read_rootca, Read_cert y Read_privatekey, que son de tipo String.

  //se convierten las variables Read_rootca, Read_cert y Read_privatekey 
  //de tipo String a arreglos de caracteres (cadenas C) utilizando malloc y strcpy
  char *pRead_rootca;
  pRead_rootca = (char *)malloc(sizeof(char) * (Read_rootca.length() + 1));
  strcpy(pRead_rootca, Read_rootca.c_str());

  char *pRead_cert;
  pRead_cert = (char *)malloc(sizeof(char) * (Read_cert.length() + 1));
  strcpy(pRead_cert, Read_cert.c_str());

  char *pRead_privatekey;
  pRead_privatekey = (char *)malloc(sizeof(char) * (Read_privatekey.length() + 1));
  strcpy(pRead_privatekey, Read_privatekey.c_str());

  //se muestra por el puerto serial el contenido de cada certificado y clave privada que se leerá para verificar su correcta lectura.

  Serial.println("================================================================================================");
  Serial.println("Certificados que pasan adjuntan al espClient");
  Serial.println();
  Serial.println("Root CA:");
  Serial.write(pRead_rootca);
  Serial.println("================================================================================================");
  Serial.println();
  Serial.println("Cert:");
  Serial.write(pRead_cert);
  Serial.println("================================================================================================");
  Serial.println();
  Serial.println("privateKey:");
  Serial.write(pRead_privatekey);
  Serial.println("================================================================================================");
  //se configuran los certificados y la clave privada en el objeto espClient (cliente seguro de Wi-Fi) 
  //para que se utilicen en la conexión segura MQTT.
  espClient.setCACert(pRead_rootca);
  espClient.setCertificate(pRead_cert);
  espClient.setPrivateKey(pRead_privatekey);

  //se configuran el cliente MQTT (client) 
  //se definen las funciones de callback para recibir mensajes MQTT.
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  //******************************************
  WiFi.macAddress(mac);
  snprintf(mac_Id, sizeof(mac_Id), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println(mac_Id);
  //****************************************
  delay(2000);
  //se utiliza para obtener la dirección MAC (Media Access Control) del módulo ESP32, 
  //que es una identificación única de 48 bits asignada a cada dispositivo de red. 
  //La dirección MAC se utiliza para identificar de manera única el ESP32

  // Crear colas
  temperature1Queue = xQueueCreate(1, sizeof(float));
  humidity1Queue = xQueueCreate(1, sizeof(float));
  temperature2Queue = xQueueCreate(1, sizeof(float));
  humidity2Queue = xQueueCreate(1, sizeof(float));
  adcValueQueue = xQueueCreate(1, sizeof(int));

  // Crear tareas
  xTaskCreatePinnedToCore(taskReadDHT1, "ReadDHT1Task", 4096, NULL, 1, NULL, 0); // Tarea para sensor DHT1 en el core 0
  xTaskCreatePinnedToCore(taskReadDHT2, "ReadDHT2Task", 4096, NULL, 1, NULL, 0); // Tarea para sensor DHT2 en el core 0
  xTaskCreatePinnedToCore(taskReadADC, "ReadADCTask", 4096, NULL, 1, NULL, 0);   // Tarea para ADC en el core 0
}

void loop()
{
  // ******************** SENSOR DHTxx *********************************
 
  if (!client.connected())  // verifica si el cliente MQTT (client) está desconectado
  {
    reconnect();   //Si el cliente MQTT no está conectado, se llama a la función reconnect()
  }
  client.loop();  //realizar un intento de reconexión

  // Obtener valores de las colas y publicarlos en el servidor MQTT
  
    float temperature1, humidity1, temperature2, humidity2;
    int adcValue;
  //Recepción de datos de las colas: Se utiliza la función xQueueReceive() para obtener los valores de las colas
  if (xQueueReceive(temperature1Queue, &temperature1, portMAX_DELAY) &&
      xQueueReceive(humidity1Queue, &humidity1, portMAX_DELAY) &&
      xQueueReceive(temperature2Queue, &temperature2, portMAX_DELAY) &&
      xQueueReceive(humidity2Queue, &humidity2, portMAX_DELAY) &&
      xQueueReceive(adcValueQueue, &adcValue, portMAX_DELAY))
  { 
   
   // verifica si han pasado al menos 500 milisegundos desde la última publicación
   long now = millis();
   if (now - lastMsg > 500){ 
    lastMsg = now;

    // Lectura del valor analógico del ADC
    int adcValue1 = analogRead(ADC_PIN);
    // Lectura del valor Promedio analógico del ADC
    int adcprom = adcValue;
    //Se utiliza la función snprintf() para formatear el mensaje que se enviará al servidor MQTT
    String macIdStr = mac_Id;
    String Temprature1 = String(temperature1);
    String Humidity1 = String(humidity1);
    String Temprature2 = String(temperature2) ;
    String Humidity2 = String(humidity2);
    snprintf(msg, BUFFER_LEN, "{\"mac_Id\" : \"%s\", \"Temperatura1\" : %s, \"Humedad1 \" : %s, \"Temperatura2 \" : %s, \"Humedad2 \" : %s, \"ADCprom\" : %d, \"ADC\" : %d}", macIdStr.c_str(), Temprature1.c_str(), Humidity1.c_str(), Temprature2.c_str(), Humidity2.c_str(), adcprom, adcValue1);
    //El mensaje se almacena en el arreglo de caracteres msg con el formato JSON.
    Serial.print(count);
    Serial.println(msg);
    client.publish("sensor", msg); //El mensaje formateado se publica en el tópico "sensor" del servidor MQTT 
                                   //utilizando la función client.publish()
    count = count + 1;
    //================================================================================================
   }
  }  
  digitalWrite(2, HIGH);  //establece el pin digital 2 en HIGH
  delay(1000);
  digitalWrite(2, LOW);    //establece el pin digital 2 en LOW
  delay(1000);
}
