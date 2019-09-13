
#ifndef TELEM_NODEINFO_H
#define TELEM_NODEINFO_H

#include <stdint.h>

struct NodeStruct {
	uint8_t id = 0;
	uint8_t hopCount = 0;
	bool active = false;
	uint32_t updated = 0;
	uint8_t mpr = 0;
	uint8_t rssi = 0;
	uint8_t remrssi = 0;
};
struct NodeEdge {
	uint8_t nodeid = 0;
	uint8_t sysid = 0;
	uint8_t compid = 0;
	uint8_t channel = 0;
};

#endif // TELEM_NODEINFO_H