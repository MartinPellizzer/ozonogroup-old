bool compareArray(uint8_t *a, uint8_t *b) {
  for (uint8_t i = 0; i < BUFFER_SIZE; i++) {
    if (a[i] != b[i]) {
      return false;
    }
  }
  return true;
}

bool nextionExecCommand(uint8_t arr[], uint8_t arr_size) {
  for (uint8_t i = 0; i < arr_size; i++) {
    Serial.write(arr[i]);
  }
}

void nextionClearBuffer() {
  for (int i = 0; i < BUFFER_SIZE; i++) {
    buffer_nextion[i] = 0;
  }
}

void nextionDebugSerial() {
  for (int i = 0; i < BUFFER_SIZE; i++) {
    Serial.print(buffer_nextion[i]);
    Serial.print(" ");
  }
  Serial.println();
}

unsigned char FucCheckSum(unsigned char *i, unsigned char ln)
{
  unsigned char j, tempq = 0;
  i += 1;
  for (j = 0; j < (ln - 2); j++)
  {
    tempq += *i;
    i++;
  }
  tempq = (~tempq) + 1; return (tempq);
}
