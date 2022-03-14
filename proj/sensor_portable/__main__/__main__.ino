int8_t gen_state = 0;

int8_t countdown_flag = 1;
uint32_t countdown_timer = 0;

#define DIG_1 32
#define DIG_2 25
#define DIG_3 26
#define DIG_4 27

//#define SEG_A 13
//#define SEG_B 14
//#define SEG_C 15
//#define SEG_D 22
//#define SEG_E 23
//#define SEG_F 18
//#define SEG_G 19
//#define SEG_DP 21


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

      // DEBUG
      /*
        for (int i = 0; i < 9; i++)
        {
        Serial.println(sensor_buff[i]);
        sensor_buff[i] = 0;
        }
      */

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

  digitalWrite(DIG_1, HIGH);


  Serial.begin(9600);
  Serial2.begin(9600);

  Serial.println("Serial OK");
}


uint8_t digits[][8] = {
  {1, 1, 1, 1, 1, 1, 0, 0},
  {0, 1, 1, 0, 0, 0, 0, 0},
  {1, 1, 0, 1, 1, 0, 1, 0},
  {1, 1, 1, 1, 0, 0, 1, 0},
  {0, 1, 1, 0, 0, 1, 1, 0},
  {1, 0, 1, 1, 0, 1, 1, 0},

  {1, 0, 1, 1, 1, 1, 1, 0},
  {1, 1, 1, 0, 0, 0, 0, 0},
  {1, 1, 1, 1, 1, 1, 1, 0},
  {1, 1, 1, 1, 0, 1, 1, 0},
};

int delay_time = 4;

void seg_num_draw(int16_t num)
{
  int digit1 = num % 10000 / 1000;
  int digit2 = num % 1000 / 100;
  int digit3 = num % 100 / 10;
  int digit4 = num % 10 / 1;

  digitalWrite(DIG_1, HIGH);
  digitalWrite(DIG_2, LOW);
  digitalWrite(DIG_3, LOW);
  digitalWrite(DIG_4, LOW);
  seg_digit_draw(digit1);
  delay(delay_time);

  digitalWrite(DIG_1, LOW);
  digitalWrite(DIG_2, HIGH);
  digitalWrite(DIG_3, LOW);
  digitalWrite(DIG_4, LOW);
  seg_digit_draw(digit2);
  delay(delay_time);

  digitalWrite(DIG_1, LOW);
  digitalWrite(DIG_2, LOW);
  digitalWrite(DIG_3, HIGH);
  digitalWrite(DIG_4, LOW);
  seg_digit_draw(digit3);
  delay(delay_time);

  digitalWrite(DIG_1, LOW);
  digitalWrite(DIG_2, LOW);
  digitalWrite(DIG_3, LOW);
  digitalWrite(DIG_4, HIGH);
  seg_digit_draw(digit4);
  delay(delay_time);
}

void seg_digit_draw(int8_t num) {
  digitalWrite(SEG_A, !digits[num][0]);
  digitalWrite(SEG_B, !digits[num][1]);
  digitalWrite(SEG_C, !digits[num][2]);
  digitalWrite(SEG_D, !digits[num][3]);
  digitalWrite(SEG_E, !digits[num][4]);
  digitalWrite(SEG_F, !digits[num][5]);
  digitalWrite(SEG_G, !digits[num][6]);
  digitalWrite(SEG_DP, !digits[num][7]);
}

int demo_num = 1234;
uint32_t demo_timer = 0;


void loop()
{
  sensor_manager();
  if (sensor1.ppb_prev != sensor1.ppb_curr)
  {
    sensor1.ppb_prev = sensor1.ppb_curr;
    /*
    Serial.println(sensor1.ppb_curr);
    print_digit(demo_counter);
    demo_counter++;
    demo_counter %= 10;
    */
  }
    seg_num_draw(sensor1.ppb_curr);
//  seg_num_draw(demo_num);

/*
  seg_num_draw(demo_num);


  if (millis() - demo_timer > 1000)
  {
    demo_timer = millis();
    demo_num++;
  }
*/
  /*
    if (Serial2.available() > 0)
    {
    Serial.println(Serial2.read());
    }*/

  /*
    digitalWrite(DIG_4, LOW);
    digitalWrite(DIG_1, HIGH);
    delay(delay_time);
    digitalWrite(DIG_1, LOW);
    digitalWrite(DIG_2, HIGH);
    delay(delay_time);
    digitalWrite(DIG_2, LOW);
    digitalWrite(DIG_3, HIGH);
    delay(delay_time);
    digitalWrite(DIG_3, LOW);
    digitalWrite(DIG_4, HIGH);
    delay(delay_time);*/

}
