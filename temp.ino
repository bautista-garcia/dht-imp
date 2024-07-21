#define PIN 2
#define TIMEOUT 0xFFFFFFFF 

uint8_t data[5];
int medicion = 0;

void setup () {
  Serial.begin(9600);
}

void loop (){
  Serial.print("Medicion ");
  Serial.print(medicion);
  Serial.println(": ");
  read();
  Serial.print("Humidity: ");
  Serial.print(data[0]);
  Serial.print(".");
  Serial.print(data[1]);
  Serial.print("%, Temperature: ");
  Serial.print(data[2]);
  Serial.print(".");
  Serial.print(data[3]);
  Serial.println("C");
  medicion ++;
  delay(2000); // Wait before next read (e.g., 2 seconds)
}


void read() {

  // Reset all bytes to 0
  data[0] = data[1] = data[2] = data[3] = data[4] = 0;

  // 1st: Pull up mode
  pinMode(PIN, INPUT_PULLUP);
  delay(1);

  // 2nd: Start LOW signal for 18ms
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, LOW);
  delay(20); //min: 18ms, 20ms to be safe

  // Array storing the duration of each cycle
  uint32_t cycles[80];
  {
    // PULL-UP for 20-40uS
    pinMode(PIN, INPUT_PULLUP);
    delayMicroseconds(40);


    // Critical timings so we avoid interruptions
    noInterrupts();

    if (expectPulse(LOW) == TIMEOUT) {
      return;
    }
    if (expectPulse(HIGH) == TIMEOUT) {
      return;
    }

    for (int i = 0; i < 80; i += 2) {
      cycles[i] = expectPulse(LOW);
      cycles[i + 1] = expectPulse(HIGH);
    }
  }
  // Reading finalized, we enable back interruptions
  interrupts();


  for (int i = 0; i < 40; ++i) {
    uint32_t lowCycles = cycles[2 * i];
    uint32_t highCycles = cycles[2 * i + 1];

    data[i / 8] <<= 1;
    // Now compare the low and high cycle times to see if the bit is a 0 or 1.
    if (highCycles > lowCycles) {
      // High cycles are greater than 50us low cycle count, must be a 1.
      data[i / 8] |= 1;
    }
  }

  // Checksum: Last 8 bits of sum must be equal to data[4] -> &0xFF cuts the higher bits
  if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
    return;
  } else {
    Serial.println("There was a CHECKSUM error ...");
    return;
  }
}


uint32_t expectPulse(bool level){
  uint32_t count = 0; 
  while (digitalRead(PIN) == level) {
    if (count++ >= 1120) {
      return TIMEOUT;
    }
  }
  return count;
}
