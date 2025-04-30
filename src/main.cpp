#include <Arduino.h>
#include <LCD_I2C.h>

LCD_I2C lcd(0x27);

#define LED 2
#define Buzzer 3
#define Sensor A1

#define SafeIndicator 4
#define ModerateIndicator 5
#define DangerIndicator 6

int previousSensorValue = 0;

const int differenceSize = 5;
int differenceHistory[differenceSize] = {0}; 
int differenceHistoryIndex = 0;

void moderateMelody() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(Buzzer, HIGH);
    delay(400);
    digitalWrite(Buzzer, LOW);
    delay(400);
  }
}

void dangerousMelody() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(Buzzer, HIGH);
    delay(200);
    digitalWrite(Buzzer, LOW);
    delay(200);
  }
}

void resetLightIndicators(){
  digitalWrite(SafeIndicator, LOW);
  digitalWrite(ModerateIndicator, LOW);
  digitalWrite(DangerIndicator, LOW);
}

void resetIndicators(){
  digitalWrite(LED, LOW);
  digitalWrite(Buzzer, LOW);
}

void reset(){
  resetIndicators();
  for (int i = 0; i < differenceSize; i++) {
    differenceHistory[i] = 0;
  }
}

void clearLCDBottomRow() {
  lcd.setCursor(0, 1);      
  lcd.print("                "); 
}

void setSafe(){
  resetIndicators();
  clearLCDBottomRow();
  lcd.setCursor(0, 1);
  lcd.print("No Gas Detected");
  resetLightIndicators();
  digitalWrite(SafeIndicator, HIGH);
  delay(400);
}

void LCDSetWarningText(String text){
  lcd.setCursor(0, 1);
  lcd.print(text);
}

void showLightValue(int level){
  resetLightIndicators();
  if (level == 0){
    digitalWrite(SafeIndicator, HIGH);
    digitalWrite(ModerateIndicator, LOW);
    digitalWrite(DangerIndicator, LOW);
  } else if (level == 1){
    digitalWrite(SafeIndicator, LOW);
    digitalWrite(ModerateIndicator, HIGH);
    digitalWrite(DangerIndicator, LOW);
  } else if (level == 2){
    digitalWrite(SafeIndicator, LOW);
    digitalWrite(ModerateIndicator, LOW);
    digitalWrite(DangerIndicator, HIGH);
  }
}

void buzz(int level){
  if (level == 1){
    moderateMelody();
  } else if (level == 2){
    dangerousMelody();
  }
}

void warn(int level) {
  digitalWrite(LED, HIGH);
  digitalWrite(Buzzer, HIGH);
  clearLCDBottomRow();
  
  showLightValue(level);
  LCDSetWarningText("GAS DETECTED!");
  buzz(level);
  
  delay(1000);
  reset(); 
}

void recordDataWithSerialMonitor(String label) {
  Serial.print("Label: ");
  Serial.print(label);
  Serial.print(", History: ");
  for (int i = 0; i < differenceSize; i++) {
    Serial.print(differenceHistory[i]);
    if (i < differenceSize - 1) {
      Serial.print(", ");
    }
  }
  Serial.println();
}

void showValue(int difference){
  lcd.setCursor(0, 0);
  lcd.print("Value : ");
  lcd.print(difference);
  lcd.print("  ");
}

boolean checkForGas(){
  int butaneDifferenceCount = 0;
  for (int i = 0; i < differenceSize; i++){
    if (differenceHistory[i] > 7){
      butaneDifferenceCount++;
    }
  }
  if (butaneDifferenceCount >= 1){
    return true;
  } else {
    return false;
  }
}

int checkForLevel(int difference){
  if (difference < 8){
    return 0; // Safe
  } else if (difference > 7 && difference <= 100){
    return 1; // Moderate
  } else if (difference > 100){
    return 2; // Danger
  } else {
    return -1; // Invalid value
  }
}

void setup() {
  Serial.begin(9200);
  lcd.begin();
  lcd.backlight();
  pinMode(LED, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(Sensor, INPUT);
  pinMode(SafeIndicator, OUTPUT);
  pinMode(ModerateIndicator, OUTPUT);
  pinMode(DangerIndicator, OUTPUT);
}

void loop() {
  int currentValue = analogRead(Sensor);

  int difference = currentValue - previousSensorValue;
  previousSensorValue = currentValue; 

  differenceHistory[differenceHistoryIndex] = difference;
  differenceHistoryIndex++;
  if (differenceHistoryIndex >= differenceSize) {
    differenceHistoryIndex = 0;
  }
  
  showValue(difference);
  
  boolean gasDetected = checkForGas();
  int level = checkForLevel(difference);

  if (gasDetected) {
    recordDataWithSerialMonitor("GAS DETECTED!");
    warn(level);
  } else {
    recordDataWithSerialMonitor("SAFE");
    setSafe();
  }
}