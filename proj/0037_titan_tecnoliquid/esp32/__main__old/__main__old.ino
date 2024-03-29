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
  uint8_t screen;

  uint8_t listen_new_data_state;
  uint8_t listen_buffer_counter;
  uint32_t listen_current_millis;

  int8_t generator1_state_prev;
  int8_t generator1_state_curr;
} nextion_t;
nextion_t nextion = {};



typedef struct core_t {
  uint32_t millis_curr;
  uint32_t seconds_prev;
  uint32_t seconds_curr;
  uint8_t cycle_prev;
  uint8_t cycle_curr;
} core_t;
core_t core = {};

typedef struct relay_t {
  int counter;
} relay_t;
relay_t relay_1 = {};
relay_t relay_2 = {};
relay_t relay_3 = {};
relay_t relay_4 = {};

enum screen {
  screen_splash,
  screen_generators,
  screen_settings,
  screen_info,
};

const uint8_t BUFFER_SIZE = 20;
uint8_t buffer_nextion[BUFFER_SIZE];

uint8_t nextion_page_generators_to_settings[BUFFER_SIZE] = {101, 1, 2, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint8_t nextion_page_settings_to_generators[BUFFER_SIZE] = {101, 2, 1, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


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

void nextion_evaluate_serial()
{
  if (0) {}
  else if (compare_array(buffer_nextion, nextion_page_generators_to_settings))
  {
    nextion.screen = screen_settings;
    uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x32, 0xff, 0xff, 0xff};
    nextion_exec_cmd(buff, sizeof(buff));
  }
  else if (compare_array(buffer_nextion, nextion_page_settings_to_generators))
  {
    nextion.screen = screen_generators;
    uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x31, 0xff, 0xff, 0xff};
    nextion_exec_cmd(buff, sizeof(buff));
  }
}


void relay(int r1, int r2, int r3, int r4)
{
  digitalWrite(RELAY1, r1);
  digitalWrite(RELAY2, r2);
  digitalWrite(RELAY3, r3);
  digitalWrite(RELAY4, r4);
}

// -------------------------------
// ----------- NEXTION -----------
// -------------------------------
void nextion_goto_page_splash()
{
  nextion.screen = screen_splash;
  uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x30, 0xff, 0xff, 0xff};
  nextion_exec_cmd(buff, sizeof(buff));
}
void nextion_goto_page_generators()
{
  nextion.screen = screen_generators;
  uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x31, 0xff, 0xff, 0xff};
  nextion_exec_cmd(buff, sizeof(buff));
}
void nextion_goto_page_settings()
{
  nextion.screen = screen_settings;
  uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x32, 0xff, 0xff, 0xff};
  nextion_exec_cmd(buff, sizeof(buff));
}
void nextion_goto_page_info()
{
  nextion.screen = screen_info;
  uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x33, 0xff, 0xff, 0xff};
  nextion_exec_cmd(buff, sizeof(buff));
}
void nextion_manager()
{
  nextion_listen();
  if (nextion.screen == screen_generators) nextion_screen_generators_manager();
  else if (nextion.screen == screen_settings) nextion_screen_settings_manager();
  else if (nextion.screen == screen_info) nextion_screen_info_manager();
}
void nextion_screen_generators_manager()
{

}
void nextion_screen_settings_manager()
{

}
void nextion_screen_info_manager()
{

}

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

  delay(1000);
  nextion_goto_page_splash();
  delay(3000);
  nextion_goto_page_generators();

}

