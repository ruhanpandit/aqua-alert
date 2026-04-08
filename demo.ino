#include <LiquidCrystal.h>

// Pins
const int LCD_PIN_REGISTER_SELECT = 12;
const int LCD_PIN_ENABLE          = 11;
const int LCD_PIN_DATA4           = 5;
const int LCD_PIN_DATA5           = 4;
const int LCD_PIN_DATA6           = 3;
const int LCD_PIN_DATA7           = 2;

const int RESET_BUTTON_PIN = 7;
const int BUZZER_PIN       = 8;

// Settings
const int CLEANING_INTERVAL_HOURS = 48;

// Global Variables
LiquidCrystal lcd(LCD_PIN_REGISTER_SELECT, LCD_PIN_ENABLE,
                  LCD_PIN_DATA4, LCD_PIN_DATA5, LCD_PIN_DATA6, LCD_PIN_DATA7);

unsigned long timerStartMs    = 0;
unsigned long intervalMs      = (unsigned long)CLEANING_INTERVAL_HOURS * 60 * 60 * 1000;
bool          alarmIsRinging  = false;

// Setup
void setup() {
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  lcd.begin(16, 2);
  lcd.print("Aqua Alert!");
  delay(1500);
  lcd.clear();

  timerStartMs = millis();
}

// Loop
void loop() {
  unsigned long elapsed   = millis() - timerStartMs;
  unsigned long remaining;
  if (elapsed >= intervalMs) {
    remaining = 0;
  }
  else {
    remaining = intervalMs - elapsed;
  }

  // When timer runs out, ring the alarm
  if (remaining == 0 && !alarmIsRinging) {
    alarmIsRinging = true;
    tone(BUZZER_PIN, 1000);
  }

  // Show the right screen depending on state
  if (alarmIsRinging) {
    lcd.setCursor(0, 0);
    lcd.print("!! Clean water !! ");
    lcd.setCursor(0, 1);
    lcd.print("Press to reset    ");
  } else {
    // Display time remaining as HH:MM:SS
    unsigned long totalSeconds = remaining / 1000;
    int hours   = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    char timeDisplay[17];

    // write info into char array timeDisplay
    snprintf(timeDisplay, sizeof(timeDisplay), "Clean in %02d:%02d:%02d", hours, minutes, seconds);

    lcd.setCursor(0, 0);
    lcd.print(timeDisplay);
    lcd.setCursor(0, 1);
    lcd.print("Water filter demo ");
  }

  // Reset button stops the alarm and restarts the timer
  if (digitalRead(RESET_BUTTON_PIN) == LOW) {
    noTone(BUZZER_PIN);
    alarmIsRinging = false;
    timerStartMs   = millis();
    lcd.clear();
    delay(300);  // debounce
  }

  delay(500);
}
