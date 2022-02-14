// TODO: refactor for god sake


int8_t gen_state = 0;

int8_t countdown_flag = 1;
uint32_t countdown_timer = 0;

#define RELAY1 25
#define RELAY2 26
#define RELAY3 27
#define RELAY4 18

#define RELAY5 19

#define IN1 32

typedef struct core_t
{
  int32_t millis_curr;

  int16_t seconds_prev;
  int16_t seconds_curr;
} core_t;
core_t core = {};

typedef struct din_t
{
  int8_t state = 0;
  int8_t state_prev = -1;
} din_t;
din_t din1;

typedef struct relays_t
{
  uint8_t update;
  uint8_t index;
  uint8_t counter;

  uint32_t millis_delay_start;
} relays_t;
relays_t relays = {};

typedef struct relay_t
{
  int32_t seconds_prev;
  int32_t seconds_curr;
} relay_t;
relay_t gen_1 = {};
relay_t gen_2 = {};
relay_t gen_3 = {};
relay_t gen_4 = {};

enum screens {
  screen_splash,
  screen_generators,
  screen_settings,
  screen_info,
};

typedef struct nextion_t
{
  uint8_t screen;

  uint8_t listen_new_data_state;
  uint8_t listen_buffer_counter;
  uint32_t listen_current_millis;

  uint8_t relay1_update;
  uint8_t relay2_update;
  uint8_t relay3_update;
  uint8_t relay4_update;
} nextion_t;
nextion_t nextion = {};

const uint8_t BUFFER_SIZE = 20;
uint8_t buffer_nextion[BUFFER_SIZE];
uint8_t nextion_page_generators_to_settings[BUFFER_SIZE] = {101, 1, 2, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_page_generators_to_info[BUFFER_SIZE] = {101, 1, 3, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint8_t nextion_page_settings_to_generators[BUFFER_SIZE] = {101, 2, 1, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_page_settings_to_info[BUFFER_SIZE] = {101, 2, 3, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint8_t nextion_page_info_to_generators[BUFFER_SIZE] = {101, 3, 1, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_page_info_to_settings[BUFFER_SIZE] = {101, 3, 2, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// ----------------------------
// ------ NEXTION UTILS -------
// ----------------------------
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

// -----------------------
// ------- NEXTION -------
// -----------------------
void nextion_manager()
{
  nextion_listen();
  nextion_screen_generator_update();
}

void nextion_evaluate_serial()
{
  if (0) {}
  else if (compare_array(buffer_nextion, nextion_page_generators_to_settings))
  {
    uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x32, 0xff, 0xff, 0xff};
    nextion_exec_cmd(buff, sizeof(buff));
  }
  else if (compare_array(buffer_nextion, nextion_page_generators_to_info))
  {
    uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x33, 0xff, 0xff, 0xff};
    nextion_exec_cmd(buff, sizeof(buff));
  }

  else if (compare_array(buffer_nextion, nextion_page_settings_to_generators))
  {
    uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x31, 0xff, 0xff, 0xff};
    nextion_exec_cmd(buff, sizeof(buff));
  }
  else if (compare_array(buffer_nextion, nextion_page_settings_to_info))
  {
    uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x33, 0xff, 0xff, 0xff};
    nextion_exec_cmd(buff, sizeof(buff));
  }

  else if (compare_array(buffer_nextion, nextion_page_info_to_generators))
  {
    uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x31, 0xff, 0xff, 0xff};
    nextion_exec_cmd(buff, sizeof(buff));
  }
  else if (compare_array(buffer_nextion, nextion_page_info_to_settings))
  {
    uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x32, 0xff, 0xff, 0xff};
    nextion_exec_cmd(buff, sizeof(buff));
  }
}

