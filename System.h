#ifndef TELEM_SYSTEM_H
#define TELEM_SYSTEM_H


#include <climits>
#include <cstddef>
#include <stdint.h>
#include <vector>

#include "Target.h"

class System
{
public:
	System();

	bool acceptMavlinkMSG(Target::MavlinkPacket *packet);

	void setSysID(uint8_t id);
	uint8_t getSysID();
	void setCompID(uint8_t id);
	uint8_t getCompID();


	void setChannel(uint8_t chan);
	uint8_t getChannel();

	void setLocal();
	bool getLocal();


	void dumpStats();


private:
	bool localRoute = false;
	uint8_t sysid = 0;
	uint8_t compid = 0;
	uint32_t seq = 0; 
	uint64_t start = 0;

};


#endif // TELEM_SYSTEM_H