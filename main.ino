#include <LiquidCrystal.h>

// ── Pin Configuration ─────────────────────────────────────────────────────────
const int LCD_PIN_REGISTER_SELECT = 12;
const int LCD_PIN_ENABLE          = 11;
const int LCD_PIN_DATA4           = 5;
const int LCD_PIN_DATA5           = 4;
const int LCD_PIN_DATA6           = 3;
const int LCD_PIN_DATA7           = 2;

const int PIN_BTN_DISMISS = 7;  // Dismisses alarm / selects Timer Mode at startup
const int PIN_BTN_SNOOZE  = 6;  // Snoozes alarm
const int PIN_BTN_REFILL  = 9;  // Logs a refill / selects Refill-Count Mode at startup
const int PIN_BUZZER      = 8;

// ── Constants ─────────────────────────────────────────────────────────────────
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

const unsigned long CLEANING_INTERVAL_MS = 48UL * 60 * 60 * 1000;  // 48 hours
const unsigned long SNOOZE_DURATION_MS   =  6UL * 60 * 60 * 1000;  //  6 hours
const int           REFILLS_PER_CYCLE    = 8;  // refills allowed before cleaning alert

// ── Enums ─────────────────────────────────────────────────────────────────────
enum Mode        { MODE_SELECT, TIMER_MODE, REFILL_MODE };
enum TimerState  { TIMER_RUNNING, TIMER_ALARM };
enum RefillState { REFILL_COUNTING, REFILL_ALARM };

// ── Globals ───────────────────────────────────────────────────────────────────
LiquidCrystal lcd(LCD_PIN_REGISTER_SELECT, LCD_PIN_ENABLE, LCD_PIN_DATA4, LCD_PIN_DATA5, LCD_PIN_DATA6, LCD_PIN_DATA7);

Mode activeMode = MODE_SELECT;

// Timer mode
unsigned long cleaningTimerStartMs    = 0;
unsigned long cleaningTimerDurationMs = CLEANING_INTERVAL_MS;
TimerState    cleaningTimerState      = TIMER_RUNNING;

// Refill-count mode
int         refillsUsed    = 0;
int         refillsPerCycle = REFILLS_PER_CYCLE;
RefillState refillModeState = REFILL_COUNTING;

// Previous button readings for edge detection
bool prevDismissBtn = HIGH;
bool prevSnoozeBtn  = HIGH;
bool prevRefillBtn  = HIGH;

// Timestamp of last LCD countdown redraw (throttled to once per second)
unsigned long lastCountdownRedrawMs = 0;

// ── Helpers ───────────────────────────────────────────────────────────────────

// Returns true on a falling edge (button just pressed).
bool buttonPressed(int pin, bool &prevReading) {
  bool currentReading = digitalRead(pin);
  bool isPressed = (prevReading == HIGH && currentReading == LOW);
  prevReading = currentReading;
  return isPressed;
}

void buzzerOn()  { tone(PIN_BUZZER, 1000); }
void buzzerOff() { noTone(PIN_BUZZER); }

// Write a padded string to one LCD row so leftover characters are cleared.
void lcdLine(int row, const String &text) {
  lcd.setCursor(0, row);
  String paddedText = text;
  while ((int)paddedText.length() < LCD_COLS) paddedText += ' ';
  lcd.print(paddedText.substring(0, LCD_COLS));
}

String formatDuration(unsigned long ms) {
  unsigned long totalSeconds = ms / 1000;
  char timeStr[10];
  snprintf(timeStr, sizeof(timeStr), "%02lu:%02lu:%02lu",
           totalSeconds / 3600,
           (totalSeconds % 3600) / 60,
           totalSeconds % 60);
  return String(timeStr);
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  lcd.begin(LCD_COLS, LCD_ROWS);

  pinMode(PIN_BTN_DISMISS, INPUT_PULLUP);
  pinMode(PIN_BTN_SNOOZE,  INPUT_PULLUP);
  pinMode(PIN_BTN_REFILL,  INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);

  lcdLine(0, "Select Mode:");
  lcdLine(1, "DIS=Timer RFL=Rfil");
  Serial.println("Waiting for mode selection...");
}