void nextion_push_screen_splash()
{
  nextion.screen = screen_splash;
  uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x30, 0xff, 0xff, 0xff};
  nextion_exec_cmd(buff, sizeof(buff));
}
void nextion_push_screen_generators()
{
  nextion.screen = screen_generators;
  uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x31, 0xff, 0xff, 0xff};
  nextion_exec_cmd(buff, sizeof(buff));
}
void nextion_push_screen_settings()
{
  nextion.screen = screen_settings;
  uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x32, 0xff, 0xff, 0xff};
  nextion_exec_cmd(buff, sizeof(buff));
}
void nextion_push_screen_info()
{
  nextion.screen = screen_info;
  uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x33, 0xff, 0xff, 0xff};
  nextion_exec_cmd(buff, sizeof(buff));
}

void nextion_screen_generator_update()
{
  if (nextion.relay1_update)
  {
    nextion.relay1_update = 0;
    uint8_t buff[] = {0x74, 0x30, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
    buff[8] = (gen_1.seconds_curr % 10000 / 1000) + 0x30;
    buff[9] = (gen_1.seconds_curr % 1000 / 100) + 0x30;
    buff[11] = (gen_1.seconds_curr % 100 / 10) + 0x30;
    buff[12] = (gen_1.seconds_curr % 10 / 1) + 0x30;
    nextion_exec_cmd(buff, sizeof(buff));

    if (relays.index != 1)
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
  if (nextion.relay2_update)
  {
    nextion.relay2_update = 0;
    uint8_t buff[] = {0x74, 0x31, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
    buff[8] = (gen_2.seconds_curr % 10000 / 1000) + 0x30;
    buff[9] = (gen_2.seconds_curr % 1000 / 100) + 0x30;
    buff[11] = (gen_2.seconds_curr % 100 / 10) + 0x30;
    buff[12] = (gen_2.seconds_curr % 10 / 1) + 0x30;
    nextion_exec_cmd(buff, sizeof(buff));

    if (relays.index != 2)
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
  if (nextion.relay3_update)
  {
    nextion.relay3_update = 0;
    uint8_t buff[] = {0x74, 0x32, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
    buff[8] = (gen_3.seconds_curr % 10000 / 1000) + 0x30;
    buff[9] = (gen_3.seconds_curr % 1000 / 100) + 0x30;
    buff[11] = (gen_3.seconds_curr % 100 / 10) + 0x30;
    buff[12] = (gen_3.seconds_curr % 10 / 1) + 0x30;
    nextion_exec_cmd(buff, sizeof(buff));
    
    if (relays.index != 3)
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
  if (nextion.relay4_update)
  {
    nextion.relay4_update = 0;
    uint8_t buff[] = {0x74, 0x33, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
    buff[8] = (gen_4.seconds_curr % 10000 / 1000) + 0x30;
    buff[9] = (gen_4.seconds_curr % 1000 / 100) + 0x30;
    buff[11] = (gen_4.seconds_curr % 100 / 10) + 0x30;
    buff[12] = (gen_4.seconds_curr % 10 / 1) + 0x30;
    nextion_exec_cmd(buff, sizeof(buff));

    if (relays.index != 0)
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
  }
}

// -----------------------
// --------- CORE --------
// -----------------------
void core_manager()
{
  if (millis() - core.millis_curr > 1000)
  {
    core.millis_curr = millis();
    core.seconds_curr++;

    relays.update = 1;
  }
}

// -----------------------
// -------- RELAYS -------
// -----------------------
void relays_manager()
{
  relays_seconds_update();
  relays_power_on();
}
void relays_seconds_update()
{
  if (relays.update)
  {
    relays.update = 0;

    if (gen_state)
    {
      if (++relays.counter >= 15)
      {
        relays.counter = 0;
        relays.index = ++relays.index % 4;
      }

      if (relays.index == 0)
      {
        gen_1.seconds_curr++;
        gen_2.seconds_curr++;
        gen_3.seconds_curr++;
        gen_4.seconds_curr = 0;
      }
      else if (relays.index == 1)
      {
        gen_1.seconds_curr = 0;
        gen_2.seconds_curr++;
        gen_3.seconds_curr++;
        gen_4.seconds_curr++;
      }
      else if (relays.index == 2)
      {
        gen_1.seconds_curr++;
        gen_2.seconds_curr = 0;
        gen_3.seconds_curr++;
        gen_4.seconds_curr++;
      }
      else if (relays.index == 3)
      {
        gen_1.seconds_curr++;
        gen_2.seconds_curr++;
        gen_3.seconds_curr = 0;
        gen_4.seconds_curr++;
      }
    }
  }

  if (gen_1.seconds_prev != gen_1.seconds_curr)
  {
    gen_1.seconds_prev = gen_1.seconds_curr;
    nextion.relay1_update = 1;
  }
  if (gen_2.seconds_prev != gen_2.seconds_curr)
  {
    gen_2.seconds_prev = gen_2.seconds_curr;
    nextion.relay2_update = 1;
  }
  if (gen_3.seconds_prev != gen_3.seconds_curr)
  {
    gen_3.seconds_prev = gen_3.seconds_curr;
    nextion.relay3_update = 1;
  }
  if (gen_4.seconds_prev != gen_4.seconds_curr)
  {
    gen_4.seconds_prev = gen_4.seconds_curr;
    nextion.relay4_update = 1;
  }
}

void relays_power_on()
{
  if (gen_state)
  {
    if (relays.index == 0)
    {
      digitalWrite(RELAY1, HIGH);
      digitalWrite(RELAY2, HIGH);
      digitalWrite(RELAY3, HIGH);
      digitalWrite(RELAY4, LOW);
    }
    else if (relays.index == 1)
    {
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, HIGH);
      digitalWrite(RELAY3, HIGH);
      digitalWrite(RELAY4, HIGH);
    }
    else if (relays.index == 2)
    {
      digitalWrite(RELAY1, HIGH);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, HIGH);
      digitalWrite(RELAY4, HIGH);
    }
    else if (relays.index == 3)
    {
      digitalWrite(RELAY1, HIGH);
      digitalWrite(RELAY2, HIGH);
      digitalWrite(RELAY3, LOW);
      digitalWrite(RELAY4, HIGH);
    }
  }
  else
  {
    digitalWrite(RELAY1, LOW);
    digitalWrite(RELAY2, LOW);
    digitalWrite(RELAY3, LOW);
    digitalWrite(RELAY4, LOW);
  }
}
void relays_debug()
{
  Serial.print(gen_1.seconds_curr);
  Serial.print(", ");
  Serial.print(gen_2.seconds_curr);
  Serial.print(", ");
  Serial.print(gen_3.seconds_curr);
  Serial.print(", ");
  Serial.print(gen_4.seconds_curr);
  Serial.println();
}

// -----------------------
// -------- OXYGEN -------
// -----------------------
void oxygen_manager()
{
  if (!digitalRead(IN1)) digitalWrite(RELAY5, HIGH);
  else digitalWrite(RELAY5, LOW);
}

// -----------------------
// -------- INPUT --------
// -----------------------
void input_manager()
{
  // get input
  din1.state = !digitalRead(IN1);

  // one shot flags
  if (din1.state_prev != din1.state)
  {
    din1.state_prev = din1.state;

    if (din1.state)
    {
      countdown_flag = 1;
      countdown_timer = millis();
    }
    else
    {
      countdown_flag = 0;
    }
  }

  // delay generators start
  if (countdown_flag)
  {
    if (millis() - countdown_timer > 3000)
    {
      gen_state = 1;
    }
  }
  else
  {
    gen_state = 0;
  }
}

void setup()
{
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);
  pinMode(RELAY5, OUTPUT);
  pinMode(IN1, INPUT);

  Serial.begin(9600);
  Serial2.begin(9600);

  delay(1000);
  nextion_push_screen_splash();
  delay(3000);
  nextion_push_screen_generators();
}

void loop()
{
  core_manager();
  input_manager();
  oxygen_manager();
  relays_manager();
  nextion_manager();
}
