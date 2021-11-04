void OnOffHandler()
{
  _GetDebouncedOnOff();
}

void _GetDebouncedOnOff()
{
  onoff_current = digitalRead(PIN_IN_START_CYCLE);

  if (onoff_old != onoff_current)
  {
    onoff_old = onoff_current;
    onoff_debouncing = 1;
    onoff_millis_current = millis();
  }

  if (onoff_debouncing)
  {
    if ((millis() - onoff_millis_current) > 40)
    {      
      if (!onoff_current) is_cycle_input_on = true;
      else
      {
        is_cycle_input_on = false;
        first_off = true;
      }
      
      onoff_debouncing = 0;
    }
  }
}
