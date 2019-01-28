// mysensor.c
#include "mysensor.h"
#include <string.h>
#include <stdlib.h>

float random_value(int min, int max)
{
    int scale = rand() / (int) RAND_MAX;
    return min + scale * (max - min);
}

struct Sensor read_temperature()
{
    struct Sensor temp;

    strncpy(temp.name, "Temperature", 15);
    temp.value = random_value(0, 35);

    return temp;
}

struct Sensor read_humidity()
{
    struct Sensor humdidty;

    strncpy(humdidty.name, "Humidity", 15);
    humdidty.value = random_value(40, 80);

    return humdidty;
}
