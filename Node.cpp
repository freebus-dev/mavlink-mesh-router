#include "Node.h"


Node::Node() {

}
void Node::setID(uint8_t _id) {
	id = _id;

}
uint8_t Node::getID() {
	return id;
}

bool Node::addEdge(uint8_t sysid, uint8_t compid, uint8_t channel, uint8_t nodeid) {
	for (size_t i = 0; i < edges.size(); i++) {
		if (edges[i]->sysid == sysid &&
			edges[i]->compid == compid &&
			edges[i]->channel == channel &&
			edges[i]->nodeid == (edges[i]->nodeid == 0 ? getID() : nodeid)) {
			return false;
		}
	}

	NodeEdge* edge = new  NodeEdge;

	edge->sysid = sysid;
	edge->compid = compid;
	edge->channel = channel;

	if (nodeid == 0) {
		edge->nodeid = getID();
	}
	else {
		edge->nodeid = nodeid;
	}

	edges.push_back(edge);
	return true;
}


NodeEdge* Node::getEdge(uint8_t sysid, uint8_t compid, uint8_t channel) {
	for (size_t i = 0; i < edges.size(); i++) {
		if (edges[i]->sysid == sysid && edges[i]->compid == compid) {

			if (edges[i]->channel == 0) {
				return edges[i];
			}
			else if (edges[i]->channel == channel) {
				return edges[i];
			}
		}
	}
	return NULL;
}


void Node::addLocalNode(uint8_t nodeid) {

	if (nodeid == getID()) {
		return;
	}

	uint32_t now = clock_gettime_us(CLOCK_MONOTONIC) / 1000;
	for (size_t i = 0; i < nodes.size(); i++) {
		if (nodes[i]->id == nodeid && nodes[i]->mpr == getID()) {
			if (!nodes[i]->active) {
				printf("Node(%d) back online %dms\n", nodes[i]->id, now - nodes[i]->updated);
			}
			nodes[i]->active = true;
			nodes[i]->updated = clock_gettime_us(CLOCK_MONOTONIC) / 1000;
			return;
		}
	}

	NodeStruct* node = new NodeStruct();
	node->id = nodeid;
	node->hopCount = 0;
	node->active = true;
	node->updated = clock_gettime_us(CLOCK_MONOTONIC) / 1000;
	node->mpr = getID();
	nodes.push_back(node);

}

void Node::removeLocalNode(uint8_t nodeid) {

	if (nodeid == getID()) {
		return;
	}

	for (size_t i = 0; i < nodes.size(); i++) {
		if (nodes[i]->id == nodeid && nodes[i]->mpr == getID()) {
			nodes.erase(nodes.begin() + i);
			return;
		}
	}

}

void Node::addHopNode(uint8_t nodeid, uint8_t mpr, bool active) {

	if (nodeid == getID() || mpr == getID()) {
		return;
	}

	for (size_t i = 0; i < nodes.size(); i++) {
		if (nodes[i]->id == nodeid && nodes[i]->mpr == mpr) {
			nodes[i]->active = true;
			nodes[i]->updated = clock_gettime_us(CLOCK_MONOTONIC) / 1000;
			return;
		}
	}

	NodeStruct* node = new NodeStruct();
	node->id = nodeid;
	node->hopCount = 0;
	node->active = active;
	node->updated = clock_gettime_us(CLOCK_MONOTONIC) / 1000;
	node->mpr = mpr;
	nodes.push_back(node);

}


void Node::removeAllHopNode(uint8_t mpr) {
	if (mpr == getID()) {
		return;
	}

	for (size_t i = 0; i < nodes.size(); i++) {
		if (nodes[i]->mpr == mpr) {
			nodes.erase(nodes.begin() + i);
			i--;
		}
	}
}


void Node::removeHopNode(uint8_t nodeid, uint8_t mpr) {

	if (nodeid == getID() || mpr == getID()) {
		return;
	}

	for (size_t i = 0; i < nodes.size(); i++) {
		if (nodes[i]->id == nodeid && nodes[i]->mpr == mpr) {
			nodes.erase(nodes.begin() + i);
			return;
		}
	}
}

void Node::SendNodeInfo(TargetLinkPacket* source) {

	uint8_t nodeid = source->getRemoteSysID();

	for (size_t i = 0; i < nodes.size(); i++) {
		source->writeNodeStructPacket(nodes[i]);
	}

	for (size_t i = 0; i < edges.size(); i++) {
		source->writeNodeEdgePacket(edges[i]);
	}

}

void Node::processNodePing(uint8_t nodeid, mavlink_ping_t* pingResponce) {


}

void Node::processNodeInfo(uint8_t nodeid, NodeStruct* info) {

	addHopNode(info->id, nodeid, info->active);

}

void Node::processEdgeInfo(uint8_t nodeid, NodeEdge* info) {
	addEdge(info->sysid, info->compid, info->channel, info->nodeid);
}


NodeStruct* Node::getRoute(uint8_t nodeid) {


	NodeStruct* direct = NULL;
	NodeStruct* best = NULL;

	uint32_t now = clock_gettime_us(CLOCK_MONOTONIC) / 1000;

	for (size_t i = 0; i < nodes.size(); i++) {
		if (nodes[i]->id == nodeid) {

			if (nodes[i]->active && nodes[i]->mpr == getID()) {

				if (now - nodes[i]->updated > 500) {
					printf("Taking node(%d) offline %dms\n", nodes[i]->id, now - nodes[i]->updated);
					nodes[i]->active = false;
				}
				else {
					direct = nodes[i];
				}

			}
			else {

				best = nodes[i];
			}
		}
	}

	if (direct != NULL) {
		return direct;
	}
	else if (best != NULL) {

		for (size_t i = 0; i < nodes.size(); i++) {
			//printf("lookup node(%d)  %d %d\n", nodes[i]->id, nodes[i]->mpr, nodeid);
			if (nodes[i]->id == best->mpr && nodes[i]->mpr == getID()) {

				if (now - nodes[i]->updated < 500) {
					return nodes[i];
				}
				else {
					printf("Taking node(%d) offline %dms\n", nodes[i]->id, now - nodes[i]->updated);
				}
				//printf("best node(%d)  %d %d\n", nodes[i]->id, nodes[i]->mpr, nodeid);
				
			}
		}
		printf("best node(%d)  %d %d\n", best->id, best->mpr, nodeid);
		return NULL;
	}
	else {
		return NULL;
	}

}