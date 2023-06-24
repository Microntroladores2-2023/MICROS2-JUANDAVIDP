#include <Arduino.h>
#include <WiFi.h>

void TaskServer(void *Parametro);
void InitWiFi(void);

void setup()
{
  Serial.begin(115200);

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

      // Print the client's IP address
      Serial.println(client.remoteIP());

      // Send a message to the client
      client.println("Hola, cliente de ESP32 Server");

      // Wait for the client to close the connection
      while (client.connected())
      {
        if (client.available())
        {
          // Read data from the client
          String data = client.readString();

          // Print the data to the serial monitor
          Serial.println(data);

          // Send a response to the client with a newline character
          client.println("\n");
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }

      // Close the client connection
      client.stop();

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
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}