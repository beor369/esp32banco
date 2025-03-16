#ifndef ULTRASONICSENSOR_H
#define ULTRASONICSENSOR_H

#include <Arduino.h>

class UltrasonicSensor {
public:
    UltrasonicSensor(int trigPin, int echoPin);
    void begin();
    float getDistance();
    void printDistance();

private:
    int trigPin;
    int echoPin;
};

extern UltrasonicSensor ultrasonic_sensor;

#endif // ULTRASONICSENSOR_H
