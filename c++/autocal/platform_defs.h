#pragma once

// platform defs encapsulate some platform differences
// where possible ;)


#ifdef PEPPER

// assumption, botton 4 inputs have been setup as gates
// note: pepper has no digital outputs
static constexpr unsigned numDigIn = 4;
static constexpr unsigned digInPins[numDigIn] = {11, 9, 13, 12};
static constexpr unsigned numDigOut = 0; // !! <------|
static constexpr unsigned digOutPins[1] = {1}; // !! FAKE !!
static constexpr unsigned numButton = 2;
static constexpr unsigned buttonPins[numButton] = {15, 14};
static constexpr unsigned numLeds = 10;
static constexpr unsigned ledPins[numLeds] = {6, 7, 10, 2, 3, 0, 1, 4, 5, 8 };

static constexpr float voltInRange = 10.0f;
static constexpr float voltInMin = 0.0f;
static constexpr float voltInMax = 10.0f;
static constexpr float voltOutRange = 5.0f;
static constexpr float voltOutMin = 0.0f;
static constexpr float voltOutMax = 5.0f;


#else // !PEPPER
#ifdef SALT

// for now, I'll assume expander
// i'll also count trigs as both buttons and trigs,
// note: digPin[0]=trig1, but buttonPin[0] = switch1
static constexpr unsigned numDigIn = 4;
static constexpr unsigned digInPins[numDigIn] = {15, 14, 1, 3};
static constexpr unsigned numDigOut = 4;
static constexpr unsigned digOutPins[numDigOut] = {0, 5, 12, 13};
static constexpr unsigned numButton = 4;
static constexpr unsigned buttonPins[numButton] = {6, 14, 1, 3};
static constexpr unsigned numLeds = 4;
static constexpr unsigned ledPins[numLeds] = {2, 4, 8, 9};
static constexpr unsigned pwmPin = 7;


static constexpr float voltInRange = 10.0f;
static constexpr float voltInMin = -5.0f;
static constexpr float voltInMax = 5.0f;
static constexpr float voltOutRange = 10.0f;
static constexpr float voltOutMin = -5.0f;
static constexpr float voltOutMax = 5.0f;

#else // !PEPPER OR SALT -> BELA

// bela, is completely open
// but lets assume inPin1 = button and outPin1 = led
// this allows at least some basic UI ?
static constexpr unsigned numDigIn = 8;
static constexpr unsigned digInPins[numDigIn] = {1, 2, 3, 4, 5, 6, 7, 8};
static constexpr unsigned numDigOut = 8;
static constexpr unsigned digOutPins[numDigOut] = {1, 2, 3, 4, 5, 6, 7, 8};
static constexpr unsigned numButton = 1;
static constexpr unsigned buttonPins[numButton] = {1};
static constexpr unsigned numLeds = 1;
static constexpr unsigned ledPins[numLeds] = {1};

static constexpr float voltInRange = 5.0f;
static constexpr float voltInMin = 0.0f;
static constexpr float voltInMax = 5.0f;
static constexpr float voltOutRange = 5.0f;
static constexpr float voltOutMin = 0.0f;
static constexpr float voltOutMax = 5.0f;


#endif // SALT
#endif // PEPPER

bool buttonState[numButton];
bool buttonRiseEdge[numButton];
bool digInState[numButton];
bool digInRiseEdge[numButton];

void initDigital(BelaContext* context) {
    // init inputs
    for (unsigned n = 0; n < numButton; n++) {
        pinMode(context, 0, buttonPins[n], INPUT);
    }
    for (unsigned n = 0; n < numDigIn; n++) {
        pinMode(context, 0, digInPins[n], INPUT);
    }
    // init outputs
    for (unsigned n = 0; n < numLeds; n++) {
        pinMode(context, 0, ledPins[n], OUTPUT);
    }
    for (unsigned n = 0; n < numDigOut; n++) {
        pinMode(context, 0, digOutPins[n], OUTPUT);
    }
}

void dsRead(BelaContext* context) {
    // bool prev = false;

    // clear rising edge state
    for (unsigned n = 0; n < numButton; n++) {
        buttonRiseEdge[n] = false;
    }
    for (unsigned n = 0; n < numDigIn; n++) {
        digInRiseEdge[n] = false;
    }

    // read in states, and look for rising edge
    for (unsigned f = 0; f < context->digitalFrames; ++f) {
        for (unsigned n = 0; n < numButton; n++) {
            bool prev = buttonState[n];
            buttonState[n] = digitalRead(context, f, buttonPins[n]);
            buttonRiseEdge[n] = buttonRiseEdge[n] ||  (!prev && buttonState[n]); //trig = rising edge, any time in this render
        }

        for (unsigned n = 0; n < numDigIn; n++) {
            bool prev = digInState[n];
            digInState[n] = digitalRead(context, f, digInPins[n]);
            digInRiseEdge[n] = digInRiseEdge[n] ||  (!prev && digInState[n]);
        }
    }
}

bool dsIsButton(unsigned n) {
    if (n >= numButton) return false;
    return buttonState[n];
}

bool dsIsButtonTrig(unsigned n) {
    if (n >= numButton) return false;
    return buttonRiseEdge[n];
}

bool dsIsDigIn(unsigned n) {
    if (n >= numDigIn) return false;
    return digInState[n];
}

bool dsIsDigInTrig(unsigned n) {
    if (n >= numDigIn) return false;
    return digInRiseEdge[n];
}


// inefficient, but  useful for testing!
// use digital state for more efficient method.
bool isButtonDown(BelaContext* context, unsigned n) {
    if (n >= numButton) return false;

    for (unsigned int f = 0; f < context->digitalFrames; ++f) {
        if (digitalRead(context, f, buttonPins[n])) {
            return true;
        }
    }
    return false;
}

void setDigOut(BelaContext* context, unsigned n,  bool v) {
    if (n >= numDigOut) return;
    digitalWrite(context, 0, digOutPins[n], v);
}


void setLed(BelaContext* context, unsigned n,  unsigned color) {
    if (n >= numLeds) return;

    unsigned ledPin = ledPins[n];
#ifdef SALT
    if (color == 0) {
        pinMode(context, 0, ledPin, INPUT);
        return;
    }

    pinMode(context, 0, ledPin, OUTPUT);
    digitalWrite(context, 0, ledPin, color > 1);
#else
    digitalWrite(context, 0, ledPin, color > 0);
#endif
}

// this might be used to set brightness on other boards e.g. pepper.
void drivePwm(BelaContext* context) {
#ifdef SALT
    // count controls brightness, flipping = brightest
    static unsigned count = 0;
    pinMode(context, 0, pwmPin, OUTPUT);
    for (unsigned f = 0; f < context->digitalFrames; ++f) {
        digitalWriteOnce(context, f, pwmPin, count & 1);
        count++;
    }
#else
    ;
#endif
}
