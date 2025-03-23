#ifndef ULTRASONICSENSOR_H
#define ULTRASONICSENSOR_H

#include <Arduino.h>

class UltrasonicSensor {
public:
    UltrasonicSensor(int trigPin, int echoPin);
    void begin();
    float getDistance();
    void printDistance();
    float get_distance_fashion(int ciclos, bool is_fashion);
    float distaceFashion;

private:
    int trigPin;
    int echoPin;

};

extern UltrasonicSensor ultrasonic_sensor;

#endif // ULTRASONICSENSOR_H
