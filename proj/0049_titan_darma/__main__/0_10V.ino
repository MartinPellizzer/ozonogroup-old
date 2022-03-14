void DataLoggerHandler()
{
  int temp_val = s2_ppb_current;
  if(temp_val < 0) temp_val = 0;
  if(temp_val > 10000) temp_val = 10000;
  byte val = map(temp_val, 0, 10000, 0, 255);
  analogWrite(PIN_OUT_DATA, val);
}
