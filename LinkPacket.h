#ifndef LINK_PACKET_H
#define LINK_PACKET_H

#include <stdint.h>
#include "Target.h"

struct LinkPacket {
	static const int MAX_PAYLOAD = 1370; /* MTU minus the header */

	struct system_t {
		uint8_t sysid = 0;
		uint8_t compid = 0;

		uint8_t targetSysid = 0;
		uint8_t targetCompid = 0;


		uint8_t hopSysid = 0;
		uint8_t hopCompid = 0;

		uint8_t sourceSysid = 0;
		uint8_t sourceCompid = 0;

		uint8_t rssi = 0;
		bool local = false;
	};

	uint32_t seq;
	uint16_t length;
	uint8_t sysid;
	uint8_t compid;
	uint8_t rssi;
	Target::telem_format_t format;


	uint8_t payload[MAX_PAYLOAD];

	// everything but the payload
	static const int HDR_LEN = 16;
};

#endif // LINK_PACKET_H
