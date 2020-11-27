#include "Pins.h"
#include "Rgb.h"
#include "voltage.h"
#include "MsTimer2.h"
#include "mode.h"

void setup() {
    voltageInit();
    rgb.initialize();
    MsTimer2::set(5, timerLoop);
    mode = MOVE;
    MsTimer2::start();
}

void loop() {
    voltageMeasure();
    if (low_voltage_flag == 1) {
        rgb.brightRedColor();
        mode = LOW_POWER;
    } else {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == 's')
                mode = MOVE;
            else if (c == 'e')
                mode = STOP;
        }
    }
    switch(mode) {
        case MOVE:
            rgb.brightGreenColor();
            break;
        case STOP:
            rgb.brightYellowColor();
            break;
        case LOW_POWER:
            rgb.brightRedColor();
            break;
    }
    rgb.blink(100);
}

void timerLoop() {
    //
}