// ── Mode Selection ────────────────────────────────────────────────────────────
void handleModeSelect() {
  if (buttonPressed(PIN_BTN_DISMISS, prevDismissBtn)) {
    activeMode            = TIMER_MODE;
    cleaningTimerDurationMs = CLEANING_INTERVAL_MS;
    cleaningTimerStartMs    = millis();
    cleaningTimerState      = TIMER_RUNNING;
    lastCountdownRedrawMs   = 0;  // force immediate redraw
    Serial.println("Timer Mode selected. Cleaning timer: 48 hours.");

    lcdLine(0, "Timer Mode");
    lcdLine(1, "48hr cleaning tmr");
    delay(1200);
  }
  else if (buttonPressed(PIN_BTN_REFILL, prevRefillBtn)) {
    activeMode     = REFILL_MODE;
    refillsUsed    = 0;
    refillsPerCycle = REFILLS_PER_CYCLE;
    refillModeState = REFILL_COUNTING;
    Serial.print("Refill-Count Mode selected. Refills per cycle: ");
    Serial.println(refillsPerCycle);

    lcdLine(0, "Refill-Count Mode");
    lcdLine(1, String("Target: ") + refillsPerCycle + " uses");
    delay(1200);
  }
}

// ── Timer Mode ────────────────────────────────────────────────────────────────
void handleTimerMode() {
  if (cleaningTimerState == TIMER_RUNNING) {
    unsigned long elapsedMs      = millis() - cleaningTimerStartMs;
    unsigned long timeRemainingMs = (elapsedMs >= cleaningTimerDurationMs)
                                      ? 0
                                      : cleaningTimerDurationMs - elapsedMs;

    // Redraw countdown once per second to avoid LCD flicker
    if (millis() - lastCountdownRedrawMs >= 1000) {
      lastCountdownRedrawMs = millis();
      lcdLine(0, "Clean water in:");
      lcdLine(1, formatDuration(timeRemainingMs));
    }

    if (timeRemainingMs == 0) {
      cleaningTimerState = TIMER_ALARM;
      buzzerOn();
      Serial.println("Timer expired! Buzzer on.");
      lcdLine(0, "!! CLEAN WATER !!");
      lcdLine(1, "DIS=rst  SNZ=+6hr");
    }
  }
  else {  // TIMER_ALARM
    if (buttonPressed(PIN_BTN_DISMISS, prevDismissBtn)) {
      buzzerOff();
      cleaningTimerDurationMs = CLEANING_INTERVAL_MS;
      cleaningTimerStartMs    = millis();
      cleaningTimerState      = TIMER_RUNNING;
      lastCountdownRedrawMs   = 0;
      Serial.println("Alarm dismissed. Timer reset to 48 hours.");
    }
    else if (buttonPressed(PIN_BTN_SNOOZE, prevSnoozeBtn)) {
      buzzerOff();
      cleaningTimerDurationMs = SNOOZE_DURATION_MS;
      cleaningTimerStartMs    = millis();
      cleaningTimerState      = TIMER_RUNNING;
      lastCountdownRedrawMs   = 0;
      Serial.println("Snoozed. Timer set to 6 hours.");
    }
  }
}

// ── Refill-Count Mode ─────────────────────────────────────────────────────────
void handleRefillMode() {
  if (refillModeState == REFILL_COUNTING) {
    int refillsRemaining = refillsPerCycle - refillsUsed;
    lcdLine(0, "Refills left:");
    lcdLine(1, String(refillsRemaining) + " of " + refillsPerCycle);

    if (buttonPressed(PIN_BTN_REFILL, prevRefillBtn)) {
      refillsUsed++;
      Serial.print("Refill logged: ");
      Serial.print(refillsUsed);
      Serial.print(" / ");
      Serial.println(refillsPerCycle);

      if (refillsUsed >= refillsPerCycle) {
        refillModeState = REFILL_ALARM;
        buzzerOn();
        Serial.println("Refill threshold reached! Buzzer on.");
        lcdLine(0, "!! CLEAN WATER !!");
        lcdLine(1, "DIS=rst  SNZ=-1");
      }
    }
  }
  else {  // REFILL_ALARM
    if (buttonPressed(PIN_BTN_DISMISS, prevDismissBtn)) {
      buzzerOff();
      refillsUsed     = 0;
      refillModeState = REFILL_COUNTING;
      Serial.println("Alarm dismissed. Refill count reset.");
    }
    else if (buttonPressed(PIN_BTN_SNOOZE, prevSnoozeBtn)) {
      buzzerOff();
      refillsUsed--;  // undo last refill; one more use will re-trigger
      refillModeState = REFILL_COUNTING;
      Serial.println("Snoozed. Refill count decremented by 1.");
    }
  }
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {
  switch (activeMode) {
    case MODE_SELECT: handleModeSelect(); break;
    case TIMER_MODE:  handleTimerMode();  break;
    case REFILL_MODE: handleRefillMode(); break;
  }
  delay(50);  // debounce polling interval
}
