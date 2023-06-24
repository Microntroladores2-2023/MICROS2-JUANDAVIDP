#include <Arduino.h>
#include <WiFi.h>

// Network information
const char *ssid = "HOLETB 2.4_EXT";
const char *password = "primitiva";

int ModbusTCP_port = 502;

WiFiServer server(ModbusTCP_port);

WiFiClient client;

void TaskWiFiServer(void *pvParameters);
void setupWiFiServer();

//**************************************
//********* Init WiFi Server ***********
//**************************************
void setupWiFiServer()
{

  WiFi.mode(WIFI_STA);
  // Desactiva la suspensión de wifi en modo STA para mejorar la velocidad de respuesta
  WiFi.setSleep(false);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    vTaskDelay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  xTaskCreatePinnedToCore(TaskWiFiServer, "WiFiServer", 4096, NULL, 2, NULL, 1);
}

//**************************************
//*************** TASKs ****************
//**************************************

void TaskWiFiServer(void *pvParameters)
{

  uint8_t ByteArray[255]; // buffer de recepcion de los datos recibidos de los clientes
  uint8_t nDatosRecibidos;

  for (;;)
  {

    // Check if a client has connected
    client = server.available(); // listen for incoming clients
    if (client)                  // if you get a client
    {
      while (client.connected()) // loop while the client's connected
      {
        if (client.available())
        {
          nDatosRecibidos = 0;

          while (client.available())
          {
          }
            ByteArray[nDatosRecibidos++] = client.read();
          // termina bytes to read from the client

          client.flush(); // Discard any bytes that have been written to the client but not yet read.
          if (nDatosRecibidos > 0)
          {
            for (int i = 0; i < nDatosRecibidos; i++)
             Serial.print(ByteArray[i]);
             // printf("%c", ByteArray[i]); // imprime datos Recibidos
          }
          Serial.println("");

          // if( nDatosRecibidos>0)modbusTCPIP(ByteArray, nDatosRecibidos) ;
        }
      }
      client.stop();
    } // fin cliente

    vTaskDelay(100);

  } // fin for
} // fin Task

void setup()
{
  Serial.begin(115200);


  setupWiFiServer();
}

void loop() {}
