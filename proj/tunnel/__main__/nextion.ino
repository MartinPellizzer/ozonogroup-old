const uint8_t BUFFER_SIZE = 20;
uint8_t buffer_nextion[BUFFER_SIZE];

uint8_t cmd_p1_onoff[BUFFER_SIZE] = {101, 1, 10, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint8_t cmd_set_s1[BUFFER_SIZE] = {101, 1, 11, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t cmd_set_s2[BUFFER_SIZE] = {101, 1, 12, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t cmd_set_s3[BUFFER_SIZE] = {101, 1, 13, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint8_t cmd_back[BUFFER_SIZE] = {101, 2, 1, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t cmd_save[BUFFER_SIZE] = {101, 2, 2, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint8_t cmd_min_sub[BUFFER_SIZE] = {101, 2, 6, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t cmd_min_add[BUFFER_SIZE] = {101, 2, 7, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t cmd_max_sub[BUFFER_SIZE] = {101, 2, 8, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t cmd_max_add[BUFFER_SIZE] = {101, 2, 9, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t cmd_time_sub[BUFFER_SIZE] = {101, 2, 10, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t cmd_time_add[BUFFER_SIZE] = {101, 2, 11, 1, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

bool new_data = false;
unsigned long nextion_current_millis = 0;
int buffer_counter = 0;

void NextionHandler()
{
  listenNextion();
  updateNextion();
}

void listenNextion() {
  if (Serial.available()) {
    new_data = true;
    buffer_nextion[buffer_counter] = Serial.read();
    if (buffer_counter < BUFFER_SIZE) buffer_counter++;
    nextion_current_millis = millis();
  }
  if (new_data) {
    if ((millis() - nextion_current_millis) > 10) {
      new_data = false;
      buffer_counter = 0;
      nextionEvalSerial();

      //nextionDebugSerial();
      nextionClearBuffer();
    }
  }
}

void nextionEvalSerial() {
  if (page_current == 1) {
    //if (compareArray(cmd_p1_onoff, buffer_nextion)) is_on_temp = !is_on_current;

    if (!is_on_current) {
      if (compareArray(cmd_set_s1, buffer_nextion)) page_current = 11;
      else if (compareArray(cmd_set_s2, buffer_nextion)) page_current = 12;
      else if (compareArray(cmd_set_s3, buffer_nextion)) page_current = 13;
    }
  }

  else if (page_current == 11) {
    if (compareArray(cmd_back, buffer_nextion)) {
      s1_max_temp = s1_max_current;
      page_current = 1;
    }
    else if (compareArray(cmd_save, buffer_nextion)) {
      s1_max_current = s1_max_temp;
      EEPROMWriteInt(12, s1_max_current);
      page_current = 1;
    }
    else if (compareArray(cmd_max_sub, buffer_nextion)) {
      if (s1_max_temp - 100 > 0) s1_max_temp -= 100;
    }
    else if (compareArray(cmd_max_add, buffer_nextion)) {
      if (s1_max_temp + 100 < 10000) s1_max_temp += 100;
    }
  }

  else if (page_current == 12) {
    if (compareArray(cmd_back, buffer_nextion)) {
      s2_min_temp = s2_min_current;
      s2_max_temp = s2_max_current;
      s2_time_temp = s2_time_current;
      page_current = 1;
    }
    else if (compareArray(cmd_save, buffer_nextion)) {
      s2_min_current = s2_min_temp;
      s2_max_current = s2_max_temp;
      s2_time_current = s2_time_temp;
      s2_time_countdown = s2_time_current;
      EEPROMWriteInt(20, s2_min_current);
      EEPROMWriteInt(22, s2_max_current);
      EEPROMWriteInt(24, (s2_time_current / 1000 / 60));
      EEPROMWriteInt(26, (s2_time_countdown / 1000 / 60));
      page_current = 1;
    }
    else if (compareArray(cmd_min_sub, buffer_nextion)) {
      if (s2_min_temp - 100 > 0) s2_min_temp -= 100;
    }
    else if (compareArray(cmd_min_add, buffer_nextion)) {
      if (s2_min_temp + 100 < 10000 && s2_min_temp < s2_max_temp) s2_min_temp += 100;
    }
    else if (compareArray(cmd_max_sub, buffer_nextion)) {
      if (s2_max_temp - 100 > 0 && s2_max_temp > s2_min_temp) s2_max_temp -= 100;
    }
    else if (compareArray(cmd_max_add, buffer_nextion)) {
      if (s2_max_temp + 100 < 10000) s2_max_temp += 100;
    }
    else if (compareArray(cmd_time_sub, buffer_nextion)) {
      if (s2_time_temp - 60000 >= 60000) s2_time_temp -= 60000;
    }
    else if (compareArray(cmd_time_add, buffer_nextion)) {
      if (s2_time_temp + 60000 <= 5400000) s2_time_temp += 60000;
    }
  }

  if (page_current == 13) {
    if (compareArray(cmd_back, buffer_nextion)) {
      s3_max_temp = s3_max_current;
      page_current = 1;
    }
    else if (compareArray(cmd_save, buffer_nextion)) {
      s3_max_current = s3_max_temp;
      EEPROMWriteInt(32, s3_max_current);
      page_current = 1;
    }
    else if (compareArray(cmd_max_sub, buffer_nextion)) {
      if (s3_max_temp - 100 > 0) s3_max_temp -= 100;
    }
    else if (compareArray(cmd_max_add, buffer_nextion)) {
      if (s3_max_temp + 100 < 10000) s3_max_temp += 100;
    }
  }
}



/* -------------------------------------------------- */
void updateNextion()
{
  uint8_t force_refresh = 0;

  if (page_old != page_current)
  {
    page_old = page_current;
    force_refresh = 1;
  }

  /**/ if (page_current == 1)   nextionUpdatePageHome(force_refresh);
  else if (page_current == 11)  nextionUpdatePageSettings1(force_refresh);
  else if (page_current == 12)  nextionUpdatePageSettings2(force_refresh);
  else if (page_current == 13)  nextionUpdatePageSettings3(force_refresh);
}



/* ------------------------------------------------------------------------------------------------------------------------------------------- */
/* PAGE HOME --------------------------------------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------------------------------------------- */
void nextionUpdatePageHome(uint8_t force_refresh) {
  if (force_refresh) {
    uint8_t cmd_page_home[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x5F, 0x68, 0x6F, 0x6D, 0x65, 0xff, 0xff, 0xff};
    nextionExecCommand(cmd_page_home, sizeof(cmd_page_home) / sizeof(uint8_t));
  }
  /*if (force_refresh || is_on_old != is_on_current) {
    is_on_old = is_on_current;
    nextionUpdateOnOff();
    }*/
  if (force_refresh || s1_settings_old != s1_settings_current) {
    s1_settings_old = s1_settings_current;
    nextionUpdateSensor1SettingsIcon();
  }
  if (force_refresh || s2_settings_old != s2_settings_current) {
    s2_settings_old = s2_settings_current;
    nextionUpdateSensor2SettingsIcon();
  }
  if (force_refresh || s3_settings_old != s3_settings_current) {
    s3_settings_old = s3_settings_current;
    nextionUpdateSensor3SettingsIcon();
  }
  if (force_refresh || s1_ppb_old != s1_ppb_current) {
    s1_ppb_old = s1_ppb_current;
    nextionUpdateSensor1Val();
  }
  if (force_refresh || s1_color_old != s1_color_current) {
    s1_color_old = s1_color_current;
    nextionUpdateSensor1Color();
  }
  if (force_refresh || s1_min_old != s1_min_current) {
    s1_min_old = s1_min_current;
    nextionUpdateSensor1Min();
  }
  if (force_refresh || s1_max_old != s1_max_current) {
    s1_max_old = s1_max_current;
    nextionUpdateSensor1Max();
  }
  if (force_refresh || s2_ppb_old != s2_ppb_current) {
    s2_ppb_old = s2_ppb_current;
    nextionUpdateSensor2Val();
  }
  if (force_refresh || s2_color_old != s2_color_current) {
    s2_color_old = s2_color_current;
    nextionUpdateSensor2Color();
  }
  if (force_refresh || s2_min_old != s2_min_current) {
    s2_min_old = s2_min_current;
    nextionUpdateSensor2Min();
  }
  if (force_refresh || s2_max_old != s2_max_current) {
    s2_max_old = s2_max_current;
    nextionUpdateSensor2Max();
  }
  if (force_refresh || s2_time_old != s2_time_countdown) {
    s2_time_old = s2_time_countdown;
    nextionUpdateSensor2Time();
  }
  if (force_refresh || s3_ppb_old != s3_ppb_current) {
    s3_ppb_old = s3_ppb_current;
    nextionUpdateSensor3Val();
  }
  if (force_refresh || s3_color_old != s3_color_current) {
    s3_color_old = s3_color_current;
    nextionUpdateSensor3Color();
  }
  if (force_refresh || s3_min_old != s3_min_current) {
    s3_min_old = s3_min_current;
    nextionUpdateSensor3Min();
  }
  if (force_refresh || s3_max_old != s3_max_current) {
    s3_max_old = s3_max_current;
    nextionUpdateSensor3Max();
  }
}

/*
  void nextionUpdateOnOff() {
  uint8_t _cmd_off[] = {0x70, 0x30, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x33, 0xff, 0xff, 0xff};
  uint8_t _cmd_on[] = {0x70, 0x30, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x34, 0xff, 0xff, 0xff};
  if (is_on_current) nextionExecCommand(_cmd_on, sizeof(_cmd_on) / sizeof(uint8_t));
  else nextionExecCommand(_cmd_off, sizeof(_cmd_off) / sizeof(uint8_t));
  }
*/

void nextionUpdateSensor1SettingsIcon() {
  uint8_t _cmd_enable[] = {0x70, 0x31, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x36, 0xff, 0xff, 0xff};
  uint8_t _cmd_disable[] = {0x70, 0x31, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x37, 0xff, 0xff, 0xff};
  if (!is_on_current) nextionExecCommand(_cmd_enable, sizeof(_cmd_enable));
  else nextionExecCommand(_cmd_disable, sizeof(_cmd_disable));
}

void nextionUpdateSensor2SettingsIcon() {
  uint8_t _cmd_enable[] = {0x70, 0x32, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x36, 0xff, 0xff, 0xff};
  uint8_t _cmd_disable[] = {0x70, 0x32, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x37, 0xff, 0xff, 0xff};
  if (!is_on_current) nextionExecCommand(_cmd_enable, sizeof(_cmd_enable));
  else nextionExecCommand(_cmd_disable, sizeof(_cmd_disable));
}

void nextionUpdateSensor3SettingsIcon() {
  uint8_t _cmd_enable[] = {0x70, 0x33, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x36, 0xff, 0xff, 0xff};
  uint8_t _cmd_disable[] = {0x70, 0x33, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x37, 0xff, 0xff, 0xff};
  if (!is_on_current) nextionExecCommand(_cmd_enable, sizeof(_cmd_enable));
  else nextionExecCommand(_cmd_disable, sizeof(_cmd_disable));
}

void nextionUpdateSensor1Val() {
  uint8_t _buffer[] = {0x74, 0x30, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
  if (s1_working_current_attempts < WORKING_MAX_ATTEMPTS)
  {
    _buffer[8] = (s1_ppb_current % 100000 / 10000) + 0x30;
    _buffer[9] = (s1_ppb_current % 10000 / 1000) + 0x30;
    _buffer[11] = (s1_ppb_current % 1000 / 100) + 0x30;
    _buffer[12] = (s1_ppb_current % 100 / 10) + 0x30;
  }
  else
  {
    _buffer[8] = 0x2D;
    _buffer[9] = 0x2D;
    _buffer[11] = 0x2D;
    _buffer[12] = 0x2D;
  }
  
  nextionExecCommand(_buffer, sizeof(_buffer) / sizeof(uint8_t));
}

void nextionUpdateSensor1Color() {
  uint8_t _cmd_red[] = {0x74, 0x30, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x35, 0x39, 0x39, 0x30, 0x37, 0xff, 0xff, 0xff};
  uint8_t _cmd_white[] = {0x74, 0x30, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x35, 0x37, 0x30, 0x38, 0x33, 0xff, 0xff, 0xff};
  if (s1_color_current == 1) nextionExecCommand(_cmd_red, sizeof(_cmd_red) / sizeof(uint8_t));
  else nextionExecCommand(_cmd_white, sizeof(_cmd_white) / sizeof(uint8_t));
}

void nextionUpdateSensor1Min() {
  uint8_t _buffer[] = {0x74, 0x33, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
  _buffer[8] = (s1_min_current % 100000 / 10000) + 0x30;
  _buffer[9] = (s1_min_current % 10000 / 1000) + 0x30;
  _buffer[11] = (s1_min_current % 1000 / 100) + 0x30;
  _buffer[12] = (s1_min_current % 100 / 10) + 0x30;
  nextionExecCommand(_buffer, sizeof(_buffer) / sizeof(uint8_t));
}

void nextionUpdateSensor1Max() {
  uint8_t _buffer[] = {0x74, 0x34, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
  _buffer[8] = (s1_max_current % 100000 / 10000) + 0x30;
  _buffer[9] = (s1_max_current % 10000 / 1000) + 0x30;
  _buffer[11] = (s1_max_current % 1000 / 100) + 0x30;
  _buffer[12] = (s1_max_current % 100 / 10) + 0x30;
  nextionExecCommand(_buffer, sizeof(_buffer) / sizeof(uint8_t));
}
void nextionUpdateSensor2Val() {
  uint8_t _buffer[] = {0x74, 0x31, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
  if (s2_working_current_attempts < WORKING_MAX_ATTEMPTS)
  {
    _buffer[8] = (s2_ppb_current % 100000 / 10000) + 0x30;
    _buffer[9] = (s2_ppb_current % 10000 / 1000) + 0x30;
    _buffer[11] = (s2_ppb_current % 1000 / 100) + 0x30;
    _buffer[12] = (s2_ppb_current % 100 / 10) + 0x30;
  }
  else
  {
    _buffer[8] = 0x2D;
    _buffer[9] = 0x2D;
    _buffer[11] = 0x2D;
    _buffer[12] = 0x2D;
  }
  nextionExecCommand(_buffer, sizeof(_buffer) / sizeof(uint8_t));
}
void nextionUpdateSensor2Color() {
  uint8_t _cmd_green[] = {0x74, 0x31, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x32, 0x30, 0x31, 0x30, 0x32, 0xff, 0xff, 0xff};
  uint8_t _cmd_blue[] = {0x74, 0x31, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x31, 0x33, 0x37, 0x35, 0xff, 0xff, 0xff};
  uint8_t _cmd_white[] = {0x74, 0x31, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x35, 0x37, 0x30, 0x38, 0x33, 0xff, 0xff, 0xff};
  if (s2_color_current == 1) nextionExecCommand(_cmd_green, sizeof(_cmd_green) / sizeof(uint8_t));
  else nextionExecCommand(_cmd_white, sizeof(_cmd_white) / sizeof(uint8_t));
}
void nextionUpdateSensor2Min() {
  uint8_t _buffer[] = {0x74, 0x35, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
  _buffer[8] = (s2_min_current % 100000 / 10000) + 0x30;
  _buffer[9] = (s2_min_current % 10000 / 1000) + 0x30;
  _buffer[11] = (s2_min_current % 1000 / 100) + 0x30;
  _buffer[12] = (s2_min_current % 100 / 10) + 0x30;
  nextionExecCommand(_buffer, sizeof(_buffer) / sizeof(uint8_t));
}
void nextionUpdateSensor2Max() {
  uint8_t _buffer[] = {0x74, 0x36, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x35, 0x2E, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
  _buffer[8] = (s2_max_current % 100000 / 10000) + 0x30;
  _buffer[9] = (s2_max_current % 10000 / 1000) + 0x30;
  _buffer[11] = (s2_max_current % 1000 / 100) + 0x30;
  _buffer[12] = (s2_max_current % 100 / 10) + 0x30;
  nextionExecCommand(_buffer, sizeof(_buffer) / sizeof(uint8_t));
}
void nextionUpdateSensor2Time() {
  uint8_t _buffer[] = {0x74, 0x39, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
  unsigned long millisToMinutes = s2_time_countdown / 1000 / 60;
  unsigned long millisToSeconds = s2_time_countdown / 1000 % 60;
  _buffer[8] = (millisToMinutes % 100 / 10) + 0x30;
  _buffer[9] = (millisToMinutes % 10 / 1) + 0x30;
  _buffer[11] = (millisToSeconds % 100 / 10) + 0x30;
  _buffer[12] = (millisToSeconds % 10 / 1) + 0x30;
  nextionExecCommand(_buffer, sizeof(_buffer) / sizeof(uint8_t));
}
void nextionUpdateSensor3Val() {
  
  uint8_t _buffer[] = {0x74, 0x32, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
  if (s3_working_current_attempts < WORKING_MAX_ATTEMPTS)
  {
    _buffer[8] = (s3_ppb_current % 100000 / 10000) + 0x30;
    _buffer[9] = (s3_ppb_current % 10000 / 1000) + 0x30;
    _buffer[11] = (s3_ppb_current % 1000 / 100) + 0x30;
    _buffer[12] = (s3_ppb_current % 100 / 10) + 0x30;
  }
  else
  {
    _buffer[8] = 0x2D;
    _buffer[9] = 0x2D;
    _buffer[11] = 0x2D;
    _buffer[12] = 0x2D;
  }
  nextionExecCommand(_buffer, sizeof(_buffer) / sizeof(uint8_t));
}
void nextionUpdateSensor3Color() {
  uint8_t _cmd_red[] = {0x74, 0x32, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x35, 0x39, 0x39, 0x30, 0x37, 0xff, 0xff, 0xff};
  uint8_t _cmd_white[] = {0x74, 0x32, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x35, 0x37, 0x30, 0x38, 0x33, 0xff, 0xff, 0xff};
  if (s3_color_current == 1) nextionExecCommand(_cmd_red, sizeof(_cmd_red) / sizeof(uint8_t));
  else nextionExecCommand(_cmd_white, sizeof(_cmd_white) / sizeof(uint8_t));
}
void nextionUpdateSensor3Min() {
  uint8_t _buffer[] = {0x74, 0x37, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
  _buffer[8] = (s3_min_current % 100000 / 10000) + 0x30;
  _buffer[9] = (s3_min_current % 10000 / 1000) + 0x30;
  _buffer[11] = (s3_min_current % 1000 / 100) + 0x30;
  _buffer[12] = (s3_min_current % 100 / 10) + 0x30;
  nextionExecCommand(_buffer, sizeof(_buffer) / sizeof(uint8_t));
}
void nextionUpdateSensor3Max() {
  uint8_t _buffer[] = {0x74, 0x38, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
  _buffer[8] = (s3_max_current % 100000 / 10000) + 0x30;
  _buffer[9] = (s3_max_current % 10000 / 1000) + 0x30;
  _buffer[11] = (s3_max_current % 1000 / 100) + 0x30;
  _buffer[12] = (s3_max_current % 100 / 10) + 0x30;
  nextionExecCommand(_buffer, sizeof(_buffer) / sizeof(uint8_t));
}



/* ------------------------------------------------------------------------------------------------------------------------------------------- */
/* PAGE SETTINGS ----------------------------------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------------------------------------------- */
void nextionUpdatePageSettings1(uint8_t force_refresh) {
  if (force_refresh) {
    s1_max_temp = s1_max_current;
    uint8_t cmd_page_settings[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x5F, 0x73, 0x65, 0x74, 0xff, 0xff, 0xff};
    nextionExecCommand(cmd_page_settings, sizeof(cmd_page_settings) / sizeof(uint8_t));
    nextionUpdateMinusIcon1(0);
    nextionUpdatePlusIcon1(0);
    nextionUpdateMinusIcon2(1);
    nextionUpdatePlusIcon2(1);
    nextionUpdateMinusIcon3(0);
    nextionUpdatePlusIcon3(0);
  }
  if (force_refresh || s1_min_old != s1_min_temp) {
    s1_min_old = s1_min_temp;
    nextionUpdateSettingsPage1SensorMin();
  }
  if (force_refresh || s1_max_old != s1_max_temp) {
    s1_max_old = s1_max_temp;
    nextionUpdateSettingsPage1SensorMax();
  }
  if (force_refresh || s1_time_old != s1_time_temp) {
    s1_time_old = s1_time_temp;
    nextionUpdateSettingsPage1SensorTime();
  }
}
void nextionUpdatePageSettings2(uint8_t force_refresh) {
  if (force_refresh) {
    s2_min_temp = s2_min_current;
    s2_max_temp = s2_max_current;
    s2_time_temp = s2_time_current;
    uint8_t cmd_page_settings[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x5F, 0x73, 0x65, 0x74, 0xff, 0xff, 0xff};
    nextionExecCommand(cmd_page_settings, sizeof(cmd_page_settings) / sizeof(uint8_t));
    nextionUpdateMinusIcon1(1);
    nextionUpdatePlusIcon1(1);
    nextionUpdateMinusIcon2(1);
    nextionUpdatePlusIcon2(1);
    nextionUpdateMinusIcon3(1);
    nextionUpdatePlusIcon3(1);
  }
  if (force_refresh || s2_min_old != s2_min_temp) {
    s2_min_old = s2_min_temp;
    nextionUpdateSettingsPage2SensorMin();
  }
  if (force_refresh || s2_max_old != s2_max_temp) {
    s2_max_old = s2_max_temp;
    nextionUpdateSettingsPage2SensorMax();
  }
  if (force_refresh || s2_time_old != s2_time_temp) {
    s2_time_old = s2_time_temp;
    nextionUpdateSettingsPage2SensorTime();
  }
}
void nextionUpdatePageSettings3(uint8_t force_refresh) {
  if (force_refresh) {
    s3_max_temp = s3_max_current;
    uint8_t _cmd[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x70, 0x5F, 0x73, 0x65, 0x74, 0xff, 0xff, 0xff};
    nextionExecCommand(_cmd, sizeof(_cmd));
    nextionUpdateMinusIcon1(0);
    nextionUpdatePlusIcon1(0);
    nextionUpdateMinusIcon2(1);
    nextionUpdatePlusIcon2(1);
    nextionUpdateMinusIcon3(0);
    nextionUpdatePlusIcon3(0);
  }
  if (force_refresh || s3_min_old != s3_min_temp) {
    s3_min_old = s3_min_temp;
    nextionUpdateSettingsPage3SensorMin();
  }
  if (force_refresh || s3_max_old != s3_max_temp) {
    s3_max_old = s3_max_temp;
    nextionUpdateSettingsPage3SensorMax();
  }
  if (force_refresh || s3_time_old != s3_time_temp) {
    s3_time_old = s3_time_temp;
    nextionUpdateSettingsPage3SensorTime();
  }
}

void nextionUpdateSettingsPage1SensorMin() {
  uint8_t _cmd_1[] = {0x74, 0x30, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x38, 0x34, 0x35, 0x32, 0xff, 0xff, 0xff};
  nextionExecCommand(_cmd_1, sizeof(_cmd_1));
  uint8_t _cmd_2[] = {0x74, 0x30, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
  _cmd_2[8] = (s1_min_temp % 100000 / 10000) + 0x30;
  _cmd_2[9] = (s1_min_temp % 10000 / 1000) + 0x30;
  _cmd_2[11] = (s1_min_temp % 1000 / 100) + 0x30;
  _cmd_2[12] = (s1_min_temp % 100 / 10) + 0x30;
  nextionExecCommand(_cmd_2, sizeof(_cmd_2));
}
void nextionUpdateSettingsPage1SensorMax() {
  uint8_t _cmd[] = {0x74, 0x31, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
  _cmd[8] = (s1_max_temp % 100000 / 10000) + 0x30;
  _cmd[9] = (s1_max_temp % 10000 / 1000) + 0x30;
  _cmd[11] = (s1_max_temp % 1000 / 100) + 0x30;
  _cmd[12] = (s1_max_temp % 100 / 10) + 0x30;
  nextionExecCommand(_cmd, sizeof(_cmd));
}
void nextionUpdateSettingsPage1SensorTime() {
  uint8_t _cmd_1[] = {0x74, 0x32, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x38, 0x34, 0x35, 0x32, 0xff, 0xff, 0xff};
  nextionExecCommand(_cmd_1, sizeof(_cmd_1));
  uint8_t _cmd_2[] = {0x74, 0x32, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
  _cmd_2[8] = (s1_time_temp % 100000 / 10000) + 0x30;
  _cmd_2[9] = (s1_time_temp % 10000 / 1000) + 0x30;
  _cmd_2[11] = (s1_time_temp % 1000 / 100) + 0x30;
  _cmd_2[12] = (s1_time_temp % 100 / 10) + 0x30;
  nextionExecCommand(_cmd_2, sizeof(_cmd_2));
}
void nextionUpdateSettingsPage2SensorMin() {
  uint8_t _cmd[] = {0x74, 0x30, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
  _cmd[8] = (s2_min_temp % 100000 / 10000) + 0x30;
  _cmd[9] = (s2_min_temp % 10000 / 1000) + 0x30;
  _cmd[11] = (s2_min_temp % 1000 / 100) + 0x30;
  _cmd[12] = (s2_min_temp % 100 / 10) + 0x30;
  nextionExecCommand(_cmd, sizeof(_cmd));
}
void nextionUpdateSettingsPage2SensorMax() {
  uint8_t _cmd[] = {0x74, 0x31, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
  _cmd[8] = (s2_max_temp % 100000 / 10000) + 0x30;
  _cmd[9] = (s2_max_temp % 10000 / 1000) + 0x30;
  _cmd[11] = (s2_max_temp % 1000 / 100) + 0x30;
  _cmd[12] = (s2_max_temp % 100 / 10) + 0x30;
  nextionExecCommand(_cmd, sizeof(_cmd));
}
void nextionUpdateSettingsPage2SensorTime() {
  uint8_t _cmd[] = {0x74, 0x32, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
  unsigned long millisToMinutes = s2_time_temp / 60 / 1000;
  _cmd[8] = (millisToMinutes % 10000 / 1000) + 0x30;
  _cmd[9] = (millisToMinutes % 1000 / 100) + 0x30;
  _cmd[10] = (millisToMinutes % 100 / 10) + 0x30;
  _cmd[11] = (millisToMinutes % 10 / 1) + 0x30;
  nextionExecCommand(_cmd, sizeof(_cmd));
}
void nextionUpdateSettingsPage3SensorMin() {
  uint8_t _cmd_1[] = {0x74, 0x30, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x38, 0x34, 0x35, 0x32, 0xff, 0xff, 0xff};
  nextionExecCommand(_cmd_1, sizeof(_cmd_1));
  uint8_t _cmd_2[] = {0x74, 0x30, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
  _cmd_2[8] = (s3_min_temp % 100000 / 10000) + 0x30;
  _cmd_2[9] = (s3_min_temp % 10000 / 1000) + 0x30;
  _cmd_2[11] = (s3_min_temp % 1000 / 100) + 0x30;
  _cmd_2[12] = (s3_min_temp % 100 / 10) + 0x30;
  nextionExecCommand(_cmd_2, sizeof(_cmd_2));
}
void nextionUpdateSettingsPage3SensorMax() {
  uint8_t _cmd[] = {0x74, 0x31, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
  _cmd[8] = (s3_max_temp % 100000 / 10000) + 0x30;
  _cmd[9] = (s3_max_temp % 10000 / 1000) + 0x30;
  _cmd[11] = (s3_max_temp % 1000 / 100) + 0x30;
  _cmd[12] = (s3_max_temp % 100 / 10) + 0x30;
  nextionExecCommand(_cmd, sizeof(_cmd));
}

void nextionUpdateSettingsPage3SensorTime() {
  uint8_t _cmd_1[] = {0x74, 0x32, 0x2E, 0x70, 0x63, 0x6F, 0x3D, 0x38, 0x34, 0x35, 0x32, 0xff, 0xff, 0xff};
  nextionExecCommand(_cmd_1, sizeof(_cmd_1));
  uint8_t _cmd_2[] = {0x74, 0x32, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x22, 0xff, 0xff, 0xff};
  _cmd_2[8] = (s3_time_temp % 100000 / 10000) + 0x30;
  _cmd_2[9] = (s3_time_temp % 10000 / 1000) + 0x30;
  _cmd_2[11] = (s3_time_temp % 1000 / 100) + 0x30;
  _cmd_2[12] = (s3_time_temp % 100 / 10) + 0x30;
  nextionExecCommand(_cmd_2, sizeof(_cmd_2));
}

void nextionUpdateMinusIcon1(uint8_t is_enable) {
  uint8_t _cmd_enable[] = {0x70, 0x30, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x31, 0x32, 0xff, 0xff, 0xff};
  uint8_t _cmd_disable[] = {0x70, 0x30, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x31, 0x33, 0xff, 0xff, 0xff};
  if (is_enable) nextionExecCommand(_cmd_enable, sizeof(_cmd_enable));
  else nextionExecCommand(_cmd_disable, sizeof(_cmd_disable));
}
void nextionUpdatePlusIcon1(uint8_t is_enable) {
  uint8_t _cmd_enable[] = {0x70, 0x31, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x39, 0xff, 0xff, 0xff};
  uint8_t _cmd_disable[] = {0x70, 0x31, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x31, 0x30, 0xff, 0xff, 0xff};
  if (is_enable) nextionExecCommand(_cmd_enable, sizeof(_cmd_enable));
  else nextionExecCommand(_cmd_disable, sizeof(_cmd_disable));
}

void nextionUpdateMinusIcon2(uint8_t is_enable) {
  uint8_t _cmd_enable[] = {0x70, 0x32, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x31, 0x32, 0xff, 0xff, 0xff};
  uint8_t _cmd_disable[] = {0x70, 0x32, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x31, 0x33, 0xff, 0xff, 0xff};
  if (is_enable) nextionExecCommand(_cmd_enable, sizeof(_cmd_enable));
  else nextionExecCommand(_cmd_disable, sizeof(_cmd_disable));
}
void nextionUpdatePlusIcon2(uint8_t is_enable) {
  uint8_t _cmd_enable[] = {0x70, 0x33, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x39, 0xff, 0xff, 0xff};
  uint8_t _cmd_disable[] = {0x70, 0x33, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x31, 0x30, 0xff, 0xff, 0xff};
  if (is_enable) nextionExecCommand(_cmd_enable, sizeof(_cmd_enable));
  else nextionExecCommand(_cmd_disable, sizeof(_cmd_disable));
}

void nextionUpdateMinusIcon3(uint8_t is_enable) {
  uint8_t _cmd_enable[] = {0x70, 0x34, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x31, 0x32, 0xff, 0xff, 0xff};
  uint8_t _cmd_disable[] = {0x70, 0x34, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x31, 0x33, 0xff, 0xff, 0xff};
  if (is_enable) nextionExecCommand(_cmd_enable, sizeof(_cmd_enable));
  else nextionExecCommand(_cmd_disable, sizeof(_cmd_disable));
}
void nextionUpdatePlusIcon3(uint8_t is_enable) {
  uint8_t _cmd_enable[] = {0x70, 0x35, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x39, 0xff, 0xff, 0xff};
  uint8_t _cmd_disable[] = {0x70, 0x35, 0x2E, 0x70, 0x69, 0x63, 0x3D, 0x31, 0x30, 0xff, 0xff, 0xff};
  if (is_enable) nextionExecCommand(_cmd_enable, sizeof(_cmd_enable));
  else nextionExecCommand(_cmd_disable, sizeof(_cmd_disable));
}
