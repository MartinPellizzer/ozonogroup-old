#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3);

void setup()
{
  mySerial.begin(9600);
}

void loop()
{
  mySerial.write("0");
  delay(1000);
}
