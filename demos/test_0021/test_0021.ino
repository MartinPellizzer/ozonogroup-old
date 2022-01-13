#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3);

int16_t ppb = 0;

void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600);
  pinMode(8, OUTPUT);
}
unsigned char get_checksum(unsigned char *i, unsigned char ln)
{
  unsigned char j, tempq = 0;
  i += 1;
  for (j = 0; j < (ln - 2); j++)
  {
    tempq += *i;
    i++;
  }
  tempq = (~tempq) + 1;
  return (tempq);
}
void sensor_read()
{
  if (mySerial.available() > 0)
  {
    byte buff[8];
    mySerial.readBytes(buff, 9);
    if (get_checksum(buff, 9) == buff[8])
    {
      int tmp = buff[4] * 256 + buff[5];
      if (tmp >= 0 && tmp <= 10000)
      {
        ppb = tmp;
        Serial.println(ppb);
      }
    }
  }
}

void loop()
{
  sensor_read();
}