void nextion_draw_gen1(int val)
{
  // Print Val
  {
    uint8_t buff[] = {0x74, 0x30, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
    buff[11] = (relay_1.counter % 100 / 10) + 0x30;
    buff[12] = (relay_1.counter % 10 / 1) + 0x30;
    nextion_exec_cmd(buff, sizeof(buff));
  }

  //Change Color
  if (val)
  {
    {
      uint8_t buff[] = {0x74, 0x34, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
      nextion_exec_cmd(buff, sizeof(buff));
    }
    {
      uint8_t buff[] = {0x74, 0x30, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
      nextion_exec_cmd(buff, sizeof(buff));
    }
  }
  else
  {
    {
      uint8_t buff[] = {0x74, 0x34, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x34, 0x33, 0x35, 0x37, 0xff, 0xff, 0xff};
      nextion_exec_cmd(buff, sizeof(buff));
    }
    {
      uint8_t buff[] = {0x74, 0x30, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x34, 0x33, 0x35, 0x37, 0xff, 0xff, 0xff};
      nextion_exec_cmd(buff, sizeof(buff));
    }
  }
}

void nextion_draw_gen2(int val)
{
  // Print Val
  {
    uint8_t buff[] = {0x74, 0x31, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
    buff[11] = (relay_2.counter % 100 / 10) + 0x30;
    buff[12] = (relay_2.counter % 10 / 1) + 0x30;
    nextion_exec_cmd(buff, sizeof(buff));
  }

  //Change Color
  if (val)
  {
    {
      uint8_t buff[] = {0x74, 0x35, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
      nextion_exec_cmd(buff, sizeof(buff));
    }
    {
      uint8_t buff[] = {0x74, 0x31, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
      nextion_exec_cmd(buff, sizeof(buff));
    }
  }
  else
  {
    {
      uint8_t buff[] = {0x74, 0x35, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x34, 0x33, 0x35, 0x37, 0xff, 0xff, 0xff};
      nextion_exec_cmd(buff, sizeof(buff));
    }
    {
      uint8_t buff[] = {0x74, 0x31, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x34, 0x33, 0x35, 0x37, 0xff, 0xff, 0xff};
      nextion_exec_cmd(buff, sizeof(buff));
    }
  }
}

void nextion_draw_gen3(int val)
{
  // Print Val
  {
    uint8_t buff[] = {0x74, 0x32, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
    buff[11] = (relay_3.counter % 100 / 10) + 0x30;
    buff[12] = (relay_3.counter % 10 / 1) + 0x30;
    nextion_exec_cmd(buff, sizeof(buff));
  }

  //Change Color
  if (val)
  {
    {
      uint8_t buff[] = {0x74, 0x36, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
      nextion_exec_cmd(buff, sizeof(buff));
    }
    {
      uint8_t buff[] = {0x74, 0x32, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
      nextion_exec_cmd(buff, sizeof(buff));
    }
  }
  else
  {
    {
      uint8_t buff[] = {0x74, 0x36, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x34, 0x33, 0x35, 0x37, 0xff, 0xff, 0xff};
      nextion_exec_cmd(buff, sizeof(buff));
    }
    {
      uint8_t buff[] = {0x74, 0x32, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x34, 0x33, 0x35, 0x37, 0xff, 0xff, 0xff};
      nextion_exec_cmd(buff, sizeof(buff));
    }
  }
}

void nextion_draw_gen4(int val)
{
  // Print Val
  /*
    {
      uint8_t buff[] = {0x74, 0x33, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
      buff[11] = (relay_4.counter % 100 / 10) + 0x30;
      buff[12] = (relay_4.counter % 10 / 1) + 0x30;
      nextion_exec_cmd(buff, sizeof(buff));
    }
    //Change Color
    if (val)
    {
      {
        uint8_t buff[] = {0x74, 0x37, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
        nextion_exec_cmd(buff, sizeof(buff));
      }
      {
        uint8_t buff[] = {0x74, 0x33, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x36, 0x35, 0x35, 0x33, 0x35, 0xff, 0xff, 0xff};
        nextion_exec_cmd(buff, sizeof(buff));
      }
    }
    else
    {
      {
        uint8_t buff[] = {0x74, 0x37, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x34, 0x33, 0x35, 0x37, 0xff, 0xff, 0xff};
        nextion_exec_cmd(buff, sizeof(buff));
      }
      {
        uint8_t buff[] = {0x74, 0x33, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x34, 0x33, 0x35, 0x37, 0xff, 0xff, 0xff};
        nextion_exec_cmd(buff, sizeof(buff));
      }
    }
  */
}






void loop()
{
  nextion_manager();

  if (core.seconds_prev != core.seconds_curr)
  {
    core.seconds_prev = core.seconds_curr;

    if (core.cycle_curr == 0)
    {
      relay_1.counter++;
      relay_2.counter++;
      relay_3.counter++;
      relay_4.counter = 0;

      nextion_draw_gen1(1);
      nextion_draw_gen2(1);
      nextion_draw_gen3(1);
      nextion_draw_gen4(0);
    }
    else if (core.cycle_curr == 1)
    {
      relay_1.counter = 0;
      relay_2.counter++;
      relay_3.counter++;
      relay_4.counter++;

      nextion_draw_gen1(0);
      nextion_draw_gen2(1);
      nextion_draw_gen3(1);
      nextion_draw_gen4(1);
    }
    else if (core.cycle_curr == 2)
    {
      relay_1.counter++;
      relay_2.counter = 0;
      relay_3.counter++;
      relay_4.counter++;

      nextion_draw_gen1(1);
      nextion_draw_gen2(0);
      nextion_draw_gen3(1);
      nextion_draw_gen4(1);
    }
    else if (core.cycle_curr == 3)
    {
      relay_1.counter++;
      relay_2.counter++;
      relay_3.counter = 0;
      relay_4.counter++;

      nextion_draw_gen1(1);
      nextion_draw_gen2(1);
      nextion_draw_gen3(0);
      nextion_draw_gen4(1);
    }

    Serial.print(relay_1.counter);
    Serial.print(", ");
    Serial.print(relay_2.counter);
    Serial.print(", ");
    Serial.print(relay_3.counter);
    Serial.print(", ");
    Serial.print(relay_4.counter);
    Serial.println();
  }

  if (digitalRead(IN1) == 0)
  {
    if (millis() - core.millis_curr > 1000)
    {
      core.millis_curr = millis();
      core.seconds_curr++;
    }

    if (core.seconds_curr >= 15)
    {
      core.seconds_curr = 0;
      core.cycle_curr++;
      core.cycle_curr %= 4;
    }

    if (core.cycle_curr == 0) relay(1, 1, 1, 0);
    else if (core.cycle_curr == 1) relay(0, 1, 1, 1);
    else if (core.cycle_curr == 2) relay(1, 0, 1, 1);
    else if (core.cycle_curr == 3) relay(1, 1, 0, 1);
  }
  else
  {
    relay(0, 0, 0, 0);
  }
}
