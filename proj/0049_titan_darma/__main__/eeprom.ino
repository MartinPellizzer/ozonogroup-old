void manageEEPROM() {
  if (EEPROM.read(0) != 1) {
    for (int i = 0; i < 256; i++) EEPROM.update(i, 0);
    EEPROM.update(0, 1);
    initFirstTimeEEPROM();
  } else { 
    initEEPROM();
  }

}

void initFirstTimeEEPROM() {
  EEPROMWriteInt(10, s1_min_current);
  EEPROMWriteInt(12, s1_max_current);
  EEPROMWriteInt(14, (s1_time_current / 1000 / 60));
  EEPROMWriteInt(16, (s1_time_countdown / 1000 / 60));
  EEPROMWriteInt(20, s2_min_current);
  EEPROMWriteInt(22, s2_max_current);
  EEPROMWriteInt(24, (s2_time_current / 1000 / 60));
  EEPROMWriteInt(26, (s2_time_countdown / 1000 / 60));
  EEPROMWriteInt(30, s3_min_current);
  EEPROMWriteInt(32, s3_max_current);
  EEPROMWriteInt(34, (s3_time_current / 1000 / 60));
  EEPROMWriteInt(36, (s3_time_countdown / 1000 / 60));
}

void initEEPROM() {
  s1_min_current = EEPROMReadInt(10);
  s1_max_current = EEPROMReadInt(12);
  s1_time_current = long(EEPROMReadInt(14)) * 60 * 1000;
  s1_time_countdown = long(EEPROMReadInt(16)) * 60 * 1000;
  s2_min_current = EEPROMReadInt(20);
  s2_max_current = EEPROMReadInt(22);
  s2_time_current = long(EEPROMReadInt(24)) * 60 * 1000;
  s2_time_countdown = long(EEPROMReadInt(26)) * 60 * 1000;
  s3_min_current = EEPROMReadInt(30);
  s3_max_current = EEPROMReadInt(32);
  s3_time_current = long(EEPROMReadInt(34)) * 60 * 1000;
  s3_time_countdown = long(EEPROMReadInt(36)) * 60 * 1000;
}

/* -------------------------------------------------------------------------------------- */
/* utils -------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
void EEPROMWriteInt(int address, int value)
{
  byte two = (value & 0xFF);
  byte one = ((value >> 8) & 0xFF);
  
  EEPROM.update(address, two);
  EEPROM.update(address + 1, one);
}

int EEPROMReadInt(int address)
{
  long two = EEPROM.read(address);
  long one = EEPROM.read(address + 1);
 
  return ((two << 0) & 0xFFFFFF) + ((one << 8) & 0xFFFFFFFF);
}

void EEPROMWriteByte(int address, int value)
{
  EEPROM.update(address, value);
}

int EEPROMReadByte(int address)
{
  return EEPROM.read(address);
}
