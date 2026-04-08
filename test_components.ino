#include <LiquidCrystal.h>

// ── Pin Configuration ─────────────────────────────────────────────────────────
// LCD (parallel interface: RS, E, D4, D5, D6, D7)
const int LCD_PIN_REGISTER_SELECT = 12;
const int LCD_PIN_ENABLE          = 11;
const int LCD_PIN_DATA4           = 5;
const int LCD_PIN_DATA5           = 4;
const int LCD_PIN_DATA6           = 3;
const int LCD_PIN_DATA7           = 2;

// Button (connects pin to GND when pressed; uses internal pull-up)
const int PIN_BUTTON = 7;

// Buzzer (active or passive buzzer)
const int PIN_BUZZER = 8;

// ── Constants ─────────────────────────────────────────────────────────────────
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

// ── Globals ───────────────────────────────────────────────────────────────────
LiquidCrystal lcd(LCD_PIN_REGISTER_SELECT, LCD_PIN_ENABLE, LCD_PIN_DATA4, LCD_PIN_DATA5, LCD_PIN_DATA6, LCD_PIN_DATA7);

bool lastButtonState = HIGH;  // HIGH = not pressed (pull-up)
int  pressCount      = 0;

// ── Helpers ───────────────────────────────────────────────────────────────────
void beep(int frequency, int durationMs) {
  tone(PIN_BUZZER, frequency, durationMs);
  delay(durationMs);
  noTone(PIN_BUZZER);
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);

  // LCD
  lcd.begin(LCD_COLS, LCD_ROWS);

  // Button with internal pull-up (no external resistor needed)
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  // Buzzer
  pinMode(PIN_BUZZER, OUTPUT);

  // ── LCD test ──────────────────────────────────────────────────────────────
  lcd.setCursor(0, 0);
  lcd.print("LCD:  OK");
  lcd.setCursor(0, 1);
  lcd.print("Testing...");
  Serial.println("LCD initialized.");

  delay(1000);

  // ── Buzzer test ───────────────────────────────────────────────────────────
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Buzzer test");
  Serial.println("Buzzer: beeping...");

  beep(1000, 200);
  delay(100);
  beep(1500, 200);
  delay(100);
  beep(2000, 200);

  lcd.setCursor(0, 1);
  lcd.print("Buzzer: OK");
  Serial.println("Buzzer: OK.");
  delay(1000);

  // ── Ready prompt ──────────────────────────────────────────────────────────
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press button");
  lcd.setCursor(0, 1);
  lcd.print("to test it!");
  Serial.println("Ready. Press the button.");
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {
  bool currentButtonState = digitalRead(PIN_BUTTON);

  // Detect falling edge (HIGH → LOW = press)
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    pressCount++;

    Serial.print("Button pressed! Count: ");
    Serial.println(pressCount);

    // Update LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Button: OK");
    lcd.setCursor(0, 1);
    lcd.print("Presses: ");
    lcd.print(pressCount);

    // Short confirmation beep
    beep(1200, 100);

    delay(50);  // debounce
  }

  lastButtonState = currentButtonState;
}
