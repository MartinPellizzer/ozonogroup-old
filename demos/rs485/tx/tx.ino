#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX, TX

void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600);
}

byte test = 0;
void loop()
{
  mySerial.write(test);
  delay(500);
}
