#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Servo.h>
#include "SafeState.h"
#include "icons.h"

/* Define a structure to hold all components */
struct SafeComponents {
    LiquidCrystal *lcd;
    Keypad *keypad;
    Servo *servo;
};

/* Locking mechanism definitions */
#define SERVO_PIN 6
#define SERVO_LOCK_POS   20
#define SERVO_UNLOCK_POS 90

/* Keypad setup */
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;
byte rowPins[KEYPAD_ROWS] = {5, 4, 3, 2};
byte colPins[KEYPAD_COLS] = {A3, A2, A1, A0};
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);

/* LCD */
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

/* Servo */
Servo lockServo;

/* SafeState stores the secret code in EEPROM */
SafeState safeState;

/* Function prototypes */
void lock(struct SafeComponents *sc);
void unlock(struct SafeComponents *sc);
String inputSecretCode(struct SafeComponents *sc);
bool setNewCode(struct SafeComponents *sc);
void showStartupMessage(struct SafeComponents *sc);
void showUnlockMessage(struct SafeComponents *sc);
void showWaitScreen(struct SafeComponents *sc, int delayMillis);
void safeLockedLogic(struct SafeComponents *sc);
void safeUnlockedLogic(struct SafeComponents *sc);

/* Create an instance of the SafeComponents structure */
struct SafeComponents sc = {&lcd, &keypad, &lockServo};

/* Lock the safe */
void lock(struct SafeComponents *sc) {
    sc->servo->write(SERVO_LOCK_POS);
    safeState.lock();
}

/* Unlock the safe */
void unlock(struct SafeComponents *sc) {
    sc->servo->write(SERVO_UNLOCK_POS);
}

/* Show startup message on the LCD */
void showStartupMessage(struct SafeComponents *sc) {
    sc->lcd->setCursor(4, 0);
    sc->lcd->print("Welcome!");
    delay(1000);

    sc->lcd->setCursor(0, 2);
    String message = "ArduinoSafe v1.0";
    for (byte i = 0; i < message.length(); i++) {
        sc->lcd->print(message[i]);
        delay(100);
    }
    delay(500);
}

/* Input secret code using the keypad */
String inputSecretCode(struct SafeComponents *sc) {
    sc->lcd->setCursor(5, 1);
    sc->lcd->print("[____]");
    sc->lcd->setCursor(6, 1);
    String result = "";
    while (result.length() < 4) {
        char key = sc->keypad->getKey();
        if (key >= '0' && key <= '9') {
            sc->lcd->print('*');
            result += key;
        }
    }
    return result;
}

/* Show wait screen */
void showWaitScreen(struct SafeComponents *sc, int delayMillis) {
    sc->lcd->setCursor(2, 1);
    sc->lcd->print("[..........]");
    sc->lcd->setCursor(3, 1);
    for (byte i = 0; i < 10; i++) {
        delay(delayMillis);
        sc->lcd->print("=");
    }
}

/* Set a new code for the safe */
bool setNewCode(struct SafeComponents *sc) {
    sc->lcd->clear();
    sc->lcd->setCursor(0, 0);
    sc->lcd->print("Enter new code:");
    String newCode = inputSecretCode(sc);

    sc->lcd->clear();
    sc->lcd->setCursor(0, 0);
    sc->lcd->print("Confirm new code");
    String confirmCode = inputSecretCode(sc);

    if (newCode.equals(confirmCode)) {
        safeState.setCode(newCode);
        return true;
    } else {
        sc->lcd->clear();
        sc->lcd->setCursor(1, 0);
        sc->lcd->print("Code mismatch");
        sc->lcd->setCursor(0, 1);
        sc->lcd->print("Safe not locked!");
        delay(2000);
        return false;
    }
}

/* Show unlock message */
void showUnlockMessage(struct SafeComponents *sc) {
    sc->lcd->clear();
    sc->lcd->setCursor(0, 0);
    sc->lcd->write(ICON_UNLOCKED_CHAR);
    sc->lcd->setCursor(4, 0);
    sc->lcd->print("Unlocked!");
    sc->lcd->setCursor(15, 0);
    sc->lcd->write(ICON_UNLOCKED_CHAR);
    delay(1000);
}

/* Logic for when the safe is unlocked */
void safeUnlockedLogic(struct SafeComponents *sc) {
    sc->lcd->clear();
    sc->lcd->setCursor(0, 0);
    sc->lcd->write(ICON_UNLOCKED_CHAR);
    sc->lcd->setCursor(2, 0);
    sc->lcd->print(" # to lock");
    sc->lcd->setCursor(15, 0);
    sc->lcd->write(ICON_UNLOCKED_CHAR);

    bool newCodeNeeded = true;

    if (safeState.hasCode()) {
        sc->lcd->setCursor(0, 1);
        sc->lcd->print("  A = new code");
        newCodeNeeded = false;
    }

    auto key = sc->keypad->getKey();
    while (key != 'A' && key != '#') {
        key = sc->keypad->getKey();
    }

    bool readyToLock = true;
    if (key == 'A' || newCodeNeeded) {
        readyToLock = setNewCode(sc);
    }

    if (readyToLock) {
        sc->lcd->clear();
        sc->lcd->setCursor(5, 0);
        sc->lcd->write(ICON_UNLOCKED_CHAR);
        sc->lcd->print(" ");
        sc->lcd->write(ICON_RIGHT_ARROW);
        sc->lcd->print(" ");
        sc->lcd->write(ICON_LOCKED_CHAR);

        safeState.lock();
        lock(sc);
        showWaitScreen(sc, 100);
    }
}

/* Logic for when the safe is locked */
void safeLockedLogic(struct SafeComponents *sc) {
    sc->lcd->clear();
    sc->lcd->setCursor(0, 0);
    sc->lcd->write(ICON_LOCKED_CHAR);
    sc->lcd->print(" Safe Locked! ");
    sc->lcd->write(ICON_LOCKED_CHAR);

    String userCode = inputSecretCode(sc);
    bool unlockedSuccessfully = safeState.unlock(userCode);
    showWaitScreen(sc, 200);

    if (unlockedSuccessfully) {
        showUnlockMessage(sc);
        unlock(sc);
    } else {
        sc->lcd->clear();
        sc->lcd->setCursor(0, 0);
        sc->lcd->print("Access Denied!");
        showWaitScreen(sc, 1000);
    }
}

/* Setup function */
void setup() {
    sc.lcd->begin(16, 2);
    init_icons(*sc.lcd);

    sc.servo->attach(SERVO_PIN);

    /* Make sure the physical lock is sync with the EEPROM state */
    Serial.begin(115200);
    if (safeState.locked()) {
        lock(&sc);
    } else {
        unlock(&sc);
    }

    showStartupMessage(&sc);
}

/* Loop function */
void loop() {
    if (safeState.locked()) {
        safeLockedLogic(&sc);
    } else {
        safeUnlockedLogic(&sc);
    }
}
