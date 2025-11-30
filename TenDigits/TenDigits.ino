// see https://lastminuteengineers.com/esp32-pinout-reference/

// segments are active low
const uint8_t SEGMENT_A = 32;
const uint8_t SEGMENT_B = 33;
const uint8_t SEGMENT_C = 27;
const uint8_t SEGMENT_D = 13;
const uint8_t SEGMENT_E = 12;
const uint8_t SEGMENT_F = 25;  // not yet
const uint8_t SEGMENT_G = 26;
const uint8_t SEGMENT_DP = 14;

// but with PNP transistor they are acitve low
const uint8_t DRIVE1 = 18;  // was 4
const uint8_t DRIVE2 = 5;
const uint8_t DRIVE3 = 16;
const uint8_t DRIVE4 = 17;


// 0 thru 9 for now, adding hex ...
const uint8_t digitToSegments[] = {
  0b00111111,  // digit 0
  0b00000110,  // digit 1
  0b01011011,  // digit 2
  0b01001111,  // digit 3
  0b01100110,  // digit 4
  0b01101101,  // digit 5
  0b01111101,  // digit 6
  0b00000111,  // digit 7
  0b01111111,  // digit 8
  0b01100111,  // digit 9
  0b01110111,  // digit A
  0b01111100,  // digit b
};


const uint8_t segmentMap[] = {
  SEGMENT_A,
  SEGMENT_B,
  SEGMENT_C,
  SEGMENT_D,
  SEGMENT_E,
  SEGMENT_F,
  SEGMENT_G,
  SEGMENT_DP
};



void activateChannel(int n) {
  digitalWrite(DRIVE1, n == 1 ? LOW : HIGH);
  digitalWrite(DRIVE2, n == 2 ? LOW : HIGH);
  digitalWrite(DRIVE3, n == 3 ? LOW : HIGH);
  digitalWrite(DRIVE4, n == 4 ? LOW : HIGH);
}



void writeSegments(uint8_t segments) {
  uint8_t mask = 1;
  for (int x = 0; x < 8; x++) {
    digitalWrite(segmentMap[x], ((segments & mask) == 0) ? HIGH : LOW);
    mask <<= 1;
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("=================");
  Serial.println("TenDigits driver!");
  Serial.println("=================");

  pinMode(LED_BUILTIN, OUTPUT);

  // segments are outputs
  pinMode(SEGMENT_A, OUTPUT);
  pinMode(SEGMENT_B, OUTPUT);
  pinMode(SEGMENT_C, OUTPUT);
  pinMode(SEGMENT_D, OUTPUT);
  pinMode(SEGMENT_E, OUTPUT);
  pinMode(SEGMENT_F, OUTPUT);
  pinMode(SEGMENT_G, OUTPUT);
  pinMode(SEGMENT_DP, OUTPUT);

  // drives
  pinMode(DRIVE1, OUTPUT);
  pinMode(DRIVE2, OUTPUT);
  pinMode(DRIVE3, OUTPUT);
  pinMode(DRIVE4, OUTPUT);
}

bool builtinState = true;
int builtinCount = 0;
int dpPlace = 1;

const uint32_t delayTime = 3;

int counter = 0;
int counterDigits[4];
int workingDigit = 1;

void counterLoop() {
  if (++builtinCount > 25) {
    builtinCount = 0;
    builtinState = !builtinState;

    if (++dpPlace > 4)
      dpPlace = 1;

    if (++counter > 9999)
      counter = 0;

    counterDigits[3] = counter % 10;
    counterDigits[2] = (counter / 10) % 10;
    counterDigits[1] = (counter / 100) % 10;
    counterDigits[0] = (counter / 1000) % 10;
  }

  activateChannel(workingDigit);
  writeSegments(digitToSegments[counterDigits[workingDigit - 1]]);

  if (++workingDigit > 4)
    workingDigit = 1;

  delay(delayTime);
}


void faceLoop() {
  if (++builtinCount > 25) {
    builtinCount = 0;
    builtinState = !builtinState;

    if (++dpPlace > 4)
      dpPlace = 1;
  }
  digitalWrite(LED_BUILTIN, builtinState ? HIGH : LOW);

  activateChannel(1);
  digitalWrite(SEGMENT_A, LOW);
  digitalWrite(SEGMENT_B, HIGH);
  digitalWrite(SEGMENT_C, HIGH);
  digitalWrite(SEGMENT_D, HIGH);
  digitalWrite(SEGMENT_E, LOW);
  digitalWrite(SEGMENT_F, LOW);
  digitalWrite(SEGMENT_G, LOW);
  digitalWrite(SEGMENT_DP, (dpPlace == 1) ? HIGH : LOW);
  delay(delayTime);

  activateChannel(2);
  digitalWrite(SEGMENT_A, LOW);
  digitalWrite(SEGMENT_B, LOW);
  digitalWrite(SEGMENT_C, LOW);
  digitalWrite(SEGMENT_D, HIGH);
  digitalWrite(SEGMENT_E, LOW);
  digitalWrite(SEGMENT_F, LOW);
  digitalWrite(SEGMENT_G, LOW);
  digitalWrite(SEGMENT_DP, (dpPlace == 2) ? HIGH : LOW);
  delay(delayTime);

  activateChannel(3);
  digitalWrite(SEGMENT_A, LOW);
  digitalWrite(SEGMENT_B, HIGH);
  digitalWrite(SEGMENT_C, HIGH);
  digitalWrite(SEGMENT_D, LOW);
  digitalWrite(SEGMENT_E, LOW);
  digitalWrite(SEGMENT_F, LOW);
  digitalWrite(SEGMENT_G, HIGH);
  digitalWrite(SEGMENT_DP, (dpPlace == 3) ? HIGH : LOW);
  delay(delayTime);

  activateChannel(4);
  digitalWrite(SEGMENT_A, LOW);
  digitalWrite(SEGMENT_B, HIGH);
  digitalWrite(SEGMENT_C, HIGH);
  digitalWrite(SEGMENT_D, LOW);
  digitalWrite(SEGMENT_E, LOW);
  digitalWrite(SEGMENT_F, LOW);
  digitalWrite(SEGMENT_G, LOW);
  digitalWrite(SEGMENT_DP, (dpPlace == 4) ? HIGH : LOW);
  delay(delayTime);
}


void loop() {
  counterLoop();
  // faceLoop();
}
