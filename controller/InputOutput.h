#ifndef InputOutput_h
#define InputOutput_h

#include "Pins.h"
#include "PinChangeInt.h"
#include "Wire.h"
#include "MPU6050.h"
#include "SendData.h"

MPU6050 mpu;
unsigned long ts;
int ax, ay, az, gx, gy, gz;

int wheel_speed_l, wheel_speed_r;
int output_speed_l, output_speed_r;

unsigned long encoder_count_l, encoder_count_r;
void incrementEncoderL() { encoder_count_l++; }
void incrementEncoderR() { encoder_count_r++; }

void ioInit() {
    pinMode(AIN1, OUTPUT);        // Left motor forwards/backwards
    pinMode(BIN1, OUTPUT);        // Right motor forwards/backwards
    pinMode(PWMA_LEFT, OUTPUT);   // Left motor value
    pinMode(PWMB_RIGHT, OUTPUT);  // Right motor value
    pinMode(STBY_PIN, OUTPUT);
    Wire.begin();
    delay(1000);
    mpu.initialize();
    attachInterrupt(digitalPinToInterrupt(ENCODER_LEFT_A_PIN), incrementEncoderL, CHANGE);
    attachPinChangeInterrupt(ENCODER_RIGHT_A_PIN, incrementEncoderR, CHANGE);
}

void updateInput() {
    wheel_speed_l = encoder_count_l * (output_speed_l >= 0 ? 1 : -1);
    wheel_speed_r = encoder_count_r * (output_speed_r >= 0 ? 1 : -1);
    ts = millis();
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    
    float dt = 0.005, Q_angle = 0.001, Q_gyro = 0.005, R_angle = 0.5, C_0 = 1, K1 = 0.05;
}

void updateOutput(int pwm_l, int pwm_r) {
    output_speed_l = constrain(pwm_l, -255, 255);
    output_speed_r = constrain(pwm_r, -255, 255);
    if (pwm_l < 0) {
        digitalWrite(AIN1, 1);
        analogWrite(PWMA_LEFT, -output_speed_l);
    } else {
        digitalWrite(AIN1, 0);
        analogWrite(PWMA_LEFT, output_speed_l);
    }
    if (pwm_r < 0) {
        digitalWrite(BIN1, 1);
        analogWrite(PWMB_RIGHT, -output_speed_r);
    } else {
        digitalWrite(BIN1, 0);
        analogWrite(PWMB_RIGHT, output_speed_r);
    }
}


#endif
