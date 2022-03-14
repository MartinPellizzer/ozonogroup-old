#include <Wire.h>
#include "RTClib.h"
RTC_DS3231 rtc_ds3231;
DateTime now;

#include <SPI.h>
#include <SD.h>
File file;

uint8_t history_update_screen = 0;

typedef struct sd_card_t
{
  int8_t state;
  int8_t tried_to_initialize;
  int8_t pin;
  int8_t pin_cd;
  int8_t is_inserted;
  int8_t is_inserted_prev;
  int8_t write_state;
  int16_t write_val;
} sd_card_t;
sd_card_t sd_card = {};

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

typedef struct sensor_t
{
  int16_t ppb;
  int16_t ppb_prev;

  int8_t is_communicating;
  int8_t is_communicating_old;
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
  screen_clock,
  screen_ymd,
  screen_hms,
};

typedef struct nextion_t
{
  uint8_t listen_new_data_state;
  uint8_t listen_buffer_counter;
  uint32_t listen_current_millis;

  uint8_t screen;
  uint8_t refresh;
  int8_t header_clock_refresh;
  int8_t header_sensor_icon_refresh;
  int8_t header_sd_icon_refresh;

  int8_t history_page;

  int8_t screen_ymd_y_refresh;
  int8_t screen_ymd_m_refresh;
  int8_t screen_ymd_d_refresh;

  int8_t screen_hms_h_refresh;
  int8_t screen_hms_m_refresh;
  int8_t screen_hms_s_refresh;
} nextion_t;
nextion_t nextion = {};

typedef struct rtc_t
{
  int16_t year_curr;
  int16_t year_prev;
  int16_t year_tmp;
  int8_t month_curr;
  int8_t month_prev;
  int8_t month_tmp;
  int8_t day_curr;
  int8_t day_prev;
  int8_t day_tmp;
  int8_t hour_curr;
  int8_t hour_prev;
  int8_t hour_tmp;
  int8_t minute_curr;
  int8_t minute_prev;
  int8_t minute_tmp;
  int8_t second_curr;
  int8_t second_prev;
  int8_t second_tmp;
} rtc_t;
rtc_t rtc = {};

