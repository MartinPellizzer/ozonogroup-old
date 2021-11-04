/* Sensor 1 */
const uint8_t S1_BUFFER_SIZE = 20;
uint8_t s1_buffer[S1_BUFFER_SIZE];
bool s1_new_data = false;
unsigned long s1_current_millis = 0;
uint8_t s1_buffer_counter = 0;

/* Sensor 2 */
const uint8_t S2_BUFFER_SIZE = 20;
uint8_t s2_buffer[S2_BUFFER_SIZE];
bool s2_new_data = false;
unsigned long s2_current_millis = 0;
uint8_t s2_buffer_counter = 0;

/* Sensor 3 */
const uint8_t S3_BUFFER_SIZE = 20;
uint8_t s3_buffer[S3_BUFFER_SIZE];
bool s3_new_data = false;
unsigned long s3_current_millis = 0;
uint8_t s3_buffer_counter = 0;

void SensorsHandler()
{
  updateSensorsVal();
  checkSensorMain();
  checkSensorsAlarm();
  checkSensorsWorking();
}

void updateSensorsVal() {
  getSensor1Val();
  getSensor2Val();
  getSensor3Val();
}

void getSensor1Val() {
  if (Serial2.available() > 0) {
    byte s1_measure[8];
    Serial2.readBytes(s1_measure, 9);
    if (FucCheckSum(s1_measure, 9) == s1_measure[8]) {
      int _temp = s1_measure[4] * 256 + s1_measure[5];
      if (_temp >= 0 && _temp <= 10000) {
        s1_ppb_current = _temp;
        s1_ppb_old = -1;
        s1_working_current_attempts = 0;
      }
    }
  }
}

void getSensor2Val() {
  if (Serial1.available() > 0) {
    byte s2_measure[8];
    Serial1.readBytes(s2_measure, 9);
    if (FucCheckSum(s2_measure, 9) == s2_measure[8]) {
      int _temp = s2_measure[4] * 256 + s2_measure[5];
      if (_temp >= 0 && _temp <= 10000) {
        s2_ppb_current = _temp;
        s2_ppb_old = -1;
        s2_working_current_attempts = 0;
      }
    }
  }
}

void getSensor3Val() {
  if (Serial3.available() > 0) {
    byte s3_measure[8];
    Serial3.readBytes(s3_measure, 9);
    if (FucCheckSum(s3_measure, 9) == s3_measure[8]) {
      int _temp = s3_measure[4] * 256 + s3_measure[5];
      if (_temp >= 0 && _temp <= 10000) {
        s3_ppb_current = _temp;
        s3_ppb_old = -1;
        s3_working_current_attempts = 0;
      }
    }
  }
}

void checkSensorsAlarm()
{
  if (s1_ppb_current == -2)
  {
    s1_alarm_current = 1;
    s1_color_current = 0;
  }
  else if (s1_ppb_current > s1_max_current)
  {
    s1_alarm_current = 1;
    s1_color_current = 1;
  }
  else
  {
    s1_alarm_current = 0;
    s1_color_current = 0;
  }

  
  if (s3_ppb_current == -2)
  {
    s3_alarm_current = 1;
    s3_color_current = 0;
  }
  else if (s3_ppb_current > s3_max_current)
  {
    s3_alarm_current = 1;
    s3_color_current = 1;
  }
  else
  {
    s3_alarm_current = 0;
    s3_color_current = 0;
  }
}

void checkSensorsWorking()
{
  if (s1_working_current_attempts >= WORKING_MAX_ATTEMPTS)
  {
    s1_ppb_current = -2;
    s1_ppb_old = -1;
  }
  if (s2_working_current_attempts >= WORKING_MAX_ATTEMPTS)
  {
    s2_ppb_current = -2;
    s2_ppb_old = -1;
  }
  if (s3_working_current_attempts >= WORKING_MAX_ATTEMPTS)
  {
    s3_ppb_current = -2;
    s3_ppb_old = -1;
  }
}

void checkSensorMain()
{
  if (s2_ppb_current > s2_max_current)
    o3_gen_cycle_direction_current = 0;
  else if (s2_ppb_current < s2_min_current)
    o3_gen_cycle_direction_current = 1;

  if (s2_ppb_current > s2_min_current)
    start_countdown = 1;

  if (s2_ppb_current <= s2_max_current && s2_ppb_current >= s2_min_current)
    s2_color_current = 1;
  else
    s2_color_current = 0;
}





/*
  void getSensor1Val() {
  if (s1_new_data) {
    if ((millis() - s1_current_millis) > 10) {
      s1_new_data = false;
      s1_buffer_counter = 0;

      s1_ppb = (s1_buffer[4] * 256) + s1_buffer[5];
      writeNextionSensorVal(s1_ppb, 0x30);
    }
  }

  if (Serial1.available()) {
    s1_new_data = true;
    s1_buffer[s1_buffer_counter] = Serial2.read();
    if (s1_buffer_counter < S1_BUFFER_SIZE) s1_buffer_counter++;
    s1_current_millis = millis();
  }
  }
*/
