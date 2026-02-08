#include "Button.h"

// ================== CONFIG ==================
#define DEBOUNCE_TIME 30      // ms
#define HOLD_TIME 800         // ms
#define DOUBLE_CLICK_TIME 400 // ms
// ============================================

// ====== USER CALLBACKS (IMPLEMENT THESE) =====
void onSinglePress()
{
    Serial.println("Single Press detected");
    UWB_switchMode();
}

void onDoublePress()
{
    Serial.println("Double Press detected");
    if (isRanging)
    {
        UWB_stop();
    }
    else
    {
        UWB_start();
    }
}

void onHold()
{
    Serial.println("Hold detected");
}
// ============================================

static bool lastReading = false;
static bool pressed = false;
static bool holdTriggered = false;

static unsigned long lastDebounceTime = 0;
static unsigned long pressTime = 0;
static unsigned long lastReleaseTime = 0;

static uint8_t pressCount = 0;

// ============================================
void Button_setup()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

// Call this from loop()
void Button_update()
{
    bool reading = !digitalRead(BUTTON_PIN); // active LOW
    unsigned long now = millis();

    // Debounce
    if (reading != lastReading)
    {
        lastDebounceTime = now;
    }

    if (now - lastDebounceTime > DEBOUNCE_TIME)
    {

        // Button pressed
        if (reading && !pressed)
        {
            pressed = true;
            pressTime = now;
        }

        // Button released
        if (!reading && pressed)
        {
            pressed = false;

            if (!holdTriggered)
            {
                pressCount++;
                lastReleaseTime = now;
            }
            holdTriggered = false;
        }

        // Hold detection
        if (pressed && !holdTriggered && (now - pressTime >= HOLD_TIME))
        {
            holdTriggered = true;
            pressCount = 0;
            onHold();
        }

        // Single / Double press
        if (!pressed && pressCount > 0 &&
            (now - lastReleaseTime >= DOUBLE_CLICK_TIME))
        {

            if (pressCount == 1)
                onSinglePress();
            else if (pressCount == 2)
                onDoublePress();

            pressCount = 0;
        }
    }

    lastReading = reading;
}
