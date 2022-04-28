int8_t gen_state = 0;
int delay_time = 4;

int8_t countdown_flag = 1;
uint32_t countdown_timer = 0;

uint8_t digits[][8] = {
  {1, 1, 1, 1, 1, 1, 0, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1, 0}, // 2
  {1, 1, 1, 1, 0, 0, 1, 0}, // 3
  {0, 1, 1, 0, 0, 1, 1, 0}, // 4
  {1, 0, 1, 1, 0, 1, 1, 0}, // 5
  {1, 0, 1, 1, 1, 1, 1, 0}, // 6
  {1, 1, 1, 0, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1, 0}, // 8
  {1, 1, 1, 1, 0, 1, 1, 0}, // 9
};

#define DIG_1 27
#define DIG_2 26
#define DIG_3 25
#define DIG_4 32

#define SEG_A 18
#define SEG_B 5
#define SEG_C 4
#define SEG_D 2
#define SEG_E 23
#define SEG_F 22
#define SEG_G 21
#define SEG_DP 19

uint8_t sensor_buff[9] = {};
uint8_t sensor_buff_index = 0;
uint32_t sensor_buff_timer = 0;
int8_t sensor_buffer_new_data = 0;

typedef struct sensor_t
{
  int16_t ppb_curr;
  int16_t ppb_prev;
} sensor_t;
sensor_t sensor1;


// ----------------------------------------------------------------------------------------
// ;SENSOR --------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
void sensor_manager()
{
  sensor1_read();
}

unsigned char get_checksum(unsigned char *i, unsigned char ln)
{
  unsigned char j, tempq = 0;
  i += 1;
  for (j = 0; j < (ln - 2); j++)
  {
    tempq += *i;
    i++;
  }
  tempq = (~tempq) + 1;
  return (tempq);
}

void sensor1_read()
{
  if (Serial2.available() > 0)
  {
    sensor_buffer_new_data = 1;
    uint8_t temp_val = Serial2.read();
    
    Serial.println(temp_val);
    if (sensor_buff_index < 9)
    {
      sensor_buff[sensor_buff_index] = temp_val;
      sensor_buff_index++;
      sensor_buff_timer = millis();
    }
  }

  if (millis() - sensor_buff_timer > 40)
  {
    if (sensor_buffer_new_data)
    {
      sensor_buffer_new_data = 0;

      if (get_checksum(sensor_buff, 9) == sensor_buff[8])
      {
        int tmp = sensor_buff[4] * 256 + sensor_buff[5];
        if (tmp >= 0 && tmp <= 10000)
        {
          sensor1.ppb_curr = tmp;
          sensor1.ppb_prev = -1;
        }
      }


      for (int i = 0; i < 9; i++)
      {
        sensor_buff[i] = 0;
      }
      sensor_buff_index = 0;
    }
  }
}

void setup()
{
  pinMode(SEG_A, OUTPUT);
  pinMode(SEG_B, OUTPUT);
  pinMode(SEG_C, OUTPUT);
  pinMode(SEG_D, OUTPUT);
  pinMode(SEG_E, OUTPUT);
  pinMode(SEG_F, OUTPUT);
  pinMode(SEG_G, OUTPUT);
  pinMode(SEG_DP, OUTPUT);

  pinMode(DIG_1, OUTPUT);
  pinMode(DIG_2, OUTPUT);
  pinMode(DIG_3, OUTPUT);
  pinMode(DIG_4, OUTPUT);

  digitalWrite(SEG_A, LOW);
  digitalWrite(SEG_B, LOW);
  digitalWrite(SEG_C, LOW);
  digitalWrite(SEG_D, LOW);
  digitalWrite(SEG_E, LOW);
  digitalWrite(SEG_F, LOW);
  digitalWrite(SEG_G, LOW);
  digitalWrite(SEG_DP, LOW);

  Serial.begin(9600);
  Serial2.begin(9600);

  Serial.println("Serial OK");
}

void seg_num_draw(int16_t num)
{
  int don = LOW;
  int doff = HIGH;
  
  int digit1 = num % 10000 / 1000;
  int digit2 = num % 1000 / 100;
  int digit3 = num % 100 / 10;
  int digit4 = num % 10 / 1;

  digitalWrite(DIG_1, don);
  digitalWrite(DIG_2, doff);
  digitalWrite(DIG_3, doff);
  digitalWrite(DIG_4, doff);
  seg_digit_draw(digit1, 1);
  delay(delay_time);

  digitalWrite(DIG_1, doff);
  digitalWrite(DIG_2, don);
  digitalWrite(DIG_3, doff);
  digitalWrite(DIG_4, doff);
  seg_digit_draw(digit2, 0);
  delay(delay_time);

  digitalWrite(DIG_1, doff);
  digitalWrite(DIG_2, doff);
  digitalWrite(DIG_3, don);
  digitalWrite(DIG_4, doff);
  seg_digit_draw(digit3, 0);
  delay(delay_time);

  digitalWrite(DIG_1, doff);
  digitalWrite(DIG_2, doff);
  digitalWrite(DIG_3, doff);
  digitalWrite(DIG_4, don);
  seg_digit_draw(digit4, 0);
  delay(delay_time);
}

void seg_digit_draw(int8_t num, int8_t dp) {
  digitalWrite(SEG_A, !digits[num][0]);
  digitalWrite(SEG_B, !digits[num][1]);
  digitalWrite(SEG_C, !digits[num][2]);
  digitalWrite(SEG_D, !digits[num][3]);
  digitalWrite(SEG_E, !digits[num][4]);
  digitalWrite(SEG_F, !digits[num][5]);
  digitalWrite(SEG_G, !digits[num][6]);
  digitalWrite(SEG_DP, !dp);
}

int demo_num = 1234;
uint32_t demo_timer = 1234;


void loop()
{
  sensor_manager();
  seg_num_draw(sensor1.ppb_curr);
  //seg_num_draw(demo_num);
  //demo_timer++;
  //delay(1000);

}
