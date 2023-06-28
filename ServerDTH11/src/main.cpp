
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <WiFi.h>
#include <DHT.h>

void TaskServer(void *Parametro);
void InitWiFi(void);

// Definir los pines de los sensores DHT11
#define DHTPIN1 18
#define DHTPIN2 19
#define LEDPIN1 2
#define LEDPIN2 22
// Definir el tipo de sensor DHT
#define DHTTYPE DHT11
DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);

void setup()
{
  Serial.begin(115200);

  // Definir el pin del LED como una salida
  pinMode(LEDPIN1, OUTPUT);
  pinMode(LEDPIN2, OUTPUT);

  InitWiFi();
  xTaskCreatePinnedToCore(TaskServer, "TaskServer", 1024 * 4, NULL, 2, NULL, 0);
}

void loop() {}

//**************************************
//*************** TASKs ****************
//**************************************

void TaskServer(void *Parametro)
{
  // Socket = IP Address + Network Port Number
  // Create a TCP server socket on port 502
  int puerto = 502;
  WiFiServer server(puerto);

  // Start listening for incoming connections
  server.begin();

  while (1)
  {
    // Check if there are any incoming connections
    WiFiClient client = server.available();

    if (client)
    {
      // There is an incoming connection
      Serial.println("New client connected!");

      // Encender el LED cuando un cliente se conecta
      digitalWrite(LEDPIN2, HIGH);

      // Print the client's IP address
      Serial.println(client.remoteIP());

      // Send a message to the client
      client.println("Hola, cliente de ESP32 Server");

      // Wait for the client to close the connection
      while (client.connected())
      {
        // Check if there is data available from the client
        if (client.available())
        {
          // Read the data from the client
          String data = client.readStringUntil('\r');

          // If the client sends "stop" command, break out of loop
          if (data.indexOf("stop") != -1)
          {
            break;
          }
        }

        // Leer los valores de temperatura y humedad de los sensores DHT11
        float temperature1 = dht1.readTemperature();
        float humidity1 = dht1.readHumidity();
        float temperature2 = dht2.readTemperature();
        float humidity2 = dht2.readHumidity();

        // Crear una cadena de respuesta con los valores de temperatura y humedad
        String response = "Sensor 1: Temp=" + String(temperature1) + "*C Hum=" + String(humidity1) + "%\t    ";
        response += "Sensor 2: Temp=" + String(temperature2) + "*C Hum=" + String(humidity2) + "%\t";

        // Enviar la cadena de respuesta al cliente
        client.println(response);

        vTaskDelay(2000 / portTICK_PERIOD_MS);
      }

      // Close the client connection
      client.stop();

      // Apagar el LED cuando el cliente se desconecta
      digitalWrite(LEDPIN2, LOW);

      Serial.println("Client disconnected!");
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

//**************************************
//************* Funciones **************
//**************************************
void InitWiFi(void)
{

  const char *ssid = "JuanD";
  const char *password = "ingucv2207";

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    // Encender el LED mientras se conecta a la red WiFi
    digitalWrite(LEDPIN1, HIGH);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    digitalWrite(LEDPIN1, LOW);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Encender el LED cuando se establece la conexión WiFi
  digitalWrite(LEDPIN1, HIGH);

  // Inicializar los sensores DHT11
  dht1.begin();
  dht2.begin();
}