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
unsigned long lastBuzzToggle = 0;
const unsigned long buzzInterval = 300; // ms

// setup
void setup() {
    // assign component pins
    pinMode(BLUE_BUTTON, INPUT_PULLUP);
    pinMode(GREEN_BUTTON, INPUT_PULLUP);
    pinMode(BUZZER, OUTPUT);

    // initalize lcd
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Select mode");
    lcd.setCursor(0, 1);
    lcd.print("B:refill G:timer");

    // user selects mode (refill or timer)
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

// method for controlling buzzer in the background
// on/off/on/off functionality similar to an alarm clock
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
        lcd.print("REFILL");
        lcd.setCursor(0, 1);
        lcd.print("Press green");
        refillShown = true;
    }

    // turn off buzzer if user acknowledge buzzer
    if (digitalRead(GREEN_BUTTON) == HIGH && refills_left == 0 && refillShown) {
        refills_left = 2;
        refillShown = false;
        lastModeShown = -1;
        noTone(BUZZER);
    }

    // constantly call this function
    // function checks if no refills left and starts buzzer accordingly
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
    // choose mode and constantly loop through
    if (mode == 1) {
        refill();
    }
    else if (mode == 2) {
        timer();
    }
}