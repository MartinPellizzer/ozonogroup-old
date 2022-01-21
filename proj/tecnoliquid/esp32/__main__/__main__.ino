typedef struct nextion_t
{
  uint8_t listen_new_data_state;
  uint8_t listen_buffer_counter;
  uint32_t listen_current_millis;
} nextion_t;
nextion_t nextion = {};

const uint8_t BUFFER_SIZE = 20;
uint8_t buffer_nextion[BUFFER_SIZE];

uint8_t nextion_hotspot[BUFFER_SIZE] = {101, 0, 2, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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

  Serial.begin(9600);
  Serial2.begin(9600);
}


void nextion_exec_cmd(uint8_t *buff, uint8_t buff_size)
{
  for (uint8_t i = 0; i < buff_size; i++)
    Serial2.write(buff[i]);
}

bool compare_array(uint8_t *a, uint8_t *b)
{
  for (uint8_t i = 0; i < BUFFER_SIZE; i++) if (a[i] != b[i]) return false;
  return true;
}

void nextion_evaluate_serial()
{
  if (compare_array(nextion_hotspot, buffer_nextion)) {
    uint8_t buff[] = {0x74, 0x30, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x54, 0x45, 0x54, 0x54, 0x45, 0x21, 0x22, 0xff, 0xff, 0xff};
    nextion_exec_cmd(buff, sizeof(buff));
  }
}

void nextion_listen()
{
  if (Serial2.available())
  {
    nextion.listen_new_data_state = true;
    buffer_nextion[nextion.listen_buffer_counter] = Serial2.read();
    if (nextion.listen_buffer_counter < BUFFER_SIZE) nextion.listen_buffer_counter++;
    nextion.listen_current_millis = millis();
  }
  if (nextion.listen_new_data_state)
  {
    if ((millis() - nextion.listen_current_millis) > 10)
    {
      nextion.listen_new_data_state = false;
      nextion.listen_buffer_counter = 0;
      nextion_evaluate_serial();
      for (int i = 0; i < BUFFER_SIZE; i++) buffer_nextion[i] = 0;
    }
  }
}

void loop()
{

  nextion_listen();


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
