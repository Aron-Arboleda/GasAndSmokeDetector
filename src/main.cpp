#include <Arduino.h>
#include <LCD_I2C.h>

LCD_I2C lcd(0x27);

#define LED 2
#define Buzzer 3
#define Sensor A1

// Global variables to store past sensor values
int previousSensorValue = 0;

const int differenceSize = 5; // Number of past differences to store
int differenceHistory[differenceSize] = {0}; // Array to store past differences
int differenceHistoryIndex = 0; // Index for circular buffer

void setup() {
  Serial.begin(9200);
  lcd.begin();
  lcd.backlight();
  pinMode(LED, OUTPUT);
  pinMode(Buzzer, OUTPUT);
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
  lcd.print("Gas: None");
  delay(400);
}



void LCDSetWarningText(String text){
  lcd.setCursor(0, 1);
  lcd.print(text);
}

void warn(int gasNumber = 0) {
  digitalWrite(LED, HIGH);
  digitalWrite(Buzzer, HIGH);
  clearLCDBottomRow();
  int delayTime = 0;
  if (gasNumber == 0) {
    LCDSetWarningText("Gas: BUTANE!");
    delayTime = 3000;
  } else if (gasNumber == 1) {
    LCDSetWarningText("Gas: ETHANOL!");
    delayTime = 2000;
  } else if (gasNumber == 2) {
    LCDSetWarningText("Gas: SMOKE!");
    delayTime = 1000;
  }
  delay(delayTime);
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
  lcd.print("Difference :");
  lcd.print(difference);
  lcd.print("  ");
}

// int counter = 0;

boolean checkForButane(){
  int butaneDifferenceCount = 0;
  for (int i = 0; i < differenceSize; i++){
    if (differenceHistory[i] > 150){
      butaneDifferenceCount++;
    }
  }
  if (butaneDifferenceCount >= 1){
    return true; // Butane detected
  } else {
    return false; // No butane detected
  }
}

boolean checkForEthanol(){
  int ethanolDifferenceCount = 0;
  for (int i = 0; i < differenceSize; i++){
    if (differenceHistory[i] > 11 && differenceHistory[i] <= 100){
      ethanolDifferenceCount++;
    }
  }
  if (ethanolDifferenceCount >= 2){
    return true; // Ethanol detected
  } else {
    return false; // No ethanol detected
  }
}

boolean checkForSmoke(){
  int smokeDifferenceCount = 0;
  for (int i = 0; i < differenceSize; i++){
    if (differenceHistory[i] > 4 && differenceHistory[i] <= 10){
      smokeDifferenceCount++;
    }
  }
  if (smokeDifferenceCount >= 3){
    return true; // Smoke detected
  } else {
    return false; // No smoke detected
  }
}

void loop() {
  int currentValue = analogRead(Sensor);

  int difference = currentValue - previousSensorValue;
  previousSensorValue = currentValue; 

  differenceHistory[differenceHistoryIndex] = difference;
  differenceHistoryIndex++;
  if (differenceHistoryIndex >= differenceSize) {
    differenceHistoryIndex = 0; // Reset index if it exceeds the size
  }
  
  showValue(difference);
  
  boolean butaneDetected = checkForButane();
  boolean ethanolDetected = checkForEthanol();
  boolean smokeDetected = checkForSmoke();

  if (butaneDetected) { // Gas detection (large spike)
    recordDataWithSerialMonitor("BUTANE");
    warn(0);
  } else if (ethanolDetected) { // Smoke detection (small spike)
    recordDataWithSerialMonitor("ETHANOL");
    warn(1);
  } else if (smokeDetected) { // Smoke detection (small spike)
    recordDataWithSerialMonitor("SMOKE");
    warn(2);
  } else {
    recordDataWithSerialMonitor("SAFE");
    setSafe();
  }

}



// int differencesSum = 0;
//   for (int i = 0; i < differenceSize; i++) {
//     differencesSum += differenceHistory[i];
//   }
//   int averageDifference = differencesSum / differenceSize;