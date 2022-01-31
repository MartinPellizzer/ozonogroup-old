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

uint8_t nextion_page1_icon1[BUFFER_SIZE] = {101, 1, 1, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_page1_icon2[BUFFER_SIZE] = {101, 1, 2, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_page1_icon3[BUFFER_SIZE] = {101, 1, 3, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_page2_icon1[BUFFER_SIZE] = {101, 2, 1, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_page2_icon2[BUFFER_SIZE] = {101, 2, 2, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_page2_icon3[BUFFER_SIZE] = {101, 2, 3, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_page3_icon1[BUFFER_SIZE] = {101, 3, 1, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_page3_icon2[BUFFER_SIZE] = {101, 3, 2, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_page3_icon3[BUFFER_SIZE] = {101, 3, 3, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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
  if (compare_array(nextion_page1_icon1, buffer_nextion)) {
  }
  if (compare_array(nextion_page1_icon2, buffer_nextion)) {
    uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x32, 0xff, 0xff, 0xff};
    nextion_exec_cmd(buff, sizeof(buff));
  }
  if (compare_array(nextion_page1_icon3, buffer_nextion)) {
    uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x33, 0xff, 0xff, 0xff};
    nextion_exec_cmd(buff, sizeof(buff));
  }
  if (compare_array(nextion_page2_icon1, buffer_nextion)) {
    uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x31, 0xff, 0xff, 0xff};
    nextion_exec_cmd(buff, sizeof(buff));
  }
  if (compare_array(nextion_page2_icon2, buffer_nextion)) {
  }
  if (compare_array(nextion_page2_icon3, buffer_nextion)) {
    uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x33, 0xff, 0xff, 0xff};
    nextion_exec_cmd(buff, sizeof(buff));
  }
  if (compare_array(nextion_page3_icon1, buffer_nextion)) {
    uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x31, 0xff, 0xff, 0xff};
    nextion_exec_cmd(buff, sizeof(buff));
  }
  if (compare_array(nextion_page3_icon2, buffer_nextion)) {
    uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x32, 0xff, 0xff, 0xff};
    nextion_exec_cmd(buff, sizeof(buff));
  }
  if (compare_array(nextion_page3_icon3, buffer_nextion)) {
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

void relay(int r1, int r2, int r3, int r4)
{
  digitalWrite(RELAY1, r1);
  digitalWrite(RELAY2, r2);
  digitalWrite(RELAY3, r3);
  digitalWrite(RELAY4, r4);
}

uint32_t current_millis = 0;
int relay_index = 0;
uint32_t counter = 0;

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

  uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x31, 0xff, 0xff, 0xff};
  nextion_exec_cmd(buff, sizeof(buff));
}

typedef struct relay_t {
  int counter;
} relay_t;

relay_t relay_1 = {};
relay_t relay_2 = {};
relay_t relay_3 = {};
relay_t relay_4 = {};

void loop()
{
  nextion_listen();

  if (digitalRead(IN1) == 0)
  {
    if (millis() - current_millis > 1000)
    {
      current_millis = millis();
      counter++;

      if (relay_index == 0)
      {
        relay_1.counter++;
        relay_2.counter++;
        relay_3.counter++;
        relay_4.counter = 0;

        {
          uint8_t buff[] = {0x74, 0x34, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x30, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x35, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x31, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x36, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x32, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x37, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x34, 0x33, 0x35, 0x37, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x33, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x34, 0x33, 0x35, 0x37, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
      }
      else if (relay_index == 1)
      {
        relay_1.counter = 0;
        relay_2.counter++;
        relay_3.counter++;
        relay_4.counter++;

        {
          uint8_t buff[] = {0x74, 0x34, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x34, 0x33, 0x35, 0x37, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x30, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x34, 0x33, 0x35, 0x37, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x35, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x31, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x36, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x32, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x37, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x33, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
      }
      else if (relay_index == 2)
      {
        relay_1.counter++;
        relay_2.counter = 0;
        relay_3.counter++;
        relay_4.counter++;

        {
          uint8_t buff[] = {0x74, 0x34, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x30, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x35, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x34, 0x33, 0x35, 0x37, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x31, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x34, 0x33, 0x35, 0x37, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x36, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x32, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x37, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x33, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
      }
      else if (relay_index == 3)
      {
        relay_1.counter++;
        relay_2.counter++;
        relay_3.counter = 0;
        relay_4.counter++;

        {
          uint8_t buff[] = {0x74, 0x34, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x30, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x35, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x31, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x36, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x34, 0x33, 0x35, 0x37, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x32, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x34, 0x33, 0x35, 0x37, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x37, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        }
        {
          uint8_t buff[] = {0x74, 0x33, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
          nextion_exec_cmd(buff, sizeof(buff));
        } 
      }

      {
        uint8_t buff[] = {0x74, 0x30, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
        buff[11] = (relay_1.counter % 100 / 10) + 0x30;
        buff[12] = (relay_1.counter % 10 / 1) + 0x30;
        nextion_exec_cmd(buff, sizeof(buff));
      }

      {
        uint8_t buff[] = {0x74, 0x31, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
        buff[11] = (relay_2.counter % 100 / 10) + 0x30;
        buff[12] = (relay_2.counter % 10 / 1) + 0x30;
        nextion_exec_cmd(buff, sizeof(buff));
      }

      {
        uint8_t buff[] = {0x74, 0x32, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
        buff[11] = (relay_3.counter % 100 / 10) + 0x30;
        buff[12] = (relay_3.counter % 10 / 1) + 0x30;
        nextion_exec_cmd(buff, sizeof(buff));
      }

      {
        uint8_t buff[] = {0x74, 0x33, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
        buff[11] = (relay_4.counter % 100 / 10) + 0x30;
        buff[12] = (relay_4.counter % 10 / 1) + 0x30;
        nextion_exec_cmd(buff, sizeof(buff));
      }
    }

    if (counter >= 15)
    {
      counter = 0;
      relay_index++;
      relay_index %= 4;
    }

    if (relay_index == 0) relay(1, 1, 1, 0);
    else if (relay_index == 1) relay(0, 1, 1, 1);
    else if (relay_index == 2) relay(1, 0, 1, 1);
    else if (relay_index == 3) relay(1, 1, 0, 1);
  }
  else
  {
    relay(0, 0, 0, 0);
  }
}
