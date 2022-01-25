#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

const char* login = "brisa-1776554";
const char* password =  "5qabth84";

#define SEG0 23
#define SEG1 22
#define SEG2 16
#define SEG3 17
#define SEG4 5
#define SEG5 18
#define SEG6 19
#define DP   21

WiFiServer servidor (80);

void apagaTudo (){
  digitalWrite(SEG0, LOW);
  digitalWrite(SEG1, LOW); 
  digitalWrite(SEG2, LOW); 
  digitalWrite(SEG3, LOW); 
  digitalWrite(SEG4, LOW); 
  digitalWrite(SEG5, LOW); 
  digitalWrite(SEG6, LOW);  
  digitalWrite(DP, LOW);  
}

void setup() 
{
  pinMode(SEG0, OUTPUT);
  pinMode(SEG1, OUTPUT);
  pinMode(SEG2, OUTPUT);
  pinMode(SEG3, OUTPUT);
  pinMode(SEG4, OUTPUT);
  pinMode(SEG5, OUTPUT);
  pinMode(SEG6, OUTPUT);
  pinMode(DP, OUTPUT);
  
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
          apagaTudo();
          digitalWrite(SEG1, HIGH); 
          digitalWrite(SEG2, HIGH);           
        }
        else if (currentLine.endsWith("GET /2")) 
        {
          apagaTudo();
          digitalWrite(SEG0, HIGH);
          digitalWrite(SEG1, HIGH); 
          digitalWrite(SEG3, HIGH); 
          digitalWrite(SEG4, HIGH); 
          digitalWrite(SEG6, HIGH);         
        }
        else if (currentLine.endsWith("GET /3")) 
        {
          apagaTudo();
          digitalWrite(SEG0, HIGH);
          digitalWrite(SEG1, HIGH); 
          digitalWrite(SEG2, HIGH); 
          digitalWrite(SEG3, HIGH); 
          digitalWrite(SEG6, HIGH);         
        }
        else if (currentLine.endsWith("GET /4")) 
        {
          apagaTudo();
          digitalWrite(SEG1, HIGH); 
          digitalWrite(SEG2, HIGH); 
          digitalWrite(SEG5, HIGH); 
          digitalWrite(SEG6, HIGH);         
        }
         else if (currentLine.endsWith("GET /5")) 
        {
          apagaTudo();
          digitalWrite(SEG0, HIGH); 
          digitalWrite(SEG2, HIGH); 
          digitalWrite(SEG3, HIGH); 
          digitalWrite(SEG5, HIGH); 
          digitalWrite(SEG6, HIGH);         
        }
        else if (currentLine.endsWith("GET /6"))
        {
          digitalWrite(DP, HIGH);
        }
        else if (currentLine.endsWith("GET /0"))
        {
          apagaTudo();
        }
      }
    }
  }
}
