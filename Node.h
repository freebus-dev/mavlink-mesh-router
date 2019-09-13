

#ifndef TELEM_NODE_H
#define TELEM_NODE_H
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>
#include <cstddef>
#include <string>
#include <climits>
#include <cstddef>
#include <stdint.h>
#include <stdlib.h>
#include <vector>



#include "util.h"
#include "TargetLinkPacket.h"
#include "NodeStruct.h"


class Node
{
public:
	Node();

	void setID(uint8_t _id);
	uint8_t getID();
	uint8_t getRSSI() {
		return rssi;
	};
	uint8_t getRemRSSI() {
		return remrssi;
	};
	void setRSSI(uint8_t reading) {
		rssi = reading;
	}
	void setRemRSSI(uint8_t reading) {
		remrssi = reading;
	}

	bool addEdge(uint8_t sysid, uint8_t compid, uint8_t channel, uint8_t nodeid = 0);
	NodeEdge* getEdge(uint8_t sysid, uint8_t compid, uint8_t channel = 0);

	void addLocalNode(uint8_t nodeid);
	void removeLocalNode(uint8_t nodeid);

	void addHopNode(uint8_t nodeid, uint8_t mpr, bool active = true);
	void removeHopNode(uint8_t nodeid, uint8_t mpr);
	void removeAllHopNode(uint8_t mpr);

	void SendNodeInfo(TargetLinkPacket* source);

	void processNodePing(uint8_t nodeid, mavlink_ping_t* pingResponce);

	void processNodeInfo(uint8_t nodeid, NodeStruct* info); 
	void processEdgeInfo(uint8_t nodeid, NodeEdge* info);

	NodeStruct* getRoute(uint8_t nodeid);

	std::vector< NodeEdge* > edges;
	std::vector< NodeStruct* > nodes;
private:

	uint8_t id = 0;
	uint8_t rssi = 0;
	uint8_t remrssi = 0;

};


#endif // TELEM_NODE_H