void setup()
{
  Serial.begin(9600);
  Serial2.begin(9600);
}

void loop()
{
  if (Serial2.available() > 0)
  {
    Serial.println(Serial2.read());
  }
}
