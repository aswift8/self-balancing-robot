#include "Pins.h"
#include "Rgb.h"
#include "voltage.h"
#include "MsTimer2.h"
#include "mode.h"
#include "SendData.h"
#include "InputOutput.h"
#include "WheelController.h"

bool send_raw_readings = false;

void setup() {
    Serial.begin(9600);
    voltageInit();
    rgb.initialize();
    io::ioInit();
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
        switch (c) {
        case 's':
            resetTime();        // SendData method
            mode = MOVE;
            sendMessage(String("Start"));
            controller::reset();
            break;
        case 'e':
            mode = STOP;
            sendMessage(String("Stop"));
            has_connected = false;
            break;
        case 'c':
            sendMessage(String("Custom controller k=32000"));
            controller::setCustomController(32000);
            break;
        case 'm':
            sendMessage(String("Manufacturer controller"));
            controller::setDefaultController();
            break;
        case 'k':{
            binaryFloat val;
            while (Serial.available() < 4)
                delay(10);
            for (unsigned int i=0; i<4; i++)
                val.binary[i] = Serial.read();
            String msg("k=");
            msg += String(val.f);
            for (unsigned int i=0; i<4; i++)
                msg += String(val.binary[i]);
            sendMessage(msg);
            controller::setCustomController(val.f);
            break;}
        case 'r':
            sendMessage(String("Raw MPU readings"));
            send_raw_readings = true;
            break;
        case 'f':
            sendMessage(String("Float readings"));
            send_raw_readings = false;
            break;
        }
    }
    if (low_power) {
        static unsigned long last_message_time = 0;
        if (millis() - last_message_time > 1000) {
            sendError(String("Low power"));
            last_message_time = millis();
        }
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

unsigned int loop_count = 0;
void timerLoop() {
    //
    sei();
    io::updateInput();
    controller::updateController(io::filtered_pos, io::filtered_vel, io::wheel_speed_l, io::wheel_speed_r, io::filtered_vel_z);
    if (low_power || !has_connected)
        io::updateOutput(0, 0);
    else
        io::updateOutput(controller::wheel_output_l, controller::wheel_output_r);
    if (has_connected) {
        if (++loop_count >= 4) {
            loop_count = 0;
            if (send_raw_readings) {
                int vals[] = {
                    io::ax,
                    io::ay,
                    io::az,
                    io::gx,
                    io::gy,
                    io::gz
                };
                sendStampedInts(millis(), 6, vals);
            } else {
                float vals[] = {
                    io::filtered_pos,
                    io::filtered_vel,
                    controller::balance_output,
                    controller::movement_output_l,
                    controller::wheel_output_l
                };
                sendStampedFloats(millis(), 5, vals);
            }
        }
    }
}
