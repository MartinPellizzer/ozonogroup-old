#define acs712 A0

long last_sample = 0;
long sample_sum = 0;
int sample_count = 0;

float vpc = 4.8828125;

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  if (millis() > last_sample + 1)
  {
    last_sample = millis();

    sample_sum += sq(analogRead(acs712) - 512);
    sample_count++;
  }

  if (sample_count >= 1000)
  {
    float mean = sample_sum / sample_count;
    float value = sqrt(mean);
    float mv = value * vpc;
    float amperage = mv / 100; // 100 is from datasheet of acs712-20A
    
    //Serial.println(value);
    //Serial.println(value_mv);
    Serial.println(amperage);

    sample_sum = 0;
    sample_count = 0;
  }
}
