#define Potenciometer 15
#define NTC 2
float Temperature = 0.0;

void setup() {
  pinMode(Potenciometer, INPUT);
  pinMode(NTC, INPUT);
  analogReadResolution(10);
  Serial.begin(115200);
}

void loop() {
  Serial.print("Potenciometer Read: ");
  Serial.println(analogRead(Potenciometer));

  Temperature = 1 / (log(1 / (1023. / analogRead(NTC) - 1)) / 3950 + 1.0 / 298.15) - 273.15;
  //Temperature = map(analogRead(NTC),3813,462,-24,80);

  Serial.print("NTC Temperature: ");
  Serial.print(Temperature);
  Serial.println("°C");

  delay(3000); 
}