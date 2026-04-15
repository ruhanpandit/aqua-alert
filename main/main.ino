// lcd setup
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

// const variables for components
const int BLUE_BUTTON = 2;
const int GREEN_BUTTON = 3;
const int BUZZER = 5;

// variables to prevent repeating of text on lcd
int mode = 0;
int refills_left = 2;
int lastModeShown = -1;
bool refillShown = false;

// buzzer variables
bool buzzerOn = false;
bool shouldBuzz = false;
unsigned long lastBuzzToggle = 0;
const unsigned long buzzInterval = 300; // ms

// timer mode variables
unsigned long timerStart = 0;
bool timerStarted = false;
bool timerAlarm = false;
const unsigned long timerDuration = 10000; // 10 seconds (for testing)
unsigned long lastDisplayUpdate = 0;

// select mode screen
void selectMode() {
    mode = 0;
    lastModeShown = -1;

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

    // clear lcd to prepare for tracking
    lcd.clear();
    delay(500);
}

// reset all variables when returning to mode select
void resetState() {
    refills_left = 2;
    refillShown = false;
    shouldBuzz = false;
    buzzerOn = false;
    noTone(BUZZER);
    timerStarted = false;
    timerAlarm = false;
    lastDisplayUpdate = 0;
    lastModeShown = -1;
}

// setup
void setup() {
    // assign component pins
    pinMode(BLUE_BUTTON, INPUT_PULLUP);
    pinMode(GREEN_BUTTON, INPUT_PULLUP);
    pinMode(BUZZER, OUTPUT);

    // initalize lcd
    lcd.init();
    lcd.backlight();

    delay(500);
    selectMode();

    Serial.begin(9600);
}

// method for controlling buzzer in the background
// on/off/on/off functionality similar to an alarm clock
void updateBuzzer() {
    if (shouldBuzz) {
        unsigned long now = millis();
        if (now - lastBuzzToggle >= buzzInterval) {
            lastBuzzToggle = now;
            buzzerOn = !buzzerOn;

            if (buzzerOn) {
                //tone(BUZZER, 1000);
                Serial.println("Buzzer on");
            } else {
                //noTone(BUZZER);
                Serial.println("Buzzer off");
            }
        }
    } else {
        noTone(BUZZER);
        buzzerOn = false;
    }
}

// refill mode
void refill() {
    // print refills left
    if (refills_left > 0 && lastModeShown != 1) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Refills left:");
        lcd.setCursor(0, 1);
        lcd.print(refills_left);
        lastModeShown = 1;
    }

    // remove refills if blue button is pressed
    if (refills_left > 0 && digitalRead(BLUE_BUTTON) == HIGH) {
        delay(200);
        refills_left--;
        lastModeShown = -1;
    }

    // display reset message if no refills left
    if (refills_left == 0 && !refillShown) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("CLEAN BOTTLE");
        lcd.setCursor(0, 1);
        lcd.print("Press green");
        refillShown = true;
    }

    // activate buzzer when no refills left
    if (refills_left == 0 && refillShown) {
        shouldBuzz = true;
    }

    // turn off buzzer if user acknowledge buzzer
    if (digitalRead(GREEN_BUTTON) == HIGH && refills_left == 0 && refillShown) {
        delay(200);
        refills_left = 2;
        refillShown = false;
        shouldBuzz = false;
        lastModeShown = -1;
        return;
    }

    // green button goes back to mode select if buzzer is not going off
    if (digitalRead(GREEN_BUTTON) == HIGH && !shouldBuzz) {
        delay(200);
        resetState();
        selectMode();
        return;
    }

    // constantly call this function
    // function checks if no refills left and starts buzzer accordingly
    updateBuzzer();
}

void timer() {
    // start timer for the first time
    if (!timerStarted) {
        timerStart = millis();
        timerStarted = true;
    }

    // check if timer expired
    if (!timerAlarm && (millis() - timerStart >= timerDuration)) {
        timerAlarm = true;
        lastModeShown = -1;
    }

    // activate alarm and show message
    if (timerAlarm) {
        shouldBuzz = true;

        // only update lcd one time
        if (lastModeShown != 3) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("CLEAN BOTTLE");
            lcd.setCursor(0, 1);
            lcd.print("Press green");
            lastModeShown = 3;
        }

        // turn off buzzer and reset variables if user acknowledge buzzer
        if (digitalRead(GREEN_BUTTON) == HIGH) {
            delay(200);
            shouldBuzz = false;
            timerAlarm = false;
            timerStart = millis();
            lastModeShown = -1;
            lastDisplayUpdate = 0;
        }
    }
    // show countdown if no alarm
    else {
        shouldBuzz = false;

        // green button goes back to mode select if buzzer is not going off
        if (digitalRead(GREEN_BUTTON) == HIGH) {
            delay(200);
            resetState();
            selectMode();
            return;
        }

        // clear lcd when first entering countdown display
        if (lastModeShown != 2) {
            lcd.clear();
            lastModeShown = 2;
            lastDisplayUpdate = 0; // force display update
        }

        // calculate time elapsed
        unsigned long elapsed = millis() - timerStart;
        unsigned long remaining = (timerDuration - elapsed) / 1000;

        // refresh lcd every 1 second
        if (millis() - lastDisplayUpdate >= 1000 || lastDisplayUpdate == 0) {
            lastDisplayUpdate = millis();

            // split remaining seconds into hours, minutes, and seconds
            unsigned int hrs = remaining / 3600;
            unsigned int mins = (remaining % 3600) / 60;
            unsigned int secs = remaining % 60;

            // display countdown in MM:SS format
            lcd.setCursor(0, 0);
            lcd.print("Next clean in:");
            lcd.setCursor(0, 1);

            if (hrs < 10) {
                lcd.print("0");
            }
            lcd.print(hrs);
            lcd.print(":");

            if (mins < 10) {
                lcd.print("0");
            }
            lcd.print(mins);
            lcd.print(":");

            if (secs < 10) {
                lcd.print("0");
            }
            lcd.print(secs);
            lcd.print(" ");
        }
    }

    updateBuzzer();
}

void loop() {
    // choose mode and constantly loop through
    if (mode == 1) {
        refill();
    }
    else if (mode == 2) {
        timer();
    }
}