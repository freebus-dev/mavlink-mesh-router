#include "System.h"
#include "util.h"
#include <string.h>
#include <stdio.h>


System::System()
{
	sysid = 0;
	compid = 0;
	seq = 0;
	start = 0;
}

bool System::acceptMavlinkMSG(Target::MavlinkPacket* packet) {


	if (start == 0) {
		seq = packet->seq;
		start = packet->start;
		return true;
	}

	if (packet->seq > seq) {
		if (packet->seq - seq > 1) {
			printf("MSG dropped %d\n", packet->seq - seq);
		}
		seq = packet->seq;

		return true;
	}
	else if (start != packet->start) {

		seq = packet->seq;
		start = packet->start;
		return true;
	}
	return false;
}

void System::setSysID(uint8_t id) {
	sysid = id;
}
uint8_t System::getSysID() {
	return sysid;
}
void System::setCompID(uint8_t id) {
	compid = id;
}
uint8_t System::getCompID() {
	return compid;
}

void System::setLocal() {
	localRoute = true;
}
bool System::getLocal() {
	return localRoute;
}

void System::dumpStats() {

}