const uint8_t BUFFER_SIZE = 20;
uint8_t buffer_nextion[BUFFER_SIZE];
uint8_t nextion_realtime_history[BUFFER_SIZE] = {101, 1, 5, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_realtime_clock[BUFFER_SIZE] = {101, 1, 9, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint8_t nextion_history_realtime[BUFFER_SIZE] = {101, 2, 1, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_history_clock[BUFFER_SIZE] = {101, 2, 16, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_history_prev[BUFFER_SIZE] = {101, 2, 9, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_history_next[BUFFER_SIZE] = {101, 2, 10, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_history_debug[BUFFER_SIZE] = {101, 2, 13, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint8_t nextion_clock_realtime[BUFFER_SIZE] = {101, 3, 3, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_clock_history[BUFFER_SIZE] = {101, 3, 4, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_clock_ymd[BUFFER_SIZE] = {101, 3, 6, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_clock_hms[BUFFER_SIZE] = {101, 3, 7, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint8_t nextion_ymd_prev[BUFFER_SIZE] = {101, 4, 4, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_ymd_y_sub[BUFFER_SIZE] = {101, 4, 5, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_ymd_y_add[BUFFER_SIZE] = {101, 4, 6, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_ymd_m_sub[BUFFER_SIZE] = {101, 4, 7, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_ymd_m_add[BUFFER_SIZE] = {101, 4, 8, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_ymd_d_sub[BUFFER_SIZE] = {101, 4, 9, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_ymd_d_add[BUFFER_SIZE] = {101, 4, 10, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint8_t nextion_hms_prev[BUFFER_SIZE] = {101, 5, 4, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_hms_h_sub[BUFFER_SIZE] = {101, 5, 5, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_hms_h_add[BUFFER_SIZE] = {101, 5, 6, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_hms_m_sub[BUFFER_SIZE] = {101, 5, 7, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_hms_m_add[BUFFER_SIZE] = {101, 5, 8, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_hms_s_sub[BUFFER_SIZE] = {101, 5, 9, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t nextion_hms_s_add[BUFFER_SIZE] = {101, 5, 10, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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
  if (millis() - sensor1.is_communicating_timer >= 5000)
  {
    sensor1.is_communicating = 0;
    if (sensor1.is_communicating_old != sensor1.is_communicating)
    {
      sensor1.is_communicating_old = sensor1.is_communicating;
      nextion.header_sensor_icon_refresh = 1;
    }
  }
  if (millis() - sensor2.is_communicating_timer >= 5000)
  {
    sensor2.is_communicating = 0;
    if (sensor2.is_communicating_old != sensor2.is_communicating)
    {
      sensor2.is_communicating_old = sensor2.is_communicating;
      nextion.header_sensor_icon_refresh = 1;
    }
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

        sensor1.is_communicating_timer = millis();
        sensor1.is_communicating = 1;

        if (sensor1.is_communicating_old != sensor1.is_communicating)
        {
          sensor1.is_communicating_old = sensor1.is_communicating;
          nextion.header_sensor_icon_refresh = 1;
        }
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

        sensor2.is_communicating_timer = millis();
        sensor2.is_communicating = 1;

        if (sensor2.is_communicating_old != sensor2.is_communicating)
        {
          sensor2.is_communicating_old = sensor2.is_communicating;
          nextion.header_sensor_icon_refresh = 1;
        }
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
  else if (nextion.screen == screen_realtime) nextion_screen_realtime_manager(0);
  else if (nextion.screen == screen_history) nextion_screen_history_manager(0);
  else if (nextion.screen == screen_clock) nextion_screen_clock_manager(0);
  else if (nextion.screen == screen_ymd) nextion_screen_ymd_manager(0);
  else if (nextion.screen == screen_hms) nextion_screen_hms_manager(0);
}

// ;nextion header ------------------------------------------------------------------------
void nextion_header_sensor_refresh()
{
  nextion.header_sensor_icon_refresh = 0;
  uint8_t buff[] = {0x70, 0x30, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x34, 0xff, 0xff, 0xff};
  buff[7] = (sensor1.is_communicating && sensor2.is_communicating) ? 0x36 : 0x37;
  nextion_exec_cmd(buff, sizeof(buff));
}
void nextion_header_sd_refresh()
{
  nextion.header_sd_icon_refresh = 0;
  uint8_t buff[] = {0x70, 0x31, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x36, 0xff, 0xff, 0xff};
  buff[7] = (sd_card.state) ? 0x38 : 0x39;
  nextion_exec_cmd(buff, sizeof(buff));
}
void nextion_header_clock_refresh()
{
  uint8_t buff[] = {0x68, 0x30, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
  buff[8] = (rtc.hour_curr % 100 / 10) + 0x30;
  buff[9] = (rtc.hour_curr % 10 / 1) + 0x30;
  buff[11] = (rtc.minute_curr % 100 / 10) + 0x30;
  buff[12] = (rtc.minute_curr % 10 / 1) + 0x30;
  nextion_exec_cmd(buff, sizeof(buff));
}

void nextion_screen_splash_manager()
{
}

void nextion_screen_realtime_manager(uint8_t refresh)
{
  if (refresh)
  {
    nextion.screen = screen_realtime;
    uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x31, 0xff, 0xff, 0xff};
    nextion_exec_cmd(buff, sizeof(buff));
  }
  if (refresh || sensor1.ppb_prev != sensor1.ppb)
  {
    sensor1.ppb_prev = sensor1.ppb;
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
      uint8_t buff[] = {0x74, 0x30, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x30, 0x20, 0x50, 0x50, 0x4D, 0x22, 0xff, 0xff, 0xff};
      buff[8] = (ppb_avg % 100000 / 10000) + 0x30;
      buff[9] = (ppb_avg % 10000 / 1000) + 0x30;
      buff[11] = (ppb_avg % 1000 / 100) + 0x30;
      buff[12] = (ppb_avg % 100 / 10) + 0x30;
      buff[13] = (ppb_avg % 10 / 1) + 0x30;
      nextion_exec_cmd(buff, sizeof(buff));
    }
  }
  if (refresh || sensor2.ppb_prev != sensor2.ppb)
  {
    sensor2.ppb_prev = sensor2.ppb;
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
      uint8_t buff[] = {0x74, 0x30, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x30, 0x20, 0x50, 0x50, 0x4D, 0x22, 0xff, 0xff, 0xff};
      buff[8] = (ppb_avg % 100000 / 10000) + 0x30;
      buff[9] = (ppb_avg % 10000 / 1000) + 0x30;
      buff[11] = (ppb_avg % 1000 / 100) + 0x30;
      buff[12] = (ppb_avg % 100 / 10) + 0x30;
      buff[13] = (ppb_avg % 10 / 1) + 0x30;
      nextion_exec_cmd(buff, sizeof(buff));
    }
  }
  if (refresh || nextion.header_sensor_icon_refresh) nextion_header_sensor_refresh();
  if (refresh || nextion.header_sd_icon_refresh) nextion_header_sd_refresh();
  if (refresh || nextion.header_clock_refresh) nextion_header_clock_refresh();
}

void nextion_screen_history_manager(uint8_t refresh)
{
  if (refresh)
  {
    nextion.screen = screen_history;
    uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x32, 0xff, 0xff, 0xff};
    nextion_exec_cmd(buff, sizeof(buff));
  }

  if (refresh || nextion.header_sensor_icon_refresh) nextion_header_sensor_refresh();
  if (refresh || nextion.header_sd_icon_refresh) nextion_header_sd_refresh();
  if (refresh || nextion.header_clock_refresh) nextion_header_clock_refresh();

  if (refresh || history_update_screen)
  {
    history_update_screen = 0;

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

    { // pagination
      uint8_t buff[] = {0x74, 0x37, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x50, 0x61, 0x67, 0x65, 0x20, 0x31, 0x2F, 0x36, 0x22, 0xff, 0xff, 0xff};
      buff[13] = (nextion.history_page + 1) + 0x30;
      buff[15] = (NUM_PAGES) + 0x30;
      nextion_exec_cmd(buff, sizeof(buff));
    }
  }
}
void nextion_screen_clock_manager(uint8_t refresh)
{
  if (refresh)
  {
    nextion.screen = screen_clock;
    uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x33, 0xff, 0xff, 0xff};
    nextion_exec_cmd(buff, sizeof(buff));
  }

  if (refresh || nextion.header_sensor_icon_refresh) nextion_header_sensor_refresh();
  if (refresh || nextion.header_sd_icon_refresh) nextion_header_sd_refresh();
  if (refresh || nextion.header_clock_refresh) nextion_header_clock_refresh();

  if (refresh)
  {
    { // y/m/d
      uint8_t buff[] = {0x74, 0x30, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x30, 0x30, 0x2F, 0x30, 0x30, 0x2F, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
      buff[8] = (rtc.year_curr % 10000 / 1000) + 0x30;
      buff[9] = (rtc.year_curr % 1000 / 100) + 0x30;
      buff[10] = (rtc.year_curr % 100 / 10) + 0x30;
      buff[11] = (rtc.year_curr % 10 / 1) + 0x30;
      buff[13] = (rtc.month_curr % 100 / 10) + 0x30;
      buff[14] = (rtc.month_curr % 10 / 1) + 0x30;
      buff[16] = (rtc.day_curr % 100 / 10) + 0x30;
      buff[17] = (rtc.day_curr % 10 / 1) + 0x30;
      nextion_exec_cmd(buff, sizeof(buff));
    }
    { // h/m/s
      uint8_t buff[] = {0x74, 0x31, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2F, 0x30, 0x30, 0x2F, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
      buff[8] = (rtc.hour_curr % 100 / 10) + 0x30;
      buff[9] = (rtc.hour_curr % 10 / 1) + 0x30;
      buff[11] = (rtc.minute_curr % 100 / 10) + 0x30;
      buff[12] = (rtc.minute_curr % 10 / 1) + 0x30;
      buff[14] = (rtc.second_curr % 100 / 10) + 0x30;
      buff[15] = (rtc.second_curr % 10 / 1) + 0x30;
      nextion_exec_cmd(buff, sizeof(buff));
    }
  }
}
void nextion_screen_ymd_manager(uint8_t refresh)
{
  if (refresh)
  {
    nextion.screen = screen_ymd;
    uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x34, 0xff, 0xff, 0xff};
    nextion_exec_cmd(buff, sizeof(buff));
  }

  if (refresh || nextion.header_sensor_icon_refresh) nextion_header_sensor_refresh();
  if (refresh || nextion.header_sd_icon_refresh) nextion_header_sd_refresh();
  if (refresh || nextion.header_clock_refresh) nextion_header_clock_refresh();

  if (refresh ||
      nextion.screen_ymd_y_refresh ||
      nextion.screen_ymd_m_refresh ||
      nextion.screen_ymd_d_refresh)
  {
    nextion.screen_ymd_y_refresh = 0;
    nextion.screen_ymd_m_refresh = 0;
    nextion.screen_ymd_d_refresh = 0;
    { // yyyy
      uint8_t buff[] = {0x74, 0x30, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
      buff[8] = (rtc.year_tmp % 10000 / 1000) + 0x30;
      buff[9] = (rtc.year_tmp % 1000 / 100) + 0x30;
      buff[10] = (rtc.year_tmp % 100 / 10) + 0x30;
      buff[11] = (rtc.year_tmp % 10 / 1) + 0x30;
      nextion_exec_cmd(buff, sizeof(buff));
    }
    { // mm
      uint8_t buff[] = {0x74, 0x31, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
      buff[8] = (rtc.month_tmp % 100 / 10) + 0x30;
      buff[9] = (rtc.month_tmp % 10 / 1) + 0x30;
      nextion_exec_cmd(buff, sizeof(buff));
    }
    { // dd
      uint8_t buff[] = {0x74, 0x32, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
      buff[8] = (rtc.day_tmp % 100 / 10) + 0x30;
      buff[9] = (rtc.day_tmp % 10 / 1) + 0x30;
      nextion_exec_cmd(buff, sizeof(buff));
    }
  }
}
void nextion_screen_hms_manager(uint8_t refresh)
{
  if (refresh)
  {
    nextion.screen = screen_hms;
    uint8_t buff[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x61, 0x67, 0x65, 0x35, 0xff, 0xff, 0xff};
    nextion_exec_cmd(buff, sizeof(buff));
  }

  if (refresh || nextion.header_sensor_icon_refresh) nextion_header_sensor_refresh();
  if (refresh || nextion.header_sd_icon_refresh) nextion_header_sd_refresh();
  if (refresh || nextion.header_clock_refresh) nextion_header_clock_refresh();

  if (refresh ||
      nextion.screen_hms_h_refresh ||
      nextion.screen_hms_m_refresh ||
      nextion.screen_hms_s_refresh)
  {
    nextion.screen_hms_h_refresh = 0;
    nextion.screen_hms_m_refresh = 0;
    nextion.screen_hms_s_refresh = 0;
    { // hh
      uint8_t buff[] = {0x74, 0x30, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
      buff[8] = (rtc.hour_tmp % 100 / 10) + 0x30;
      buff[9] = (rtc.hour_tmp % 10 / 1) + 0x30;
      nextion_exec_cmd(buff, sizeof(buff));
    } // mm
    {
      uint8_t buff[] = {0x74, 0x31, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
      buff[8] = (rtc.minute_tmp % 100 / 10) + 0x30;
      buff[9] = (rtc.minute_tmp % 10 / 1) + 0x30;
      nextion_exec_cmd(buff, sizeof(buff));
    }
    { // ss
      uint8_t buff[] = {0x74, 0x32, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
      buff[8] = (rtc.second_tmp % 100 / 10) + 0x30;
      buff[9] = (rtc.second_tmp % 10 / 1) + 0x30;
      nextion_exec_cmd(buff, sizeof(buff));
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
      if (compare_array(nextion_realtime_history, buffer_nextion)) nextion_screen_history_manager(1);
      else if (compare_array(nextion_realtime_clock, buffer_nextion)) nextion_screen_clock_manager(1);
      break;
    case screen_history:
      if (compare_array(nextion_history_realtime, buffer_nextion)) nextion_screen_realtime_manager(1);
      else if (compare_array(nextion_history_clock, buffer_nextion)) nextion_screen_clock_manager(1);
      else if (compare_array(nextion_history_prev, buffer_nextion))
      {
        nextion.history_page = (nextion.history_page > 0) ? nextion.history_page - 1 : 0;
        history_update_screen = 1;
      }
      else if (compare_array(nextion_history_next, buffer_nextion))
      {
        nextion.history_page = (nextion.history_page < 5) ? nextion.history_page + 1 : 5;
        history_update_screen = 1;
      }
#if 0
      else if (compare_array(nextion_history_debug, buffer_nextion))
      {
        rtc_ds3231.adjust(DateTime(rtc.year_curr, rtc.month_curr, rtc.day_curr, rtc.hour_curr, 59, 55));
      }
#endif
      break;
    case screen_clock:
      if (compare_array(nextion_clock_realtime, buffer_nextion)) nextion_screen_realtime_manager(1);
      else if (compare_array(nextion_clock_history, buffer_nextion)) nextion_screen_history_manager(1);
      else if (compare_array(nextion_clock_ymd, buffer_nextion))
      {
        rtc.year_tmp = rtc.year_curr;
        rtc.month_tmp = rtc.month_curr;
        rtc.day_tmp = rtc.day_curr;
        nextion_screen_ymd_manager(1);
      }
      else if (compare_array(nextion_clock_hms, buffer_nextion))
      {
        rtc.hour_tmp = rtc.hour_curr;
        rtc.minute_tmp = rtc.minute_curr;
        rtc.second_tmp = rtc.second_curr;
        nextion_screen_hms_manager(1);
      }
      break;
    case screen_ymd:
      if (compare_array(nextion_ymd_prev, buffer_nextion))
      {
        rtc.year_curr = rtc.year_tmp;
        rtc.month_curr = rtc.month_tmp;
        rtc.day_curr = rtc.day_tmp;
        rtc_ds3231.adjust(DateTime(rtc.year_curr, rtc.month_curr, rtc.day_curr, rtc.hour_curr, rtc.minute_curr, rtc.second_curr));
        nextion_screen_clock_manager(1);
      }
      else if (compare_array(nextion_ymd_y_sub, buffer_nextion))
      {
        rtc.year_tmp--;
        nextion.screen_ymd_y_refresh = 1;
      }
      else if (compare_array(nextion_ymd_y_add, buffer_nextion))
      {
        rtc.year_tmp++;
        nextion.screen_ymd_y_refresh = 1;
      }
      else if (compare_array(nextion_ymd_m_sub, buffer_nextion))
      {
        rtc.month_tmp = (rtc.month_tmp - 1 < 1) ? 1 : rtc.month_tmp - 1;
        rtc.day_tmp = 1;
        nextion.screen_ymd_m_refresh = 1;
      }
      else if (compare_array(nextion_ymd_m_add, buffer_nextion))
      {
        rtc.month_tmp = (rtc.month_tmp + 1 > 12) ? 12 : rtc.month_tmp + 1;
        rtc.day_tmp = 1;
        nextion.screen_ymd_m_refresh = 1;
      }
      else if (compare_array(nextion_ymd_d_sub, buffer_nextion))
      {
        rtc.day_tmp = (rtc.day_tmp - 1 < 1) ? 1 : rtc.day_tmp - 1;
        nextion.screen_ymd_d_refresh = 1;
      }
      else if (compare_array(nextion_ymd_d_add, buffer_nextion))
      {
        int8_t num_days_tmp;
        if (rtc.month_tmp == 2) num_days_tmp = 28;
        else if (rtc.month_tmp == 4 || rtc.month_tmp == 6 || rtc.month_tmp == 9 || rtc.month_tmp == 11) num_days_tmp = 30;
        else num_days_tmp = 31;
        rtc.day_tmp = (rtc.day_tmp + 1 > num_days_tmp) ? num_days_tmp : rtc.day_tmp + 1;
        nextion.screen_ymd_d_refresh = 1;
      }
      break;
    case screen_hms:
      if (compare_array(nextion_hms_prev, buffer_nextion))
      {
        rtc.hour_curr = rtc.hour_tmp;
        rtc.minute_curr = rtc.minute_tmp;
        rtc.second_curr = rtc.second_tmp;
        rtc_ds3231.adjust(DateTime(rtc.year_curr, rtc.month_curr, rtc.day_curr, rtc.hour_curr, rtc.minute_curr, rtc.second_curr));
        nextion_screen_clock_manager(1);
      }
      else if (compare_array(nextion_hms_h_sub, buffer_nextion))
      {
        rtc.hour_tmp = (rtc.hour_tmp - 1 < 0) ? 0 : rtc.hour_tmp - 1;
        nextion.screen_hms_h_refresh = 1;
      }
      else if (compare_array(nextion_hms_h_add, buffer_nextion))
      {
        rtc.hour_tmp = (rtc.hour_tmp + 1 > 23) ? 23 : rtc.hour_tmp + 1;
        nextion.screen_hms_h_refresh = 1;
      }
      else if (compare_array(nextion_hms_m_sub, buffer_nextion))
      {
        rtc.minute_tmp = (rtc.minute_tmp - 1 < 0) ? 0 : rtc.minute_tmp - 1;
        nextion.screen_hms_m_refresh = 1;
      }
      else if (compare_array(nextion_hms_m_add, buffer_nextion))
      {
        rtc.minute_tmp = (rtc.minute_tmp + 1 > 59) ? 59 : rtc.minute_tmp + 1;
        nextion.screen_hms_m_refresh = 1;
      }
      else if (compare_array(nextion_hms_s_sub, buffer_nextion))
      {
        rtc.second_tmp = (rtc.second_tmp - 1 < 0) ? 0 : rtc.second_tmp - 1;
        nextion.screen_hms_s_refresh = 1;
      }
      else if (compare_array(nextion_hms_s_add, buffer_nextion))
      {
        rtc.second_tmp = (rtc.second_tmp + 1 > 59) ? 59 : rtc.second_tmp + 1;
        nextion.screen_hms_s_refresh = 1;
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

    nextion.header_clock_refresh = 1;

#if 0
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

    sd_card.write_state = 1;
    sd_card.write_val = avg;

#if 0
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

#if 0
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



// ----------------------------------------------------------------------------------------------------------
// ;sd
// ----------------------------------------------------------------------------------------------------------
void sd_manager()
{
  sd_card_init();
  sd_card_wtite();
}
void sd_card_init()
{
  sd_card.is_inserted = (digitalRead(sd_card.pin_cd) == LOW) ? 1 : 0;
  if (sd_card.is_inserted_prev != sd_card.is_inserted)
  {
    sd_card.is_inserted_prev = sd_card.is_inserted;
    if (sd_card.is_inserted)
    {
      if (!sd_card.tried_to_initialize)
      {
        sd_card.tried_to_initialize = 1;
        sd_card.state = (SD.begin(sd_card.pin)) ? 1 : 0;

        nextion.header_sd_icon_refresh = 1;
      }
    }
    else
    {
      sd_card.tried_to_initialize = 0;
      sd_card.state = 0;
      nextion.header_sd_icon_refresh = 1;
    }
  }
}
void sd_card_wtite()
{
  if (sd_card.write_state)
  {
    sd_card.write_state = 0;
    if (sd_card.state == 1)
    {
      file = SD.open("history.csv", FILE_WRITE);
      file.print(String(rtc.year_curr));
      file.write(',');
      file.print(String(rtc.month_curr));
      file.write(',');
      file.print(String(rtc.day_curr));
      file.write(',');
      file.print(String(rtc.hour_curr));
      file.write(',');
      file.print(String(rtc.minute_curr));
      file.write(',');
      file.print(String(rtc.second_curr));
      file.write(',');
      file.print(String(sd_card.write_val));
      file.write(',');
      file.write('\n');
      file.close();
    }
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

  // ************************
  // ;rtc init
  // ************************
  if (!rtc_ds3231.begin()) {}
  else {}
  if (rtc_ds3231.lostPower()) rtc_ds3231.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //rtc_ds3231.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //rtc_ds3231.adjust(DateTime(2014, 1, 21, 8, 59, 50));
  now = rtc_ds3231.now();
  rtc.year_curr = rtc.year_prev = now.year();
  rtc.month_curr = rtc.month_prev = now.month();
  rtc.day_curr = rtc.day_prev = now.day();
  rtc.hour_curr = rtc.hour_prev = now.hour();
  rtc.minute_curr = rtc.minute_prev = now.minute();
  rtc.second_curr = rtc.second_prev = now.second();

  // *************************
  // ;sd init
  // *************************
  sd_card.pin = 4;
  sd_card.pin_cd = 3;

  nextion_replace_screen(screen_splash);
  delay(2000);
  nextion_screen_realtime_manager(1);

  nextion.header_sd_icon_refresh = 1;
}



void loop()
{
  sd_manager();
  rtc_manager();

  sensor_manager();

  history_manager();

  nextion_manager();
}
