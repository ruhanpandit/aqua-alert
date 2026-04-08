#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// buttons are pins 2 and 3
const int BLUE_BUTTON = 2;
const int GREEN_BUTTON = 3;

void setup() {
  pinMode(BLUE_BUTTON, INPUT_PULLUP);
  pinMode(GREEN_BUTTON, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
}

void loop() {
  lcd.setCursor(0, 0);

  if (digitalRead(BLUE_BUTTON) == HIGH) {
    lcd.clear();
    lcd.print("BLUE");
  } else if(digitalRead(GREEN_BUTTON) == HIGH) {
    lcd.clear();
    lcd.print("GREEN");
  } else {
    lcd.print("Not pressed");
  }

  delay(100);
}