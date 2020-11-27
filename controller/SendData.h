#ifndef SendData_h
#define SendData_h

unsigned long start_time = 0;

void sendMessage(const String& msg) {
    // 1[0] 1[length] length[message]
    if (2 + msg.length() > 255)
        return;
    byte message_size = msg.length();
    if (Serial.availableForWrite() < message_size + 2)
        return;
    Serial.write(byte(0));
    Serial.write(message_size);
    Serial.write(msg.c_str());
}

void sendError(const String& err) {
    // 1[1] 1[length] length[error]
    if (2 + err.length() > 255)
        return;
    byte error_size = err.length();
    if (Serial.availableForWrite() < error_size + 2)
        return;
    Serial.write(byte(1));
    Serial.write(error_size);
    Serial.write(err.c_str());
}

typedef union {
    unsigned long timestamp;
    byte binary[4];
} binaryTimestamp;

typedef union {
    int i;
    byte binary[2];
} binaryInt;

typedef union {
    float f;
    byte binary[4];
} binaryFloat;

void sendStampedInts(unsigned long timestamp, byte count, int* ints) {
    // 1[2] 1[length] 4[timestamp] 2*count[data]
    if (Serial.availableForWrite() < 1 + 1 + 4 + 2*count)
        return;
    Serial.write(byte(2));
    Serial.write(count);
    binaryTimestamp ts;
    ts.timestamp = timestamp - start_time;
    Serial.write(ts.binary, 4);
    binaryInt b_int;
    for (unsigned int i=0; i<count; i++) {
        b_int.i = ints[i];
        Serial.write(b_int.binary, 2);
    }
}

void sendStampedFloats(unsigned long timestamp, byte count, float* floats) {
    // 1[3] 1[length] 4[timestamp] 4*count[data]
    if (Serial.availableForWrite() < 1 + 1 + 4 + 4*count)
        return;
    Serial.write(byte(3));
    Serial.write(count);
    binaryTimestamp ts;
    ts.timestamp = timestamp - start_time;
    Serial.write(ts.binary, 4);
    binaryFloat f;
    for (unsigned int i=0; i<count; i++) {
        f.f = floats[i];
        Serial.write(f.binary, 4);
    }
}

void resetTime() {
    start_time = millis();
}

#endif
