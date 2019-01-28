//mysensor.h
#ifndef MYSENSOR_H
#define MYSENSOR_H


struct Sensor {
    char name[15];
    int value;
};

struct Sensor read_temperature();
struct Sensor read_humidity();


#endif