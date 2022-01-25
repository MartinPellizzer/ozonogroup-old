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


#define RELAY1 25
#define RELAY2 26
#define RELAY3 27
#define RELAY4 18
#define RELAY5 19
#define RELAY6 21
#define RELAY7 22
#define RELAY8 23

#define IN1 32
#define IN2 33

void setup()
{
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);
  pinMode(RELAY5, OUTPUT);
  pinMode(RELAY6, OUTPUT);
  pinMode(RELAY7, OUTPUT);
  pinMode(RELAY8, OUTPUT);

  pinMode(IN1, INPUT);
  pinMode(IN2, INPUT);

  Serial.begin(9600);
  Serial2.begin(9600);
}


int relay_arr[8] = {RELAY1, RELAY2, RELAY3, RELAY4, RELAY5, RELAY6, RELAY7, RELAY8};

void relay(int r1, int r2, int r3, int r4, int r5, int r6, int r7, int r8)
{
  digitalWrite(RELAY1, r1);
  digitalWrite(RELAY2, r2);
  digitalWrite(RELAY3, r3);
  digitalWrite(RELAY4, r4);
  digitalWrite(RELAY5, r5);
  digitalWrite(RELAY6, r6);
  digitalWrite(RELAY7, r7);
  digitalWrite(RELAY8, r8);
}

uint32_t current_millis = 0;
int relay_index = 0;

void loop()
{
  if (digitalRead(IN1) == 1)
  {
    relay(0, 0, 0, 0, 0, 0, 0, 0);
  }
  else
  {
    if (millis() - current_millis > 1000)
    {
      current_millis = millis();

      if (relay_index == 0) relay(1, 1, 1, 0, 0, 0, 0, 0);
      else if (relay_index == 1) relay(0, 1, 1, 1, 0, 0, 0, 0);
      else if (relay_index == 2) relay(1, 0, 1, 1, 0, 0, 0, 0);
      else if (relay_index == 3) relay(1, 1, 0, 1, 0, 0, 0, 0);

      relay_index++;
      relay_index %= 4;
    }
  }

  /*
    relay(1, 1, 1, 0, 0, 0, 0, 0);
    delay(1000);
    relay(0, 1, 1, 1, 0, 0, 0, 0);
    delay(1000);
    relay(0, 0, 1, 1, 1, 0, 0, 0);
    delay(1000);
    relay(0, 0, 0, 1, 1, 1, 0, 0);
    delay(1000);
    relay(0, 0, 0, 0, 1, 1, 1, 0);
    delay(1000);
    relay(0, 0, 0, 0, 0, 1, 1, 1);
    delay(1000);
    relay(1, 0, 0, 0, 0, 0, 1, 1);
    delay(1000);
    relay(1, 1, 0, 0, 0, 0, 0, 1);
    delay(1000);
  */
  /*
    if (digitalRead(IN1) == LOW)
    {
    digitalWrite(RELAY1, HIGH);
    digitalWrite(RELAY2, LOW);
    digitalWrite(RELAY3, HIGH);
    digitalWrite(RELAY4, LOW);
    digitalWrite(RELAY5, HIGH);
    digitalWrite(RELAY6, LOW);
    digitalWrite(RELAY7, HIGH);
    digitalWrite(RELAY8, LOW);
    }
    else
    {
    digitalWrite(RELAY1, LOW);
    digitalWrite(RELAY2, HIGH);
    digitalWrite(RELAY3, LOW);
    digitalWrite(RELAY4, HIGH);
    digitalWrite(RELAY5, LOW);
    digitalWrite(RELAY6, HIGH);
    digitalWrite(RELAY7, LOW);
    digitalWrite(RELAY8, HIGH);
    }
  */
}
