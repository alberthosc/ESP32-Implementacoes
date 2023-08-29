#include <Wire.h>

#define RTCadress 0x68

byte BCDtoDEC(byte value)
{
  return ((value/16*10) + (value%16));
}
byte DECtoBCD(byte value)
{
  return ((value/10*16) + (value%10));
}

void setime (byte second, byte minute, byte hour, byte day, byte date, byte month, byte year)
{
  Wire.beginTransmission(RTCadress);
  Wire.write(0);
  
  Wire.write(DECtoBCD(second));
  Wire.write(DECtoBCD(minute));
  Wire.write(DECtoBCD(hour));
  Wire.write(DECtoBCD(day));
  Wire.write(DECtoBCD(date));
  Wire.write(DECtoBCD(month));
  Wire.write(DECtoBCD(year));

  Wire.endTransmission();
}

void time (byte *second, byte *minute, byte *hour, byte *day, byte *date, byte *month, byte *year)
{
  Wire.beginTransmission(RTCadress);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(RTCadress, 7);

  *second = BCDtoDEC (Wire.read() & 0x7F);
  *minute = BCDtoDEC (Wire.read());
  *hour = BCDtoDEC (Wire.read() & 0x3F);
  *day = BCDtoDEC (Wire.read());
  *date = BCDtoDEC (Wire.read());
  *month = BCDtoDEC (Wire.read());
  *year = BCDtoDEC (Wire.read());
}

void printRTC ()
{
  byte second, minute, hour, day, date, month, year;
  time(&second, &minute, &hour, &day, &date, &month, &year);

  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.print(second);
  Serial.print(" ");

  if (day == 1)
    Serial.print("(Domingo) ");
  else if (day == 2)
    Serial.print("(Segunda-feira) ");
  else if (day == 3)
    Serial.print("(Terça-feira) "); 
  else if (day == 4)
    Serial.print("(Quarta-feira) ");
  else if (day == 5)
    Serial.print("(Quinta-feira) ");
  else if (day == 6)
    Serial.print("(Sexta-feira) "); 
  else if (day == 7)
    Serial.print("(Sábado) ");

  Serial.print(date);
  Serial.print("/");
  Serial.print(month);
  Serial.print("/");
  Serial.println(year);
}


void setup() 
{
  Wire.begin(21,22);
  Serial.begin(9600);
  // Caso queira escrever no RTC:
  //setime(0,40,19,1,16,10,22); //second, minute, hour, day, date, month, year
}

void loop() {

  printRTC ();
  delay(1000); // this speeds up the simulation
}