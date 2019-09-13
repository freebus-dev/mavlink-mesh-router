#include "Smoothing.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>

SMOOTHING::SMOOTHING(int count)
{
    numReadings = count;
}

void SMOOTHING::begin()
{
    for (int thisReading = 0; thisReading < numReadings; thisReading++) {
        readings[thisReading] = 0;
    }
}

uint64_t SMOOTHING::getAverage()
{
    return average;
}

uint64_t SMOOTHING::getLast()
{
    return last;
}

void SMOOTHING::add(uint64_t reading)
{
    last = reading;
    total = total - readings[readIndex];
    // read from the sensor:
    readings[readIndex] = reading;
    // add the reading to the total:
    total = total + readings[readIndex];
    // advance to the next position in the array:
    readIndex = readIndex + 1;

    // if we're at the end of the array...
    if (readIndex >= numReadings) {
        // ...wrap around to the beginning:
        readIndex = 0;
    }

    // calculate the average:
    average = uint32_t(total / numReadings);
}