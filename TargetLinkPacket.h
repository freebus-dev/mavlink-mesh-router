#ifndef TELEM_TARGETLINKPACKET_H
#define TELEM_TARGETLINKPACKET_H

#include "Target.h"
#include "LinkPacket.h"
#include "RingBuffer.h"
#include "NodeStruct.h"
#include <vector>

class TargetLinkPacket : public Target
{
public:
	TargetLinkPacket(const uint8_t* mac, const in_addr_t dest_ip, const short dest_port_hbo,
		int tos, const int chan, const short bindPort = 0);
	TargetLinkPacket(const uint8_t* path, int baud, bool flow, const int chan);


	struct MavlinkPacketBuffer {
		static const int MAX_PAYLOAD = 280; /* MTU minus the header */

		uint32_t seq = 0;
		uint32_t start = 0;
		uint16_t length = 0;

		uint8_t payload[MAX_PAYLOAD];

		// everything but the payload
		static const int HDR_LEN = 12;
	};
	bool init();

	int _read();
	int _write(uint8_t* buf, int len);

	MavlinkPacket* readMavlinkPacket();
	int writeMavlinkPacket(Target::MavlinkPacket* packet);

	int writeNodeStructPacket(NodeStruct* packet);
	NodeStruct* readNodeStructPacket();

	int writeNodeEdgePacket(NodeEdge* packet);
	NodeEdge* readNodeEdgePacket();


	mavlink_ping_t* readPingPacket();

	int writePacket(MavlinkPacket* packet) {

	};

	Target::telem_format_t getNextPacketType();


	void writeLinkPacket();

	void fiftyHzLoop(uint64_t now_us);
	void tenHzLoop(uint64_t now_us);
	void oneHzLoop(uint64_t now_us);

	void flush();

private:
	Target::MavlinkPacket* mavlinkPacket = new Target::MavlinkPacket;
	NodeStruct* nodeStructPacket = new NodeStruct;
	NodeEdge* edgeStructPacket = new NodeEdge;

	mavlink_ping_t* pingQuery = new mavlink_ping_t;
	mavlink_ping_t* pingResponce = new mavlink_ping_t;

	int pingQueryTicks = 0;
	uint32_t pingQuerySeq = 0;

	SLIPPacket slipTX, slipRX;

	RingBuffer<4096> rxbuf;

	bool nextPacketRSSI = false;
	uint32_t nextPacketRSSISeq = 0;
	uint32_t nextPacketRSSIStart = 1;

	bool synced = false;
	bool lastRxByteWasEsc = false;

	LinkPacket packetTX;
	uint8_t* packetTXPayload = packetTX.payload;

	uint32_t packetTXNextSeq = 0;
	int packetTXUsed = 0;
	uint64_t packetTXSentTime = 0;

	LinkPacket packetRX;
	uint32_t packetRXNextSeq = 0;
};

#endif // TELEM_TARGETLINKPACKET_H