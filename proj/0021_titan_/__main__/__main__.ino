#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3);

typedef struct sensor_t
{
  int16_t ppb;
} sensor_t;
sensor_t sensor = {};

bool nextion_send_command(uint8_t arr[], uint8_t arr_size)
{
  for (uint8_t i = 0; i < arr_size; i++) Serial.write(arr[i]);
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

void read_sensor()
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
        sensor.ppb = tmp;
        uint8_t buff[] = {0x74, 0x30, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
        buff[8] = (sensor.ppb % 10000 / 1000) + 0x30;
        buff[9] = (sensor.ppb % 1000 / 100) + 0x30;
        buff[11] = (sensor.ppb % 100 / 10) + 0x30;
        buff[12] = (sensor.ppb % 10 / 1) + 0x30;
        nextion_send_command(buff, sizeof(buff));
      }
      digitalWrite(8, !digitalRead(8));
    }
  }
}

void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600);

  digitalWrite(8, LOW);
  pinMode(8, OUTPUT);
}

void loop()
{
  read_sensor();
}
