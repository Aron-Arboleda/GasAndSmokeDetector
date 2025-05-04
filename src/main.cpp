#include <Arduino.h>
#include <LCD_I2C.h>

LCD_I2C lcd(0x27);

#define Buzzer 3
#define Sensor A1

#define SafeIndicator 4
#define ModerateIndicator 5
#define DangerIndicator 6

int previousSensorValue = 0;

enum Thresholds {
  SAFE_THRESHOLD = 100,
  MODERATE_THRESHOLD = 400,
  DIFFERENCE_THRESHOLD = 7
};

enum GasLevel {
  SAFE = 0,
  MODERATE = 1,
  DANGEROUS = 2
};

unsigned long buzzPrevTime = 0;
bool buzzState = LOW;
int buzzCount = 0;
bool isBuzzing = false;
GasLevel buzzingLevel = SAFE;

void updateBuzzerControl() {
  unsigned long currentMillis = millis();
  unsigned int interval = (buzzingLevel == MODERATE) ? 400 : 200;

  if (isBuzzing && buzzCount < 4) {
    if (currentMillis - buzzPrevTime >= interval) {
      buzzPrevTime = currentMillis;
      buzzState = !buzzState;
      digitalWrite(Buzzer, buzzState);
      if (!buzzState) buzzCount++;
    }
  } else {
    isBuzzing = false;
    buzzCount = 0;
    digitalWrite(Buzzer, LOW);
  }
}

void resetLightIndicators() {
  digitalWrite(SafeIndicator, LOW);
  digitalWrite(ModerateIndicator, LOW);
  digitalWrite(DangerIndicator, LOW);
}

void clearLCDBottomRow() {
  lcd.setCursor(0, 1);
  lcd.print("                ");
}

void setSafe() {
  clearLCDBottomRow();
  lcd.setCursor(0, 1);
  lcd.print("No Gas Detected");

  digitalWrite(SafeIndicator, HIGH);
  digitalWrite(ModerateIndicator, LOW);
  digitalWrite(DangerIndicator, LOW);
}

void LCDSetWarningText(String text) {
  lcd.setCursor(0, 1);
  lcd.print(text);
}

void showLightLevel(GasLevel level) {
  resetLightIndicators();
  if (level == MODERATE) {
    digitalWrite(ModerateIndicator, HIGH);
  } else if (level == DANGEROUS) {
    digitalWrite(DangerIndicator, HIGH);
  }
}

void warn(GasLevel level) {
  clearLCDBottomRow();
  LCDSetWarningText("GAS DETECTED!");
  showLightLevel(level);

  if (!isBuzzing) {
    isBuzzing = true;
    buzzingLevel = level;
    buzzCount = 0;
    buzzState = LOW;
    buzzPrevTime = millis();
  }
}

void recordDataWithSerialMonitor(String label, int currentValue, int difference) {
  Serial.print("Label: ");
  Serial.print(label);
  Serial.print(", Value: ");
  Serial.print(currentValue);
  Serial.print(", Difference: ");
  Serial.println(difference);
}

void showValueOnLCD(int value) {
  lcd.setCursor(0, 0);
  lcd.print("Value : ");
  lcd.print(value);
  lcd.print("  ");
}

boolean checkForGas(int currentValue, int difference) {
  return (currentValue >= SAFE_THRESHOLD || difference > DIFFERENCE_THRESHOLD);
}

GasLevel checkForLevel(int value) {
  if (value > MODERATE_THRESHOLD) {
    return DANGEROUS;
  }
  return MODERATE;
}

void setup() {
  Serial.begin(9200);
  lcd.begin();
  lcd.backlight();
  pinMode(Buzzer, OUTPUT);
  pinMode(Sensor, INPUT);
  pinMode(SafeIndicator, OUTPUT);
  pinMode(ModerateIndicator, OUTPUT);
  pinMode(DangerIndicator, OUTPUT);
}

void loop() {
  int currentValue = analogRead(Sensor);
  int difference = currentValue - previousSensorValue;

  showValueOnLCD(currentValue);
  boolean gasDetected = checkForGas(currentValue, difference);

  if (gasDetected) {
    recordDataWithSerialMonitor("GAS DETECTED!", currentValue, difference);
    GasLevel level = checkForLevel(currentValue);
    warn(level);
  } else {
    recordDataWithSerialMonitor("SAFE", currentValue, difference);
    setSafe();
  }

  previousSensorValue = currentValue;
  updateBuzzerControl();

  delay(300);
}
