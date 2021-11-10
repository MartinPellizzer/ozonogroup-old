#include <Wire.h>
#include "RTClib.h"
RTC_DS3231 rtc_ds3231;
DateTime now;

uint8_t history_update_screen = 0;

typedef struct history_t
{
  int16_t second_buff[60];
  int16_t minute_buff[60];
  int16_t minute_buff_time[60];
  int16_t minute_buff_date[60];
} history_t;
history_t history = {};

typedef struct sensor_t
{
  int16_t ppb;
  int16_t ppb_prev;
} sensor_t;
sensor_t sensor1 = {};
sensor_t sensor2 = {};
int16_t ppb_avg;

enum screens
{
  screen_splash,
  screen_realtime,
  screen_history,
};

typedef struct nextion_t
{
  uint8_t listen_new_data_state;
  uint8_t listen_buffer_counter;
  uint32_t listen_current_millis;

  uint8_t screen;
} nextion_t;
nextion_t nextion = {};


typedef struct rtc_t
{
  int16_t year_curr;
  int16_t year_prev;
  int8_t month_curr;
  int8_t month_prev;
  int8_t day_curr;
  int8_t day_prev;
  int8_t hour_curr;
  int8_t hour_prev;
  int8_t minute_curr;
  int8_t minute_prev;
  int8_t second_curr;
  int8_t second_prev;
} rtc_t;
rtc_t rtc = {};

