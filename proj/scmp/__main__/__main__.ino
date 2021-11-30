#include <Wire.h>
#include "RTClib.h"
RTC_DS3231 rtc_ds3231;
DateTime now;

uint8_t history_update_screen = 0;

typedef struct history_t
{
  int16_t second_buff[60];
  int16_t minute_buff[60];
  int16_t hour_buff[60];

  int16_t hour_buff_hour[60];

  int16_t minute_buff_day[60];
  int8_t minute_buff_month[60];
  int16_t minute_buff_year[60];
} history_t;
history_t history = {};

#define SENSOR_COMMUNICATION_TIMEOUT 5000
typedef struct sensor_t
{
  int16_t ppb;
  int16_t ppb_prev;

  int8_t is_communicating;
  uint32_t is_communicating_timer;
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
  uint8_t refresh;
  int8_t screen_history_refresh_clock;
  int8_t screen_all_refresh_sensor_icon;

  int8_t history_page;

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
uint8_t nextion_history_prev[BUFFER_SIZE] = {101, 2, 10, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_history_next[BUFFER_SIZE] = {101, 2, 11, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_history_debug[BUFFER_SIZE] = {101, 2, 13, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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
  sensor1_check_communication();
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
void sensor1_check_communication()
{
  if (millis() - is_communicating_timer >= SENSOR_COMMUNICATION_TIMEOUT)
  {
    is_communicating_timer = millis();
    sensor1.is_communicating = 0;
    nextion.screen_all_refresh_sensor_icon = 1;
  }
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
        
        is_communicating_timer = millis();
        sensor1.is_communicating = 1;
        nextion.screen_all_refresh_sensor_icon = 1;
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
        sensor1.is_communicating = 1;
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
  if (nextion.refresh ||
      sensor1.ppb_prev != sensor1.ppb ||
      sensor2.ppb_prev != sensor2.ppb)
  {
    nextion.refresh = 0;
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

void nextion_screen_history_update_clock()
{
}
void nextion_screen_history_manager()
{
  if (nextion.refresh ||
      history_update_screen ||
      nextion.screen_history_refresh_clock)
  {
    nextion.refresh = 0;
    history_update_screen = 0;
    {
      /* TODO: Must change 6 to 60 */
#define NUM_PAGES 6
      for (int i = 0; i < NUM_PAGES; i++)
      {
        uint8_t page_index = i + (nextion.history_page * NUM_PAGES);
        uint8_t buff[] = {0x74, 0x30, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x3F, 0x3F, 0x2E, 0x3F, 0x3F, 0x3F, 0x20, 0x50, 0x50, 0x4D, 0x20, 0x20, 0x20, 0x3F, 0x3F, 0x3A, 0x3F, 0x3F, 0x20, 0x20, 0x20, 0x3F, 0x3F, 0x2F, 0x3F, 0x2F, 0x2F, 0x3F, 0x2F, 0x3F, 0x3F, 0x22, 0xff, 0xff, 0xff};
        buff[1] = (i) + 0x30;
        buff[8] = (history.hour_buff[page_index] % 100000 / 10000) + 0x30;
        buff[9] = (history.hour_buff[page_index] % 10000 / 1000) + 0x30;
        buff[11] = (history.hour_buff[page_index] % 1000 / 100) + 0x30;
        buff[12] = (history.hour_buff[page_index] % 100 / 10) + 0x30;
        buff[13] = (history.hour_buff[page_index] % 10 / 1) + 0x30;

        buff[21] = (history.hour_buff_hour[page_index] % 100 / 10) + 0x30;
        buff[22] = (history.hour_buff_hour[page_index] % 10 / 1) + 0x30;

        buff[24] = (0) + 0x30;
        buff[25] = (0) + 0x30;

        buff[29] = (history.minute_buff_year[page_index] % 10000 / 1000) + 0x30;
        buff[30] = (history.minute_buff_year[page_index] % 1000 / 100) + 0x30;
        buff[31] = (history.minute_buff_year[page_index] % 100 / 10) + 0x30;
        buff[32] = (history.minute_buff_year[page_index] % 10 / 1) + 0x30;

        buff[34] = (history.minute_buff_month[page_index] % 100 / 10) + 0x30;
        buff[35] = (history.minute_buff_month[page_index] % 10 / 1) + 0x30;

        buff[37] = (history.minute_buff_day[page_index] % 100 / 10) + 0x30;
        buff[38] = (history.minute_buff_day[page_index] % 10 / 1) + 0x30;

        nextion_exec_cmd(buff, sizeof(buff));
      }

      if (nextion.screen_history_refresh_clock)
      {
        uint8_t buff[] = {0x74, 0x36, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
        //uint8_t buff[] = {0x74, 0x36, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
        buff[8] = (rtc.hour_curr % 100 / 10) + 0x30;
        buff[9] = (rtc.hour_curr % 10 / 1) + 0x30;
        buff[11] = (rtc.minute_curr % 100 / 10) + 0x30;
        buff[12] = (rtc.minute_curr % 10 / 1) + 0x30;
        //buff[14] = (rtc.second_curr % 100 / 10) + 0x30;
        //buff[15] = (rtc.second_curr % 10 / 1) + 0x30;
        nextion_exec_cmd(buff, sizeof(buff));

        nextion.screen_history_refresh_clock = 0;
      }

      {
        uint8_t buff[] = {0x74, 0x37, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x50, 0x61, 0x67, 0x65, 0x20, 0x31, 0x2F, 0x36, 0x22, 0xff, 0xff, 0xff};
        buff[13] = (nextion.history_page + 1) + 0x30;
        buff[15] = (NUM_PAGES) + 0x30;
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
      else if (compare_array(nextion_history_prev, buffer_nextion))
      {
        if (nextion.history_page > 0) nextion.history_page--;
        else nextion.history_page = 0;
        history_update_screen = 1;
        //Serial.println(nextion.history_page);
      }
      else if (compare_array(nextion_history_next, buffer_nextion))
      {
        if (nextion.history_page < 5) nextion.history_page++;
        else nextion.history_page = 5;
        history_update_screen = 1;
        //Serial.println(nextion.history_page);
      }
#if 0
      else if (compare_array(nextion_history_debug, buffer_nextion))
      {
        rtc_ds3231.adjust(DateTime(rtc.year_curr, rtc.month_curr, rtc.day_curr, rtc.hour_curr, 59, 55));
      }
#endif
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
  nextion.refresh = 1;
  uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x31, 0xff, 0xff, 0xff};
  buff[9] = screen + 0x30;
  nextion_exec_cmd(buff, sizeof(buff));
}

// ***********************************************************************************************
// ;rtc
// ***********************************************************************************************
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

// ***********************************************************************************************
// ;history
// ***********************************************************************************************
void history_manager()
{
  history_second();
  history_minute();
  history_hour();
}

void history_second()
{
  if (rtc.second_prev != rtc.second_curr)
  {
    rtc.second_prev = rtc.second_curr;

    for (int i = 60 - 1; i > 0; i--) history.second_buff[i] = history.second_buff[i - 1];
    history.second_buff[0] = ppb_avg;

    nextion.screen_history_refresh_clock = 1;

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

void history_hour()
{
  if (rtc.hour_prev != rtc.hour_curr)
  {
    rtc.hour_prev = rtc.hour_curr;

    int32_t avg = 0;
    for (int i = 0; i < 60; i++) avg += history.minute_buff[i];
    avg /= 60;

    for (int i = 60 - 1; i > 0; i--) history.hour_buff[i] = history.hour_buff[i - 1];
    history.hour_buff[0] = avg;

    for (int i = 60 - 1; i > 0; i--) history.hour_buff_hour[i] = history.hour_buff_hour[i - 1];
    history.hour_buff_hour[0] = rtc.hour_curr;

    for (int i = 60 - 1; i > 0; i--) history.minute_buff_day[i] = history.minute_buff_day[i - 1];
    history.minute_buff_day[0] = rtc.day_curr;

    for (int i = 60 - 1; i > 0; i--) history.minute_buff_month[i] = history.minute_buff_month[i - 1];
    history.minute_buff_month[0] = rtc.month_curr;

    for (int i = 60 - 1; i > 0; i--) history.minute_buff_year[i] = history.minute_buff_year[i - 1];
    history.minute_buff_year[0] = rtc.year_curr;

    history_update_screen = 1;

#if 1
    Serial.println("**************************************************************");
    for (int i = 0; i < 60; i++)
    {
      Serial.print(history.hour_buff_hour[i]);
      Serial.print(",");
    }
    Serial.println();
    Serial.println("**************************************************************");
#endif
  }
}

// ***********************************************************************************************
// ;core
// ***********************************************************************************************
void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);

  if (!rtc_ds3231.begin()) {}
  else {}
  if (rtc_ds3231.lostPower())
  {
    rtc_ds3231.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  //rtc_ds3231.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //rtc_ds3231.adjust(DateTime(2014, 1, 21, 8, 59, 50));
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
