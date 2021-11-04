#include <EEPROM.h>

#define PIN_IN_START_CYCLE      24
#define PIN_OUT_GENO3           25
#define PIN_OUT_STATUS_LED      26
#define PIN_OUT_END_CYCLE       27
#define PIN_OUT_ALARM1          28
#define PIN_OUT_ALARM2          29
#define PIN_OUT_DATA            6

uint8_t is_on_temp = 0;
uint8_t is_on_current = 0;
uint8_t is_on_old = 0;

uint8_t o3_gen_cycle_state_current = 0;
uint8_t o3_gen_cycle_state_old = 0;
uint8_t o3_gen_cycle_direction_current = 0;
uint8_t o3_gen_cycle_direction_old = 0;

unsigned long o3_gen_cycle_current_millis = 0;

const unsigned long initial_countdown = 1200000;


int WORKING_MAX_ATTEMPTS = 5;

int s1_ppb_current = 0;
int s1_ppb_old = 0;
int s1_min_current = 0;
int s1_min_temp = 0;
int s1_min_old = 0;
int s1_max_current = 500;
int s1_max_temp = 500;
int s1_max_old = 500;
unsigned long s1_time_current = 1200000;
unsigned long s1_time_temp = 1200000;
unsigned long s1_time_old = 1200000;
long s1_time_countdown = 1200000;
uint8_t s1_color_current = 0;
uint8_t s1_color_old = 0;
int s1_working_current_attempts = WORKING_MAX_ATTEMPTS;

int s2_ppb_current = 0;
int s2_ppb_old = 0;
int s2_min_current = 4500;
int s2_min_temp = 4500;
int s2_min_old = 4500;
int s2_max_current = 5000;
int s2_max_temp = 5000;
int s2_max_old = 5000;
unsigned long s2_time_current = 1200000;
unsigned long s2_time_temp = 1200000;
unsigned long s2_time_old = 1200000;
long s2_time_countdown = 1200000;
uint8_t s2_color_current = 0;
uint8_t s2_color_old = 0;
int s2_working_current_attempts = WORKING_MAX_ATTEMPTS;

int s3_ppb_current = 0;
int s3_ppb_old = 0;
int s3_min_current = 0;
int s3_min_temp = 0;
int s3_min_old = 0;
int s3_max_current = 500;
int s3_max_temp = 500;
int s3_max_old = 500;
unsigned long s3_time_current = 1200000;
unsigned long s3_time_temp = 1200000;
unsigned long s3_time_old = 1200000;
long s3_time_countdown = 1200000;
uint8_t s3_color_current = 0;
uint8_t s3_color_old = 0;
int s3_working_current_attempts = WORKING_MAX_ATTEMPTS;

uint8_t page_current = 0;
uint8_t page_old = 0;

uint8_t s1_alarm_current = 0;
uint8_t s1_alarm_old = 0;
uint8_t s3_alarm_current = 0;
uint8_t s3_alarm_old = 0;

uint8_t start_countdown = 0;

/* dummy stuff */
unsigned long second_current_millis = 0;
uint8_t test_counter_current = 20;
uint8_t test_counter_old = 20;
uint8_t dir = 1;

const unsigned long O3_CYCLE_WORKING_TIMER_MILLIS = 2700000;
const unsigned long O3_CYCLE_RESTING_TIMER_MILLIS = 900000;

uint8_t s1_settings_current = 1;
uint8_t s1_settings_old = 1;
uint8_t s2_settings_current = 1;
uint8_t s2_settings_old = 1;
uint8_t s3_settings_current = 1;
uint8_t s3_settings_old = 1;

bool is_cycle_input_on_prev = false;
bool is_cycle_input_on = false;

byte onoff_current = 0;
byte onoff_old = 255;
unsigned long onoff_millis_current = 0;
byte onoff_debouncing = 0;

unsigned long second_millis_current = 0;

bool first_off = false;

void setup()
{
  pinMode(PIN_IN_START_CYCLE, INPUT_PULLUP);

  digitalWrite(PIN_OUT_GENO3, LOW);
  pinMode(PIN_OUT_GENO3, OUTPUT);

  digitalWrite(PIN_OUT_STATUS_LED, LOW);
  pinMode(PIN_OUT_STATUS_LED, OUTPUT);

  digitalWrite(PIN_OUT_END_CYCLE, LOW);
  pinMode(PIN_OUT_END_CYCLE, OUTPUT);

  digitalWrite(PIN_OUT_ALARM1, LOW);
  pinMode(PIN_OUT_ALARM1, OUTPUT);

  digitalWrite(PIN_OUT_ALARM2, LOW);
  pinMode(PIN_OUT_ALARM2, OUTPUT);

  analogWrite(PIN_OUT_DATA, 0);
  pinMode(PIN_OUT_DATA, OUTPUT);

  delay(1000);
  Serial.begin(9600);
  delay(1000);
  Serial1.begin(9600);
  delay(1000);
  Serial2.begin(9600);
  delay(1000);
  Serial3.begin(9600);
  delay(1000);

  s2_time_current = initial_countdown;
  s2_time_temp = initial_countdown;
  s2_time_old = initial_countdown;
  s2_time_countdown = initial_countdown;
  
  manageEEPROM();
  delay(1000);

  page_current = 1;
}

void loop()
{
  if((millis() - second_millis_current) > 1000)
  {
      second_millis_current = millis();
      DataLoggerHandler();

      if(s1_working_current_attempts <= WORKING_MAX_ATTEMPTS * 2) s1_working_current_attempts += 1;
      if(s2_working_current_attempts <= WORKING_MAX_ATTEMPTS * 2) s2_working_current_attempts += 1;
      if(s3_working_current_attempts <= WORKING_MAX_ATTEMPTS * 2) s3_working_current_attempts += 1;
  }
  
  SensorsHandler();
  AlarmHandler();
  
  OnOffHandler();
  CycleHandler();
  
  DataLoggerHandler();
  
  NextionHandler();
}
