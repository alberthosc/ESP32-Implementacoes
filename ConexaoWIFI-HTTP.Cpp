#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

const char* login = "******";
const char* password =  "******";

WiFiServer servidor (80);

void setup() 
{
  pinMode(23, OUTPUT);
  
  Serial.begin(9600);
  WiFi.begin(login, password);
  Serial.print("Connectando ao WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nConectado ao WiFi");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  servidor.begin();
}

void loop() 
{
  WiFiClient client = servidor.available();

  if (client) 
  {
    Serial.println("new client");
    String currentLine = "";
    while (client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();
        Serial.write(c);

        if (c != '\r') 
        {  
          currentLine += c;    
        }
        if (currentLine.endsWith("GET /1")) 
        {
          digitalWrite(23, HIGH);          
        }
        else if (currentLine.endsWith("GET /0")) 
        {
          digitalWrite(23, LOW);          
        }
      }
    }
  }
}
