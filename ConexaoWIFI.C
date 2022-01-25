#include <WiFi.h>;

const char* login = "brisa-1776554";
const char* password =  "5qabth84";

void setup() {
  Serial.begin(9600);
  WiFi.begin(login, password);
  Serial.print("Connectando ao WiFi");
 
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
 
  Serial.println("\nConectado ao WiFi");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if ((WiFi.status() == WL_CONNECTED)) //Check the current connection status
  {
    Serial.println("Tente pingar");
    delay(5000);
  }
  else
  {
    Serial.println("Conexão perdida");
  }
}
