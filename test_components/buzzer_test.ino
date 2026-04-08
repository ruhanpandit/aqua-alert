#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int BUZZER = 5;

void setup() {
  pinMode(BUZZER, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Buzzer test");
}

void loop() {
  tone(BUZZER, 1000);   // 1000 Hz sound
  lcd.setCursor(0, 1);
  lcd.print("ON ");
  delay(1000);

  noTone(BUZZER);       // stop sound
  lcd.setCursor(0, 1);
  lcd.print("OFF");
  delay(1000);
}