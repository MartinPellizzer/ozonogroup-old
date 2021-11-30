#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3);

#define RELAY_PIN 8

int16_t set_ozone_ppb = 1500;

// **********************************************************************
// ;system
// **********************************************************************
typedef struct sys_t
{
  int8_t power_is_on;
  int32_t countdown_timer;
  int32_t countdown_counter;
  int32_t countdown_millis;
  int16_t ppb_target;
  int16_t countdown_target;
} sys_t;
sys_t sys = {};

// **********************************************************************
// SENSORE
// **********************************************************************
typedef struct sensor_t
{
  int16_t ppb;
} sensor_t;
sensor_t sensor = {};

enum screens
{
  screen_splash,
  screen_home,
};
typedef struct nextion_t
{
  uint8_t listen_new_data_state;
  uint8_t listen_buffer_counter;
  uint32_t listen_current_millis;

  uint8_t screen;
  uint8_t refresh;

  int8_t power_refresh;
  int8_t ppb_refresh;
  int8_t countdown_refresh;
  int8_t set_ozone_ppb_refresh;
  int8_t ppb_target_refresh;
  int8_t countdown_target_refresh;
} nextion_t;
nextion_t nextion = {};

const uint8_t BUFFER_SIZE = 20;
uint8_t buffer_nextion[BUFFER_SIZE];
uint8_t nextion_home_power[BUFFER_SIZE] = {101, 1, 3, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_home_ppb_target_sub[BUFFER_SIZE] = {101, 1, 7, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_home_ppb_target_add[BUFFER_SIZE] = {101, 1, 8, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_home_countdown_target_sub[BUFFER_SIZE] = {101, 1, 9, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_home_countdown_target_add[BUFFER_SIZE] = {101, 1, 10, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// -----------------------------------------------------------------------------------
// ;sensor
// -----------------------------------------------------------------------------------
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
        nextion.ppb_refresh = 1;
      }
    }
  }
}

// -----------------------------------------------------------------------------------
// ;nextion
// -----------------------------------------------------------------------------------
bool compare_array(uint8_t *a, uint8_t *b)
{
  for (uint8_t i = 0; i < BUFFER_SIZE; i++) if (a[i] != b[i]) return false;
  return true;
}
bool nextion_send_command(uint8_t arr[], uint8_t arr_size)
{
  for (uint8_t i = 0; i < arr_size; i++) Serial.write(arr[i]);
}
void nextion_listen()
{
  if (Serial.available())
  {
    nextion.listen_new_data_state = true;
    buffer_nextion[nextion.listen_buffer_counter] = Serial.read();
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
  switch (nextion.screen)
  {
    case screen_home:
      if (compare_array(nextion_home_power, buffer_nextion))
      {
        if (sys.power_is_on)
        {
          sys.power_is_on = 0;
          nextion.power_refresh = 1;
          sys.countdown_counter = sys.countdown_timer;
          nextion.countdown_refresh = 1;
          digitalWrite(RELAY_PIN, LOW);
        }
        else
        {
          sys.power_is_on = 1;
          nextion.power_refresh = 1;
          digitalWrite(RELAY_PIN, HIGH);
        }
      }
      else if (compare_array(nextion_home_ppb_target_sub, buffer_nextion))
      {
        sys.ppb_target -= 100;
        if (sys.ppb_target < 0) sys.ppb_target = 0;
        nextion.ppb_target_refresh = 1;
      }
      else if (compare_array(nextion_home_ppb_target_add, buffer_nextion))
      {
        sys.ppb_target += 100;
        if (sys.ppb_target > 10000) sys.ppb_target = 10000;
        nextion.ppb_target_refresh = 1;
      }
      else if (compare_array(nextion_home_countdown_target_sub, buffer_nextion))
      {
        sys.countdown_target -= 15;
        if (sys.countdown_target < 0) sys.countdown_target = 0;
        nextion.countdown_target_refresh = 1;
      }
      else if (compare_array(nextion_home_countdown_target_add, buffer_nextion))
      {
        sys.countdown_target += 15;
        if (sys.countdown_target > 3 * 60) sys.countdown_target = 3 * 60;
        nextion.countdown_target_refresh = 1;
      }
      break;
  }
}
void nextion_replace_screen(screens screen)
{
  nextion.screen = screen;
  nextion.refresh = 1;
  uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x31, 0xff, 0xff, 0xff};
  buff[9] = screen + 0x30;
  nextion_send_command(buff, sizeof(buff));
}
void nextion_manage_screen_home()
{
  if (nextion.power_refresh)
  {
    nextion.power_refresh = 0;
    {
      // t5.txt="Power Off"
      uint8_t buff1[] = {0x74, 0x35, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x50, 0x6F, 0x77, 0x65, 0x72, 0x20, 0x4F, 0x66, 0x66, 0x22, 0xff, 0xff, 0xff};
      // t5.txt="Power On"
      uint8_t buff2[] = {0x74, 0x35, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x50, 0x6F, 0x77, 0x65, 0x72, 0x20, 0x4F, 0x6E, 0x22, 0xff, 0xff, 0xff};

      if (sys.power_is_on) nextion_send_command(buff2, sizeof(buff2));
      else nextion_send_command(buff1, sizeof(buff1));
    }
  }
  if (nextion.ppb_refresh)
  {
    nextion.ppb_refresh = 0;
    {
      uint8_t buff[] = {0x74, 0x30, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
      buff[8] = (sensor.ppb % 100000 / 10000) + 0x30;
      buff[9] = (sensor.ppb % 10000 / 1000) + 0x30;
      buff[11] = (sensor.ppb % 1000 / 100) + 0x30;
      buff[12] = (sensor.ppb % 100 / 10) + 0x30;
      //buff[13] = (sensor.ppb % 10 / 1) + 0x30;
      nextion_send_command(buff, sizeof(buff));
    }
  }
  if (nextion.set_ozone_ppb_refresh)
  {
    nextion.set_ozone_ppb_refresh = 0;
    {
      uint8_t buff[] = {0x74, 0x31, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
      buff[8] = (set_ozone_ppb % 100000 / 10000) + 0x30;
      buff[9] = (set_ozone_ppb % 10000 / 1000) + 0x30;
      buff[11] = (set_ozone_ppb % 1000 / 100) + 0x30;
      buff[12] = (set_ozone_ppb % 100 / 10) + 0x30;
      //buff[13] = (set_ozone_ppb % 10 / 1) + 0x30;
      nextion_send_command(buff, sizeof(buff));
    }
  }
  if (nextion.countdown_refresh)
  {
    nextion.countdown_refresh = 0;
    {
      uint8_t buff[] = {0x74, 0x32, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
      int8_t hrs = sys.countdown_counter / 3600;
      int8_t mins = sys.countdown_counter % 3600 / 60;
      int8_t secs = sys.countdown_counter % 60;
      buff[8] = (hrs % 100 / 10) + 0x30;
      buff[9] = (hrs % 10 / 1) + 0x30;
      buff[11] = (mins % 100 / 10) + 0x30;
      buff[12] = (mins % 10 / 1) + 0x30;
      buff[14] = (secs % 100 / 10) + 0x30;
      buff[15] = (secs % 10 / 1) + 0x30;
      nextion_send_command(buff, sizeof(buff));
    }
  }
  if (nextion.ppb_target_refresh)
  {
    nextion.ppb_target_refresh = 0;
    {
      uint8_t buff[] = {0x74, 0x33, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
      buff[8] = (sys.ppb_target % 100000 / 10000) + 0x30;
      buff[9] = (sys.ppb_target % 10000 / 1000) + 0x30;
      buff[11] = (sys.ppb_target % 1000 / 100) + 0x30;
      buff[12] = (sys.ppb_target % 100 / 10) + 0x30;
      nextion_send_command(buff, sizeof(buff));
    }
  }
  if (nextion.countdown_target_refresh)
  {
    nextion.countdown_target_refresh = 0;
    {
      uint8_t buff[] = {0x74, 0x34, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
      int8_t hrs = sys.countdown_target / 3600;
      int8_t mins = sys.countdown_target / 60;
      int8_t secs = sys.countdown_target % 60;
      buff[8] = (hrs % 100 / 10) + 0x30;
      buff[9] = (hrs % 10 / 1) + 0x30;
      buff[11] = (mins % 100 / 10) + 0x30;
      buff[12] = (mins % 10 / 1) + 0x30;
      buff[14] = (secs % 100 / 10) + 0x30;
      buff[15] = (secs % 10 / 1) + 0x30;
      nextion_send_command(buff, sizeof(buff));
    }
  }
}
void nextion_manager()
{
  nextion_listen();
  if (nextion.screen == screen_home) nextion_manage_screen_home();
}


// -----------------------------------------------------------------------------------
// ;cycle
// -----------------------------------------------------------------------------------
void cycle_manager()
{
  countdown_run();
  relay_manager();
}

// -----------------------------------------------------------------------------------
// ;relay
// -----------------------------------------------------------------------------------
void relay_manager()
{
  if (sys.power_is_on)
  {
    if (sensor.ppb < set_ozone_ppb)
    {
      digitalWrite(RELAY_PIN, HIGH);
    }
    else
    {
      digitalWrite(RELAY_PIN, LOW);
    }
  }
  else
  {
    digitalWrite(RELAY_PIN, LOW);
  }
}

// -----------------------------------------------------------------------------------
// ;countdown
// -----------------------------------------------------------------------------------
void countdown_run()
{
  if (sys.power_is_on)
  {
    if (millis() - sys.countdown_millis >= 1000)
    {
      sys.countdown_millis = millis();
      sys.countdown_counter--;
      nextion.countdown_refresh = 1;

      countdown_complete();
    }
  }
}
void countdown_complete()
{
  if (sys.countdown_counter < 0)
  {
    sys.countdown_counter = sys.countdown_timer;
    sys.power_is_on = 0;
    digitalWrite(RELAY_PIN, LOW);
  }
}

// -----------------------------------------------------------------------------------
// ;core
// -----------------------------------------------------------------------------------
void setup()
{
  sys.countdown_timer = sys.countdown_counter = 10;

  Serial.begin(9600);
  mySerial.begin(9600);

  digitalWrite(RELAY_PIN, LOW);
  pinMode(RELAY_PIN, OUTPUT);

  nextion_replace_screen(screen_splash);
  delay(2000);
  nextion_replace_screen(screen_home);

  nextion.countdown_refresh = 1;
  nextion.set_ozone_ppb_refresh = 1;
  nextion.ppb_target_refresh = 1;
  nextion.countdown_target_refresh = 1;
  nextion.power_refresh = 1;
}

void loop()
{
  nextion_manager();
  read_sensor();
  cycle_manager();
}
