void CycleHandler()
{
  if (IsCycleInputChanged())
    StartStopCycle();

  //StopCycleIfAlarm();

  ManageOzoneCycle();
}



/* ----------------------------------------------------------------------------------------------------------- */
/* -------------------------------------------------- INPUT -------------------------------------------------- */
/* ----------------------------------------------------------------------------------------------------------- */
bool IsCycleInputChanged()
{
  if (is_cycle_input_on_prev != is_cycle_input_on)
  {
    is_cycle_input_on_prev = is_cycle_input_on;
    return true;
  }
  return false;
}

void StartStopCycle()
{
  if (is_cycle_input_on) StartCycleIfPossible();
  else StopCycle();
}



/* ----------------------------------------------------------------------------------------------------------- */
/* -------------------------------------------------- START -------------------------------------------------- */
/* ----------------------------------------------------------------------------------------------------------- */
void StartCycleIfPossible()
{
  if (!s1_alarm_current && !s3_alarm_current && page_current == 1 && first_off)
  {
    is_on_current = 1;
    o3_gen_cycle_direction_current = 1;
    o3_gen_cycle_state_current = 1;
    o3_gen_cycle_current_millis = millis();
    s2_time_countdown = s2_time_current;
    start_countdown = 0;
    digitalWrite(PIN_OUT_END_CYCLE, LOW);
  }
}



/* ----------------------------------------------------------------------------------------------------------- */
/* -------------------------------------------------- STOP --------------------------------------------------- */
/* ----------------------------------------------------------------------------------------------------------- */
void StopCycleIfAlarm()
{
  if (IsAlarm1StateChanged() || IsAlarm3StateChanged())
    if(s1_alarm_current || s3_alarm_current)
      StopCycle();
}

bool IsAlarm1StateChanged()
{
  if (s1_alarm_old != s1_alarm_current)
  {
    s1_alarm_old = s1_alarm_current;
    return true;
  }
  return false;
}

bool IsAlarm3StateChanged()
{
  if (s3_alarm_old != s3_alarm_current)
  {
    s3_alarm_old = s3_alarm_current;
    return true;
  }
  return false;
}

void StopCycle()
{
  is_on_current = 0;
  s2_time_countdown = s2_time_current;

  digitalWrite(PIN_OUT_GENO3, LOW);
  o3_gen_cycle_state_current = 0;
  s1_settings_current = 1;
  s2_settings_current = 1;
  s3_settings_current = 1;
}



/* ----------------------------------------------------------------------------------------------------------- */
/* -------------------------------------------------- CYCLE -------------------------------------------------- */
/* ----------------------------------------------------------------------------------------------------------- */
void ManageOzoneCycle()
{
  if (is_on_current)
  {
    DecrementCountdown();

    if (o3_gen_cycle_state_current)
    {
      if ((millis() - o3_gen_cycle_current_millis) > O3_CYCLE_WORKING_TIMER_MILLIS)
      {
        o3_gen_cycle_current_millis = millis();
        o3_gen_cycle_state_current = !o3_gen_cycle_state_current;
      }
    }
    else
    {
      if ((millis() - o3_gen_cycle_current_millis) > O3_CYCLE_RESTING_TIMER_MILLIS)
      {
        o3_gen_cycle_current_millis = millis();
        o3_gen_cycle_state_current = !o3_gen_cycle_state_current;
      }
    }
    if (o3_gen_cycle_direction_current) digitalWrite(PIN_OUT_GENO3, o3_gen_cycle_state_current);
    else digitalWrite(PIN_OUT_GENO3, LOW);
    s1_settings_current = 0;
    s2_settings_current = 0;
    s3_settings_current = 0;
  }
  else StopCycle();
}

void DecrementCountdown()
{
  if (start_countdown)
  {
    if ((millis() - second_current_millis) > 1000)
    {
      second_current_millis = millis();

      if (s2_time_countdown - 1000 > 0) s2_time_countdown -= 1000;
      else CheckCountdown0();
    }
  }
}

void CheckCountdown0()
{
  digitalWrite(PIN_OUT_END_CYCLE, HIGH);
  StopCycle();
}
