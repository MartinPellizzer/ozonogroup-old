#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3);

void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600);
}

void loop()
{
  if (mySerial.available() > 0)
  {
    Serial.println(mySerial.read());
  }
}