const uint8_t BUFFER_SIZE = 20;
uint8_t buffer_nextion[BUFFER_SIZE];
uint8_t nextion_realtime_history[BUFFER_SIZE] = {101, 1, 5, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_history_realtime[BUFFER_SIZE] = {101, 2, 1, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

bool compare_array(uint8_t *a, uint8_t *b)
{
  for (uint8_t i = 0; i < BUFFER_SIZE; i++) if (a[i] != b[i]) return false;
  return true;
}

// ----------------------------------------------------------------------------------------
// ;sensors -------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
void sensor_manager()
{
  sensor1_read();
  sensor2_read();
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
void sensor1_read()
{
  if (Serial1.available() > 0)
  {
    byte buff[8];
    Serial1.readBytes(buff, 9);
    if (get_checksum(buff, 9) == buff[8])
    {
      int tmp = buff[4] * 256 + buff[5];
      if (tmp >= 0 && tmp <= 10000)
      {
        sensor1.ppb = tmp;
        sensor1.ppb_prev = -1;
        ppb_avg = (sensor1.ppb + sensor2.ppb) / 2;
      }
    }
  }
}
void sensor2_read()
{
  if (Serial2.available() > 0)
  {
    byte buff[8];
    Serial2.readBytes(buff, 9);
    if (get_checksum(buff, 9) == buff[8])
    {
      int tmp = buff[4] * 256 + buff[5];
      if (tmp >= 0 && tmp <= 10000)
      {
        sensor2.ppb = tmp;
        sensor2.ppb_prev = -1;
        ppb_avg = (sensor1.ppb + sensor2.ppb) / 2;
      }
    }
  }
}

// ----------------------------------------------------------------------------------------
// ;nextion -------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
void nextion_manager()
{
  nextion_listen();
  if (nextion.screen == screen_splash) nextion_screen_splash_manager();
  else if (nextion.screen == screen_realtime) nextion_screen_realtime_manager();
  else if (nextion.screen == screen_history) nextion_screen_history_manager();
}

void nextion_screen_splash_manager()
{
}

void nextion_screen_realtime_manager()
{
  if (sensor1.ppb_prev != sensor1.ppb ||
      sensor2.ppb_prev != sensor2.ppb)
  {
    sensor1.ppb_prev = sensor1.ppb;
    sensor2.ppb_prev = sensor2.ppb;
    {
      uint8_t buff[] = {0x74, 0x30, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x30, 0x20, 0x50, 0x50, 0x4D, 0x22, 0xff, 0xff, 0xff};
      buff[8] = (ppb_avg % 100000 / 10000) + 0x30;
      buff[9] = (ppb_avg % 10000 / 1000) + 0x30;
      buff[11] = (ppb_avg % 1000 / 100) + 0x30;
      buff[12] = (ppb_avg % 100 / 10) + 0x30;
      buff[13] = (ppb_avg % 10 / 1) + 0x30;
      nextion_exec_cmd(buff, sizeof(buff));
    }
    {
      uint8_t buff[] = {0x74, 0x31, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x30, 0x20, 0x50, 0x50, 0x4D, 0x22, 0xff, 0xff, 0xff};
      buff[8] = (sensor1.ppb % 100000 / 10000) + 0x30;
      buff[9] = (sensor1.ppb % 10000 / 1000) + 0x30;
      buff[11] = (sensor1.ppb % 1000 / 100) + 0x30;
      buff[12] = (sensor1.ppb % 100 / 10) + 0x30;
      buff[13] = (sensor1.ppb % 10 / 1) + 0x30;
      nextion_exec_cmd(buff, sizeof(buff));
    }
    {
      uint8_t buff[] = {0x74, 0x32, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x30, 0x20, 0x50, 0x50, 0x4D, 0x22, 0xff, 0xff, 0xff};
      buff[8] = (sensor2.ppb % 100000 / 10000) + 0x30;
      buff[9] = (sensor2.ppb % 10000 / 1000) + 0x30;
      buff[11] = (sensor2.ppb % 1000 / 100) + 0x30;
      buff[12] = (sensor2.ppb % 100 / 10) + 0x30;
      buff[13] = (sensor2.ppb % 10 / 1) + 0x30;
      nextion_exec_cmd(buff, sizeof(buff));
    }
    {
      uint8_t buff[] = {0x74, 0x33, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
      buff[8] = (rtc.hour_curr % 100 / 10) + 0x30;
      buff[9] = (rtc.hour_curr % 10 / 1) + 0x30;
      buff[11] = (rtc.minute_curr % 100 / 10) + 0x30;
      buff[12] = (rtc.minute_curr % 10 / 1) + 0x30;
      nextion_exec_cmd(buff, sizeof(buff));
    }
  }
}

void nextion_screen_history_manager()
{
  if (history_update_screen)
  {
    history_update_screen = 0;
    {
      /* TODO: Must change 6 to 60 */
      for (int i = 0; i < 6; i++)
      {
        uint8_t buff[] = {0x74, 0x30, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x3F, 0x3F, 0x2E, 0x3F, 0x3F, 0x3F, 0x20, 0x50, 0x50, 0x4D, 0x20, 0x20, 0x20, 0x3F, 0x3F, 0x2D, 0x3F, 0x3F, 0x20, 0x2F, 0x20, 0x3F, 0x3F, 0x22, 0xff, 0xff, 0xff};
        buff[1] = (i) + 0x30;
        buff[8] = (history.minute_buff[i] % 100000 / 10000) + 0x30;
        buff[9] = (history.minute_buff[i] % 10000 / 1000) + 0x30;
        buff[11] = (history.minute_buff[i] % 1000 / 100) + 0x30;
        buff[12] = (history.minute_buff[i] % 100 / 10) + 0x30;
        buff[13] = (history.minute_buff[i] % 10 / 1) + 0x30;

        int prev_time = 0;
        if (history.minute_buff_time[i] == 0) prev_time = 59;
        else prev_time = history.minute_buff_time[i] - 1;
        buff[21] = (prev_time % 100 / 10) + 0x30;
        buff[22] = (prev_time % 10 / 1) + 0x30;
        buff[24] = (history.minute_buff_time[i] % 100 / 10) + 0x30;
        buff[25] = (history.minute_buff_time[i] % 10 / 1) + 0x30;

        buff[29] = (history.minute_buff_date[i] % 100 / 10) + 0x30;
        buff[30] = (history.minute_buff_date[i] % 10 / 1) + 0x30;
        nextion_exec_cmd(buff, sizeof(buff));
      }
      {
        uint8_t buff[] = {0x74, 0x36, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
        buff[8] = (rtc.hour_curr % 100 / 10) + 0x30;
        buff[9] = (rtc.hour_curr % 10 / 1) + 0x30;
        buff[11] = (rtc.minute_curr % 100 / 10) + 0x30;
        buff[12] = (rtc.minute_curr % 10 / 1) + 0x30;
        nextion_exec_cmd(buff, sizeof(buff));
      }
    }
  }
}

void nextion_listen()
{
  if (Serial3.available())
  {
    nextion.listen_new_data_state = true;
    buffer_nextion[nextion.listen_buffer_counter] = Serial3.read();
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
    case screen_realtime:
      if (compare_array(nextion_realtime_history, buffer_nextion))
      {
        nextion_replace_screen(screen_history);
      }
      break;
    case screen_history:
      if (compare_array(nextion_history_realtime, buffer_nextion))
      {
        nextion_replace_screen(screen_realtime);
      }
      break;
  }
}

void nextion_exec_cmd(uint8_t *buff, uint8_t buff_size)
{
  for (uint8_t i = 0; i < buff_size; i++)
    Serial3.write(buff[i]);
}

void nextion_replace_screen(screens screen)
{
  nextion.screen = screen;
  uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x31, 0xff, 0xff, 0xff};
  buff[9] = screen + 0x30;
  nextion_exec_cmd(buff, sizeof(buff));
}



// rtc
void rtc_manager()
{
  now = rtc_ds3231.now();
  rtc.year_curr = now.year();
  rtc.month_curr = now.month();
  rtc.day_curr = now.day();
  rtc.hour_curr = now.hour();
  rtc.minute_curr = now.minute();
  rtc.second_curr = now.second();
}

void history_manager()
{
  history_second();
  history_minute();
}

void history_second()
{
  if (rtc.second_prev != rtc.second_curr)
  {
    rtc.second_prev = rtc.second_curr;

    for (int i = 60 - 1; i > 0; i--)
    {
      history.second_buff[i] = history.second_buff[i - 1];
    }
    history.second_buff[0] = ppb_avg;

#if 1
    Serial.println(rtc.second_curr);
#endif
#if 0
    for (int i = 0; i < 60; i++)
    {
      Serial.print(history.second_buff[i]);
      Serial.print(",");
    }
    Serial.println();
#endif
  }
}

void history_minute()
{
  if (rtc.minute_prev != rtc.minute_curr)
  {
    rtc.minute_prev = rtc.minute_curr;

    int32_t avg = 0;
    for (int i = 0; i < 60; i++) avg += history.second_buff[i];
    avg /= 60;

    for (int i = 60 - 1; i > 0; i--) history.minute_buff[i] = history.minute_buff[i - 1];
    history.minute_buff[0] = avg;

    for (int i = 60 - 1; i > 0; i--) history.minute_buff_time[i] = history.minute_buff_time[i - 1];
    history.minute_buff_time[0] = rtc.minute_curr;

    for (int i = 60 - 1; i > 0; i--) history.minute_buff_date[i] = history.minute_buff_date[i - 1];
    history.minute_buff_date[0] = rtc.day_curr;

    history_update_screen = 1;

#if 1
    for (int i = 0; i < 60; i++)
    {
      Serial.print(history.minute_buff[i]);
      Serial.print(",");
    }
    Serial.println();
#endif

  }
}

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);

  if (!rtc_ds3231.begin()) {}
  else {}
  if (rtc_ds3231.lostPower()) rtc_ds3231.adjust(DateTime(F(__DATE__), F(__TIME__)));
  now = rtc_ds3231.now();
  rtc.year_curr = rtc.year_prev = now.year();
  rtc.month_curr = rtc.month_prev = now.month();
  rtc.day_curr = rtc.day_prev = now.day();
  rtc.hour_curr = rtc.hour_prev = now.hour();
  rtc.minute_curr = rtc.minute_prev = now.minute();
  rtc.second_curr = rtc.second_prev = now.second();

  nextion_replace_screen(screen_splash);
  delay(2000);
  nextion_replace_screen(screen_realtime);
}

void loop()
{
  rtc_manager();

  sensor_manager();
  nextion_manager();

  history_manager();
}
