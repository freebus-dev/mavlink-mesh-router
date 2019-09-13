#ifndef TELEM_TARGETSLIP_H
#define TELEM_TARGETSLIP_H

#include "Target.h"
#include "SLIP.h"
#include "RingBuffer.h"
#include <vector>

class TargetSlip : public Target
{
public:
	TargetSlip(const uint8_t *mac, const in_addr_t dest_ip, const short dest_port_hbo,
                     int tos, const int chan, const short bindPort = 0);
	TargetSlip(const uint8_t *path, int baud, bool flow, const int chan);

    bool init();

	Target::MavlinkPacket* readMavlinkPacket();
	int writePacket(Target::MavlinkPacket* packet);

    int _read();
    int _write(uint8_t *buf, int len);

    void fiftyHzLoop(uint64_t now_us);
    void tenHzLoop(uint64_t now_us);
    void oneHzLoop(uint64_t now_us);

	void flush();

	int parseSlipPacket(uint8_t *in_buf, int length, uint8_t *out_buf);

	mavlink_status_t status;
	Target::MavlinkPacket* packet = new Target::MavlinkPacket;
private:
	SLIPPacket slipTX, slipRX;
	RingBuffer<2048> rxbuf;

	uint32_t seq = 0;

	bool synced = false;
	bool lastRxByteWasEsc = false;

};

#endif // TELEM_TARGETSLIP_H