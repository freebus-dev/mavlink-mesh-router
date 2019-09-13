#ifndef TELEM_ROUTER_H
#define TELEM_ROUTER_H

#include <climits>
#include <cstddef>
#include <stdint.h>
#include <iostream>
#include <vector>

#include "Node.h"
#include "Smoothing.h"
#include "Target.h"
#include "TargetMavlink.h"
#include "TargetLinkPacket.h"
#include "System.h"

#include "NodeStruct.h"

#include "c_library/ardupilotmega/mavlink.h"
#include "c_library/common/mavlink.h"
#include "arp_table.h"
#define ARP_ENTRIES_MAX 32


class Router
{
public:
	Router();

	bool init(int sysid, int compid);

	void addMavlinkTarget(Target* target);
	void processMavlinkPacket(Target* source, Target::MavlinkPacket* packet);
	void processMavlinkRSSIPacket(Target* source, Target::MavlinkPacket* packet);
	void routeMavlinkPacket(Target* source, System* system, Target::MavlinkPacket* packet);
	void writeMavlinkToRoute(Target* source, Target::MavlinkPacket* packet, uint8_t nodeid, uint8_t channel);
	void getMsgTargets(const mavlink_message_t* msg, int& sysid, int& compid);
	bool addSysCompID(uint8_t sysid, uint8_t compid, Target* target);
	System* getSystem(uint8_t sysid, uint8_t compid);

	void addLinkTarget(TargetLinkPacket* target);


	int findMacIP(const uint8_t* mac, const in_addr_t ip);
	void scrapArpTable();
	void useArpTable(bool use, const char* iface);

	void task();

	void fiftyHzLoop(uint64_t now_us);
	void tenHzLoop(uint64_t now_us);
	void oneHzLoop(uint64_t now_us);
private:
	mavlink_system_t mavlink_system;
	// Track heartbeat stats for each system ID.
	std::vector< System* > systems;
	std::vector< Target* > mavlinkTargets;
	std::vector< TargetLinkPacket* > linkTargets;

	Node* node = new Node;

	struct Ping {
		bool recived = true;
		uint32_t seq = 0;
		uint64_t timeSent = 0;
		uint64_t timeRecived = 0;
		uint8_t sysid = 0;
		uint8_t compid = 0;
		Target* target;
	};
	std::vector< Ping* > pings;


	char arpTableIface[1024];
	bool arpTable = false;
	int scrap_ticks = 0;


	int stats_ticks = 0;

	int boot_wait_ticks = 0;
	bool acceptRemotes = false;

	fd_set fds;
	int nfds = 0;
	struct timeval timeout;

	uint64_t tenHz_interval = 100000;
	uint64_t tenHz_interval_update_last = 0;

	uint64_t fiftyHz_interval = 20000;
	uint64_t fiftyHz_interval_update_last = 0;

	uint64_t oneHz_interval = 1000000;
	uint64_t oneHz_interval_update_last = 0;
};

#endif // TELEM_ROUTER_H