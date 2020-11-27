#include "Pins.h"
#include "Rgb.h"
#include "voltage.h"
#include "MsTimer2.h"
#include "mode.h"
#include "SendData.h"
#include "InputOutput.h"

void setup() {
    Serial.begin(9600);
    voltageInit();
    rgb.initialize();
    ioInit();
    MsTimer2::set(5, timerLoop);
    mode = STOP;
    MsTimer2::start();
}

bool has_connected = false;
bool low_power = true;
void loop() {
    voltageMeasure();
    low_power = low_voltage_flag == 1;
    if (Serial.available()) {
        has_connected = true;
        char c = Serial.read();
        if (c == 's') {
            mode = MOVE;
            sendMessage(String("Start"));
        } else if (c == 'e') {
            mode = STOP;
            sendMessage(String("Stop"));
            has_connected = false;
        }
    }
    if (low_power) {
        sendError(String("Low power"));
        rgb.brightRedColor();
    } else {
        switch(mode) {
            case MOVE:
                rgb.brightGreenColor();
                break;
            case STOP:
                rgb.brightYellowColor();
                break;
        }
    }
    rgb.blink(100);
}

void timerLoop() {
    //
    sei();
    updateInput();
    // Update controller
    if (low_power || !has_connected)
        updateOutput(0, 0);
    else
        updateOutput(50, 50);
}
