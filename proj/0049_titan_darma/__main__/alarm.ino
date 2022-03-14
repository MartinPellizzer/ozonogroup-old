void AlarmHandler()
{
   if(s1_alarm_current) digitalWrite(PIN_OUT_ALARM1, HIGH);
   else digitalWrite(PIN_OUT_ALARM1, LOW);
   
   if(s3_alarm_current) digitalWrite(PIN_OUT_ALARM2, HIGH);
   else digitalWrite(PIN_OUT_ALARM2, LOW);
}
