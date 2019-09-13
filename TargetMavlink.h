#ifndef TELEM_TARGETMAVLINK_H
#define TELEM_TARGETMAVLINK_H

#include "Target.h"
#include "RingBuffer.h"
#include <vector>

class TargetMavlink : public Target
{
public:

    TargetMavlink(const uint8_t *mac, const in_addr_t dest_ip, const short dest_port_hbo,
                     int tos, const int chan, const short bindPort = 0);
    TargetMavlink(const uint8_t *path, int baud, bool flow, const int chan);

    bool init();

	Target::MavlinkPacket* readMavlinkPacket();
	int writePacket(Target::MavlinkPacket* packet);

    int _read();

    void fiftyHzLoop(uint64_t now_us);
    void tenHzLoop(uint64_t now_us);
    void oneHzLoop(uint64_t now_us);

	void flush();

	Target::MavlinkPacket* packet = new Target::MavlinkPacket;
	mavlink_status_t status;
private:
	RingBuffer<2048> rxbuf;
	uint32_t seq = 0;
	uint32_t start = 0;

};

#endif // TELEM_TARGETMAVLINK_H