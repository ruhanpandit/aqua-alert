#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int BLUE_BUTTON = 2;
const int GREEN_BUTTON = 3;
const int BUZZER = 5;

int mode = 0;
int refills_left = 2;
int lastModeShown = -1;
bool refillShown = false;

bool buzzerOn = false;
unsigned long lastBuzzToggle = 0;
const unsigned long buzzInterval = 300; // ms

void setup() {
    pinMode(BLUE_BUTTON, INPUT_PULLUP);
    pinMode(GREEN_BUTTON, INPUT_PULLUP);
    pinMode(BUZZER, OUTPUT);

    lcd.init();
    lcd.backlight();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Select mode");
    lcd.setCursor(0, 1);
    lcd.print("B:refill G:timer");

    while (mode == 0) {
        if (digitalRead(BLUE_BUTTON) == HIGH) {
            mode = 1;
        } 
        else if (digitalRead(GREEN_BUTTON) == HIGH) {
            mode = 2;
        }
    }

    lcd.clear();
    delay(500);
}

void updateBuzzer() {
    if (refills_left == 0 && refillShown) {
        unsigned long now = millis();
        if (now - lastBuzzToggle >= buzzInterval) {
            lastBuzzToggle = now;
            buzzerOn = !buzzerOn;

            if (buzzerOn) {
                tone(BUZZER, 1000);
            } else {
                noTone(BUZZER);
            }
        }
    } else {
        noTone(BUZZER);
        buzzerOn = false;
    }
}

void refill() {
    if (refills_left > 0 && lastModeShown != 1) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Refills left:");
        lcd.setCursor(0, 1);
        lcd.print(refills_left);
        lastModeShown = 1;
    }

    if (refills_left > 0 && digitalRead(BLUE_BUTTON) == HIGH) {
        delay(200);
        refills_left--;
        lastModeShown = -1;
    }

    if (refills_left == 0 && !refillShown) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("REFILL");
        lcd.setCursor(0, 1);
        lcd.print("Press green");
        refillShown = true;
    }

    if (digitalRead(GREEN_BUTTON) == HIGH && refills_left == 0 && refillShown) {
        refills_left = 2;
        refillShown = false;
        lastModeShown = -1;
        noTone(BUZZER);
    }

    updateBuzzer();
}

void timer() {
    noTone(BUZZER);

    if (lastModeShown != 2) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Timer mode");
        lastModeShown = 2;
    }
}

void loop() {
    if (mode == 1) {
        refill();
    }
    else if (mode == 2) {
        timer();
    }
}