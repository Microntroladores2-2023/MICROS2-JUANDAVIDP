#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>


// Network information
const char* ssid = "JuanD";
const char* password = "ingucv2207";
const char* serverIP = "172.16.0.4"; // IP de PC
const int serverPort = 1234;  // puerto del programa que se use como servidor

void setup() {
   Serial.begin(115200);
   WiFi.begin(ssid, password);

   while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando al Wifi");
  }

  Serial.println("Conectando al Wifi");

  WiFiClient client;

  Serial.print("Conectando al Servidor");
  Serial.println(serverIP);

  if (!client.connect(serverIP, serverPort)){
    Serial.println("Conexion fallida");
    return;
  }

  Serial.println("Conectado al Servidor");
  client.println("Hola servidor! Bienvenido");
}

void loop() {

}