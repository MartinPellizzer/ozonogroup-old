void setup()
{
  /*
    pinMode(19, OUTPUT);
    pinMode(21, OUTPUT);
    pinMode(22, OUTPUT);
    pinMode(23, OUTPUT);

    pinMode(17, INPUT);
    pinMode(18, OUTPUT);
  */

  Serial2.begin(9600);
}


void nextion_exec_cmd(uint8_t *buff, uint8_t buff_size)
{
  for (uint8_t i = 0; i < buff_size; i++)
    Serial2.write(buff[i]);
}

void loop()
{

  uint8_t buff[] = {0x74, 0x30, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x54, 0x45, 0x54, 0x54, 0x45, 0x21, 0x22, 0xff, 0xff, 0xff};
  nextion_exec_cmd(buff, sizeof(buff));

  /*
    if(digitalRead(17) == HIGH)
    {
    digitalWrite(18, HIGH);
    }
    else
    {
    digitalWrite(18, LOW);
    }
  */
  /*
    digitalWrite(22, HIGH);
    digitalWrite(23, LOW );
    digitalWrite(21, LOW);
    digitalWrite(19, LOW);
    delay(3000);
    digitalWrite(22, LOW );
    digitalWrite(23, HIGH);
    digitalWrite(21, LOW);
    digitalWrite(19, LOW);
    delay(3000);
    digitalWrite(22, LOW );
    digitalWrite(23, LOW);
    digitalWrite(21, HIGH);
    digitalWrite(19, LOW);
    delay(3000);
    digitalWrite(22, LOW );
    digitalWrite(23, LOW);
    digitalWrite(21, LOW);
    digitalWrite(19, HIGH);
    delay(3000);
  */
}
