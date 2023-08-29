#include <Arduino.h>
#include <WiFi.h>

const char* ssid = "brisa-1776554";
const char* password = "5qabth84";

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao Wi-Fi...");
  }
}

void loop() {
  Serial.println("Conectado na rede!");
  delay(2000);

  int itensidadeWIFI = WiFi.RSSI();
  Serial.print("Intensidade do sinal Wi-Fi: ");
  Serial.print(itensidadeWIFI);
  Serial.println(" dBm");
}