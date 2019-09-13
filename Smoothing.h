
#ifndef SMOOTHING_h
#define SMOOTHING_h
#include <stdint.h>

class SMOOTHING
{
public:
    SMOOTHING(int count = 10);
	uint64_t getAverage();
	uint64_t getLast();
    void add(uint64_t reading);
    void begin();

private:
    int numReadings = 10;

	uint64_t readings[10]; // the readings from the analog input
    int readIndex = 0; // the index of the current reading
	uint64_t total = 0;    // the running total
	uint64_t average = 0;
	uint64_t last = 0;
	uint64_t perSecond = 0;
};

#endif