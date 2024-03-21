// Start Pins
#define buttonMotorForwardX 16
#define buttonMotorForwardY 17
#define buttonMotorForwardW 18
#define buttonMotorForwardE 19
#define buttonMotorBackwardX 20
#define buttonMotorBackwardY 21
#define buttonMotorBackwardW 22
#define buttonMotorBackwardE 23
#define buttonResetPosition 24
#define buttonHomePosition 25
#define buttonStopAuto 30
#define driverReadyToMove 31
#define sensorMaxX 26
#define sensorMaxY 27
#define sensorMaxW 28
#define dataPLC 29
#define motorFrequency 3000
int pinsMotors[4][4] = {
  { 0, 1, 2, 3 },
  { 4, 5, 6, 7 },
  { 8, 9, 10, 11 },
  { 12, 13, 14, 15 }
};
// End Pins
// Start Settings
int manipulatorPosition[4][4] = {
  { 790, 400, 0, 500 },
  { 254, 985, 999, 0 },
  { 443, 645, 1, 954 },
  { 223, 543, 432, 123 }
};
bool motorsValues[4][4]={
  {1,1,0,0},
  {0,1,1,0},
  {0,0,1,1},
  {1,0,0,1}
};
// End Settings
// Start variables function move
int countStep[4] = { 0, 0, 0, 0 };
int moveCounter[4] = {0,0,0,0};
uint32_t  moveTimer = 0;
// End variables function move
// Start variables automation
int position = 0;
bool startMove = false;
int counterAuto = 0;
bool autoReverse = false;
// End variables automation
// Start variables function parsePLC
uint32_t  timerParsePLC = 0; 
int counterParsePLC = 5;   
bool resivingPLCData = false;
// End variables function parsePLC

void setup() {
 timerParsePLC = millis();
 moveTimer = micros();
}

void loop() {
  if (micros() - moveTimer < 0){
    moveTimer = micros();
  }
  if (digitalRead(buttonStopAuto) == HIGH ) {
    startMove = false;
  }
  parsePLC();
  if (startMove) {
    digitalWrite(driverReadyToMove, LOW);
    if (manipulatorPosition[position][counterAuto] < countStep[counterAuto]) {
      autoReverse = true;
    } else {
      autoReverse = false;
    }
    move(counterAuto, autoReverse);
    if (manipulatorPosition[position][counterAuto] == countStep[counterAuto]) {
      counterAuto++;
    }
    if (counterAuto == 4) {
      startMove = false;
      counterAuto = 0;
    }
  } else {
    digitalWrite(driverReadyToMove, HIGH); 
    int buttonsActive = (digitalRead(buttonMotorForwardX) == HIGH) ? 1 : 0
      + (digitalRead(buttonMotorForwardY) == HIGH) ? 1 : 0
      + (digitalRead(buttonMotorForwardW) == HIGH) ? 1 : 0
      + (digitalRead(buttonMotorForwardE) == HIGH) ? 1 : 0 
      + (digitalRead(buttonMotorBackwardX) == HIGH) ? 1 : 0
      + (digitalRead(buttonMotorBackwardY) == HIGH) ? 1 : 0
      + (digitalRead(buttonMotorBackwardW) == HIGH) ? 1 : 0
      + (digitalRead(buttonMotorBackwardE) == HIGH) ? 1 : 0
      + (digitalRead(buttonResetPosition) == HIGH)  ? 1 : 0
      + (digitalRead(buttonHomePosition) == HIGH) ? 1 : 0;
    if (buttonsActive > 1) {
      return;
    }
    if (digitalRead(buttonMotorForwardX) == HIGH) {
      move(0, false);
    }
    if (digitalRead(buttonMotorForwardY) == HIGH) {
      move(1, false);
    }
    if (digitalRead(buttonMotorForwardW) == HIGH) {
      move(2, false);
    }
    if (digitalRead(buttonMotorForwardE) == HIGH) {
      move(3, false);
    }
    if (digitalRead(buttonMotorBackwardX) == HIGH) {
      move(0, true);
    }
    if (digitalRead(buttonMotorBackwardY) == HIGH) {
      move(1, true);
    }
    if (digitalRead(buttonMotorBackwardW) == HIGH) {
      move(2, true);
    }
    if (digitalRead(buttonMotorBackwardE) == HIGH) {
      move(3, true);
    }
    if (digitalRead(buttonResetPosition) == HIGH) {
      for (int i = 1; i < 4; i++) {
        countStep[i] *= 0;
      }
    }
    if (digitalRead(buttonHomePosition) == HIGH) {
      moveToHome();
    }
  }
}

void move(int indexMotor, bool reverse) {
  if (micros() - moveTimer > 250000/motorFrequency){
    digitalWrite(pinsMotors[indexMotor][3], motorsValues[moveCounter[indexMotor]][0]);
    digitalWrite(pinsMotors[indexMotor][2], motorsValues[moveCounter[indexMotor]][1]);
    digitalWrite(pinsMotors[indexMotor][1], motorsValues[moveCounter[indexMotor]][2]);
    digitalWrite(pinsMotors[indexMotor][0], motorsValues[moveCounter[indexMotor]][3]);
    moveTimer = micros();
    if (reverse){
      moveCounter[indexMotor]--;
      if (moveCounter[indexMotor] < 0){
      moveCounter[indexMotor] = 3;
      countStep[indexMotor]--;
      }
    }
    else {
      moveCounter[indexMotor]++;
      if (moveCounter[indexMotor] > 3){
        moveCounter[indexMotor] = 0;
        countStep[indexMotor]++;
      }
    }
  }
}

void moveToHome() {
  while (true) {
    if (digitalRead(sensorMaxX) == 0) {
      move(0, true);
    }
    else if (digitalRead(sensorMaxY) == 0) {
      move(1, true);
    }
    else if (digitalRead(sensorMaxW) == 0) {
      move(2, true);
    }
    else if (digitalRead(sensorMaxX) == 1 && digitalRead(sensorMaxY) == 1 && digitalRead(sensorMaxW) == 1) {
      break;
    }
  }
}

int booferPLCData = 0;

void parsePLC() {
  if (digitalRead(dataPLC) == HIGH || resivingPLCData == true){
    if (resivingPLCData == true){
      if (millis() - timerParsePLC >= 100){
        if (digitalRead(dataPLC) == HIGH){
          booferPLCData += 2 ^ counterParsePLC;
        }
        timerParsePLC = millis();
        counterParsePLC--;
      }
    }
    else{
      resivingPLCData = true;
      booferPLCData = 0;
      timerParsePLC = millis();
      counterParsePLC--;
    }
    if (counterParsePLC < 0 ) {
      counterParsePLC = 5; 
      if (booferPLCData == 1 ) {
        startMove = false;
      }
      else if (!startMove){
        startMove = true;
        position = booferPLCData;
      }
      resivingPLCData = false;
    }
  }
}