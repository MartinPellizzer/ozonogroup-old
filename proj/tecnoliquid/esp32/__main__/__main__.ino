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
  int8_t state_curr = 0;
  int8_t state_prev = -1;
} din_t;
din_t din1;

typedef struct relays_t
{
  uint8_t update;
  uint8_t index;
  uint8_t counter;
} relays_t;
relays_t relays = {};

typedef struct relay_t
{
  int32_t seconds_prev;
  int32_t seconds_curr;
} relay_t;
relay_t relay1 = {};
relay_t relay2 = {};
relay_t relay3 = {};
relay_t relay4 = {};

typedef struct oxygen_t
{
  int8_t state_curr = 0;
  int8_t state_prev = -1;
} oxygen_t;
oxygen_t oxygen;

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
    buff[8] = (relay1.seconds_curr % 10000 / 1000) + 0x30;
    buff[9] = (relay1.seconds_curr % 1000 / 100) + 0x30;
    buff[11] = (relay1.seconds_curr % 100 / 10) + 0x30;
    buff[12] = (relay1.seconds_curr % 10 / 1) + 0x30;
    nextion_exec_cmd(buff, sizeof(buff));
  }
  if (nextion.relay2_update)
  {
    nextion.relay2_update = 0;
    uint8_t buff[] = {0x74, 0x31, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
    buff[8] = (relay2.seconds_curr % 10000 / 1000) + 0x30;
    buff[9] = (relay2.seconds_curr % 1000 / 100) + 0x30;
    buff[11] = (relay2.seconds_curr % 100 / 10) + 0x30;
    buff[12] = (relay2.seconds_curr % 10 / 1) + 0x30;
    nextion_exec_cmd(buff, sizeof(buff));
  }
  if (nextion.relay3_update)
  {
    nextion.relay3_update = 0;
    uint8_t buff[] = {0x74, 0x32, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
    buff[8] = (relay3.seconds_curr % 10000 / 1000) + 0x30;
    buff[9] = (relay3.seconds_curr % 1000 / 100) + 0x30;
    buff[11] = (relay3.seconds_curr % 100 / 10) + 0x30;
    buff[12] = (relay3.seconds_curr % 10 / 1) + 0x30;
    nextion_exec_cmd(buff, sizeof(buff));
  }
  if (nextion.relay4_update)
  {
    nextion.relay4_update = 0;
    uint8_t buff[] = {0x74, 0x33, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
    buff[8] = (relay4.seconds_curr % 10000 / 1000) + 0x30;
    buff[9] = (relay4.seconds_curr % 1000 / 100) + 0x30;
    buff[11] = (relay4.seconds_curr % 100 / 10) + 0x30;
    buff[12] = (relay4.seconds_curr % 10 / 1) + 0x30;
    nextion_exec_cmd(buff, sizeof(buff));
  }
}

// -----------------------
// --------- CORE --------
// -----------------------
void core_manager()
{
  core_seconds_update();
}
void core_seconds_update()
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
uint32_t millis_delayed_start_curr = 0;
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

    if (digitalRead(IN1) == 0)
    {
      if (millis() - millis_delayed_start_curr > 3000)
      {
        if (++relays.counter >= 15)
        {
          relays.counter = 0;
          relays.index = ++relays.index % 4;
        }

        if (relays.index == 0)
        {
          relay1.seconds_curr++;
          relay2.seconds_curr++;
          relay3.seconds_curr++;
          relay4.seconds_curr = 0;
        }
        else if (relays.index == 1)
        {
          relay1.seconds_curr = 0;
          relay2.seconds_curr++;
          relay3.seconds_curr++;
          relay4.seconds_curr++;
        }
        else if (relays.index == 2)
        {
          relay1.seconds_curr++;
          relay2.seconds_curr = 0;
          relay3.seconds_curr++;
          relay4.seconds_curr++;
        }
        else if (relays.index == 3)
        {
          relay1.seconds_curr++;
          relay2.seconds_curr++;
          relay3.seconds_curr = 0;
          relay4.seconds_curr++;
        }
      }
    }
  }

  if (relay1.seconds_prev != relay1.seconds_curr)
  {
    relay1.seconds_prev = relay1.seconds_curr;
    nextion.relay1_update = 1;
  }
  if (relay2.seconds_prev != relay2.seconds_curr)
  {
    relay2.seconds_prev = relay2.seconds_curr;
    nextion.relay2_update = 1;
  }
  if (relay3.seconds_prev != relay3.seconds_curr)
  {
    relay3.seconds_prev = relay3.seconds_curr;
    nextion.relay3_update = 1;
  }
  if (relay4.seconds_prev != relay4.seconds_curr)
  {
    relay4.seconds_prev = relay4.seconds_curr;
    nextion.relay4_update = 1;
  }
}

void relays_power_on()
{
  if (digitalRead(IN1) == 0)
  {
    if (millis() - millis_delayed_start_curr > 3000)
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
  Serial.print(relay1.seconds_curr);
  Serial.print(", ");
  Serial.print(relay2.seconds_curr);
  Serial.print(", ");
  Serial.print(relay3.seconds_curr);
  Serial.print(", ");
  Serial.print(relay4.seconds_curr);
  Serial.println();
}

// -----------------------
// -------- OXYGEN -------
// -----------------------
void oxygen_manager()
{
  if (oxygen.state_curr) digitalWrite(RELAY5, HIGH);
  else digitalWrite(RELAY5, LOW);
}

// -----------------------
// -------- INPUT --------
// -----------------------
void input_manager()
{
  din1.state_curr = digitalRead(IN1);

  if (din1.state_prev != din1.state_curr)
  {
    din1.state_prev = din1.state_curr;
      Serial.println(din1.state_curr);

    if (!din1.state_curr)
    {
      oxygen.state_curr = 1;
      millis_delayed_start_curr = millis();
    }
    else
    {
      oxygen.state_curr = 0;
    }
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
