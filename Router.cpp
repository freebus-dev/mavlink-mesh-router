
#include "Router.h"
#include <stdlib.h>
Router::Router() {

}
bool Router::init(int sysid, int compid) {
	mavlink_system.sysid = sysid;
	mavlink_system.compid = compid;

	node->setID(sysid);

	mavlink_status_t* status = mavlink_get_channel_status(15);

	status->flags |= MAVLINK_STATUS_FLAG_OUT_MAVLINK1;

	return true;
}


void Router::addMavlinkTarget(Target* target) {
	if (!target->init()) {
		printf("!target.init()\n");
		return;
	}

	target->setChannel(mavlinkTargets.size());
	target->setSysID(mavlink_system.sysid);
	target->setCompID(mavlink_system.compid);

	mavlinkTargets.push_back(target);

	printf("Target added size:%d fd:%d\n", mavlinkTargets.size(), target->getFD());
}

void Router::addLinkTarget(TargetLinkPacket* target) {
	if (!target->init()) {
		printf("!target.init()\n");
		return;
	}

	target->setChannel(linkTargets.size());
	target->setSysID(mavlink_system.sysid);
	target->setCompID(mavlink_system.compid);

	linkTargets.push_back(target);

	printf("Target added size:%d fd:%d\n", linkTargets.size(), target->getFD());
}

System* Router::getSystem(uint8_t sysid, uint8_t compid)
{
	unsigned sys_comp_id = ((uint16_t)sysid << 8) | compid;
	for (size_t i = 0; i < systems.size(); i++) {

		if (compid == 0 || compid == -1) {
			if (systems[i]->getSysID() == sysid) {
				return systems[i];
			}
		}
		else {
			if (systems[i]->getSysID() == sysid && systems[i]->getCompID() == compid) {
				return systems[i];
			}
		}
	}
	return NULL;
}
bool Router::addSysCompID(uint8_t sysid, uint8_t compid, Target* target) {


	System* system = getSystem(sysid, compid);

	if (system != NULL) {
		return false;
	}

	system = new System;
	system->setSysID(sysid);
	system->setCompID(compid);

	if (target->isLocal()) {
		node->addEdge(sysid, compid, target->getChannel(), mavlink_system.sysid);
	}


	systems.push_back(system);

	return true;
}

void Router::getMsgTargets(const mavlink_message_t* msg, int& sysid, int& compid)
{

	const mavlink_msg_entry_t* msg_entry = mavlink_get_msg_entry(msg->msgid);
	if (msg_entry == nullptr) {
		return;
	}
	if (msg_entry->flags & MAV_MSG_ENTRY_FLAG_HAVE_TARGET_SYSTEM) {
		sysid = _MAV_RETURN_uint8_t(msg, msg_entry->target_system_ofs);
	}
	if (msg_entry->flags & MAV_MSG_ENTRY_FLAG_HAVE_TARGET_COMPONENT) {
		compid = _MAV_RETURN_uint8_t(msg, msg_entry->target_component_ofs);
	}
}
void Router::routeMavlinkPacket(Target* source, System* system, Target::MavlinkPacket* packet) {
	bool rc = (packet->msg.msgid == MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE);


	int sourceChannel = source->getChannel();

	uint8_t source_system = packet->msg.sysid;
	uint8_t source_component = packet->msg.compid;

	int target_system = -1;
	int target_component = -1;
	getMsgTargets(&packet->msg, target_system, target_component);

	bool broadcast_system = (target_system == 0 || target_system == -1);
	bool broadcast_component = (target_component == 0 || target_component == -1);
	bool match_system = broadcast_system || (target_system == mavlink_system.sysid);
	bool match_component =
		match_system && (broadcast_component || (target_component == mavlink_system.compid));
	bool process_locally = match_system && match_component;

	if (process_locally && !broadcast_system && !broadcast_component) {
		// nothing more to do - it can only be for us


		return;
	}
	else {
		if (target_system == -1)
			target_system = 0;
		if (target_component == -1)
			target_component = 0;



		if (broadcast_system) {
			//printf("routeMavlinkPacket: Broadcast system\n");



			for (size_t i = 0; i < mavlinkTargets.size(); i++) {
				if (mavlinkTargets[i] != source) {
					mavlinkTargets[i]->writePacket(packet);
				}
			}

			for (size_t i = 0; i < linkTargets.size(); i++) {
				if (linkTargets[i] != source && linkTargets[i]->getRemoteSysID() != packet->source) {
					linkTargets[i]->writeMavlinkPacket(packet);
				}
			}
			return;
		}

		System* targetSystem;

		if (broadcast_component) {
			for (size_t i = 0; i < node->edges.size(); i++) {
				if (node->edges[i]->sysid == target_system) {
					if (!rc)
						printf("routeMavlinkPacket: Broadcast component\n");
					writeMavlinkToRoute(source, packet, node->edges[i]->nodeid, node->edges[i]->channel);
				}
			}
		}
		else {
			for (size_t i = 0; i < node->edges.size(); i++) {
				if (node->edges[i]->sysid == target_system && node->edges[i]->compid == target_component) {
					if (!rc)
						printf("routeMavlinkPacket: target system\n");
					writeMavlinkToRoute(source, packet, node->edges[i]->nodeid, node->edges[i]->channel);
				}
			}
		}

	}
}
void Router::writeMavlinkToRoute(Target* source, Target::MavlinkPacket* packet, uint8_t nodeid, uint8_t channel) {



	if (nodeid == mavlink_system.sysid) {
		for (size_t j = 0; j < mavlinkTargets.size(); j++) {
			if (mavlinkTargets[j]->getChannel() == channel) {
				if (packet->msg.msgid != MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE)
					printf("writeMavlinkToRoute: local nodeid:%d chan:%d\n", nodeid, channel);
				mavlinkTargets[j]->writePacket(packet);
				break;
			}
		}
	}
	else {
		NodeStruct* route = node->getRoute(nodeid);
		if (route == NULL) {
			return;
		}

		for (size_t j = 0; j < linkTargets.size(); j++) {
			if (linkTargets[j]->getRemoteSysID() == route->id) {
				if (packet->msg.msgid != MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE)
					printf("writeMavlinkToRoute: remote nodeid:%d chan:%d\n", nodeid, channel);
				linkTargets[j]->writeMavlinkPacket(packet);
				break;
			}
		}
	}
}
void Router::processMavlinkRSSIPacket(Target* source, Target::MavlinkPacket* packet) {
	mavlink_radio_t radio;
	mavlink_msg_radio_decode(&packet->msg, &radio);

	//printf("MAVLINK_MSG_ID_RADIO id:%d seq:%d\n", radio.rssi, packet->msg.compid);
	source->setRSSI(radio.rssi, packet->msg.compid);

	/*for (size_t i = 0; i < node->neighbors.size(); i++) {
		if (node->neighbors[i]->id == source->getRemoteSysID()) {
			node->neighbors[i]->rssi = source->getRSSI();
			break;
		}
	}*/
}
void Router::processMavlinkPacket(Target* source, Target::MavlinkPacket* packet) {


	if (packet->msg.sysid == 0) {
		printf("sysid is 0 remote:%d id:%d seq:%u\n", source->getRemoteSysID(), packet->msg.sysid, packet->seq);

		return;
	}
	if (!source->isLocal() && packet->source == mavlink_system.sysid) {
		printf("our remote:%d id:%d seq:%u\n", source->getRemoteSysID(), packet->msg.sysid, packet->seq);

		return;
	}

	if (packet->msg.msgid == MAVLINK_MSG_ID_RADIO || packet->msg.msgid == MAVLINK_MSG_ID_RADIO_STATUS) {
		processMavlinkRSSIPacket(source, packet);
		return;
	}

	uint8_t source_system = packet->msg.sysid;
	uint8_t source_component = packet->msg.compid;
	bool heartBeat = (packet->msg.msgid == MAVLINK_MSG_ID_HEARTBEAT);

	uint64_t now_us = clock_gettime_us(CLOCK_MONOTONIC);

	int sourceChannel = source->getChannel();

	addSysCompID(source_system, source_component, source);

	System* system = getSystem(source_system, source_component);

	if (!system->acceptMavlinkMSG(packet)) {
		//printf("ignore dup msg remote:%d id:%d seq:%u\n", source->getRemoteSysID(), packet->msg.sysid, packet->seq);
		return;//ignore dup
	}
	printf("acceptMavlinkMSG remote:%d id:%d seq:%u\n", source->getRemoteSysID(), packet->msg.sysid, packet->seq);

	routeMavlinkPacket(source, system, packet);
}
void Router::task() {
	FD_ZERO(&fds);
	nfds = 0;
	for (size_t i = 0; i < mavlinkTargets.size(); i++) {
		int fd = mavlinkTargets[i]->getFD();
		FD_SET(fd, &fds);
		if (fd >= nfds)
			nfds = fd + 1;
	}
	for (size_t i = 0; i < linkTargets.size(); i++) {
		int fd = linkTargets[i]->getFD();
		FD_SET(fd, &fds);
		if (fd >= nfds)
			nfds = fd + 1;
	}

	timeout.tv_sec = 0;
	timeout.tv_usec = 20000;

	int res = select(nfds, &fds, NULL, NULL, &timeout);

	if (res < 0) {
		printf("select: %s\n", strerror(errno));
		/* this sleep is to avoid soaking the CPU if select starts
		   returning immediately for some reason */
		usleep(10000);
	}
	else if (res == 0) {
		/* timeout */
		// printf("select timeout\n");
	}
	else {
		for (size_t i = 0; i < mavlinkTargets.size(); i++) {
			int fd = mavlinkTargets[i]->getFD();
			int code;
			if ((code = FD_ISSET(fd, &fds))) {

				if (mavlinkTargets[i]->_read() > 0) {
					while (true) {
						Target::MavlinkPacket* packet = mavlinkTargets[i]->readMavlinkPacket();
						if (packet == NULL) {
							//printf("packet == NULL\n");
							break;
						}
						processMavlinkPacket(mavlinkTargets[i], packet);
					}
					//printf("FD_ISSET: fd:%d\n", fd);
				}
			}
		}
		for (size_t i = 0; i < linkTargets.size(); i++) {
			int fd = linkTargets[i]->getFD();
			int code;
			if ((code = FD_ISSET(fd, &fds))) {

				if (linkTargets[i]->_read() > 0) {
					if (linkTargets[i]->getRemoteSysID() != 0) {
						node->addLocalNode(linkTargets[i]->getRemoteSysID());
					}

					while (true) {
						Target::telem_format_t nextType = linkTargets[i]->getNextPacketType();

						if (nextType == Target::TF_MAVLINK) {
							Target::MavlinkPacket* packet = linkTargets[i]->readMavlinkPacket();
							if (packet == NULL) {
								//printf("packet == NULL\n");
								break;
							}
							processMavlinkPacket(linkTargets[i], packet);
						}
						else if (nextType == Target::TF_MANAGMENT_NODE) {
							NodeStruct* packet = linkTargets[i]->readNodeStructPacket();
							if (packet == NULL) {
								printf("packet == NULL\n");
								break;
							}
							if (linkTargets[i]->getRemoteSysID() != 0) {
								node->processNodeInfo(linkTargets[i]->getRemoteSysID(), packet);
							}
						}
						else if (nextType == Target::TF_MANAGMENT_EDGE) {
							NodeEdge* packet = linkTargets[i]->readNodeEdgePacket();
							if (packet == NULL) {
								printf("packet == NULL\n");
								break;
							}
							if (linkTargets[i]->getRemoteSysID() != 0) {
								node->processEdgeInfo(linkTargets[i]->getRemoteSysID(), packet);
							}
						}
						else if (nextType == Target::TF_MANAGMENT_PING_RESPONCE) {
							mavlink_ping_t* packet = linkTargets[i]->readPingPacket();
							if (packet == NULL) {
								//printf("packet == NULL\n");
								break;
							}
							std::cout << "id:";
							std::cout << linkTargets[i]->getRemoteSysID();
							std::cout << " ping time:";
							std::cout << (clock_gettime_us(CLOCK_MONOTONIC) - packet->time_usec) / 1000;
							std::cout << " seq:";
							std::cout << packet->seq;
							std::cout << "\n";
							//printf("ping id:%d time:%d seq:%u\n", linkTargets[i]->getRemoteSysID(), (clock_gettime_us(CLOCK_MONOTONIC) - packet->time_usec) / 1000, packet->seq);

						}
						else  if (nextType == Target::TF_MAX) {
							break;
						}

					}
					//printf("FD_ISSET: fd:%d\n", fd);
				}
			}
		}
	}

	uint64_t now_us = clock_gettime_us(CLOCK_MONOTONIC);
	if ((now_us - oneHz_interval_update_last) > oneHz_interval) {
		oneHz_interval_update_last = now_us;
		for (size_t i = 0; i < mavlinkTargets.size(); i++) {
			mavlinkTargets[i]->oneHzLoop(now_us);
		}
		for (size_t i = 0; i < linkTargets.size(); i++) {
			linkTargets[i]->oneHzLoop(now_us);
		}
		oneHzLoop(now_us);
	}
	now_us = clock_gettime_us(CLOCK_MONOTONIC);
	if ((now_us - tenHz_interval_update_last) > tenHz_interval) {
		tenHz_interval_update_last = now_us;
		for (size_t i = 0; i < mavlinkTargets.size(); i++) {
			mavlinkTargets[i]->tenHzLoop(now_us);
		}
		for (size_t i = 0; i < linkTargets.size(); i++) {
			linkTargets[i]->tenHzLoop(now_us);
		}
		tenHzLoop(now_us);
	}
	now_us = clock_gettime_us(CLOCK_MONOTONIC);
	if ((now_us - fiftyHz_interval_update_last) > fiftyHz_interval) {
		fiftyHz_interval_update_last = now_us;
		for (size_t i = 0; i < mavlinkTargets.size(); i++) {
			mavlinkTargets[i]->fiftyHzLoop(now_us);
		}
		for (size_t i = 0; i < linkTargets.size(); i++) {
			linkTargets[i]->fiftyHzLoop(now_us);
		}
		fiftyHzLoop(now_us);
	}
}


void Router::fiftyHzLoop(uint64_t now_us) {
	//
}
void Router::tenHzLoop(uint64_t now_us) {
	//
}
void Router::oneHzLoop(uint64_t now_us) {


	//node->oneHzLoop(now_us);

	for (size_t i = 0; i < linkTargets.size(); i++) {
		node->SendNodeInfo(linkTargets[i]);
	}

	if (arpTable) {
		scrap_ticks++;
		if (scrap_ticks > 5) {
			scrap_ticks = 0;
			scrapArpTable();
		}
	}

}

void Router::useArpTable(bool use, const char* iface)
{
	if (iface != NULL)
		memcpy(&arpTableIface, iface, 1024);

	arpTable = use;
	printf("Arp table scrap = %s %s\n", arpTable ? "True" : "False", arpTableIface);
};

int Router::findMacIP(const uint8_t* mac, const in_addr_t ip)
{
	for (int i = 0; i < mavlinkTargets.size(); i++)
	{

		sockaddr_in* sa = mavlinkTargets[i]->getHost();

		if (ip == sa->sin_addr.s_addr)
			return mavlinkTargets[i]->getChannel();
	}

	return -1;
}

void Router::scrapArpTable()
{
	int arp_entries;
	int hostapd_entries;
	int i;

	arp_entry_t arp_table[ARP_ENTRIES_MAX];

	arp_entries = ARP_ENTRIES_MAX;

	if (arp_table_get(arp_table, &arp_entries) != 0) {
		printf("ERROR reading arp table\n");
		return;
	}
	for (i = 0; i < arp_entries; i++) {

		if (strcmp(arpTableIface, arp_table[i].dev) != 0)
			continue;

		int s = findMacIP(arp_table[i].mac, arp_table[i].ip);
		/* is this arp entry one of our known telemetry destinations? */
		if (s == -1) {
			struct in_addr in;
			in.s_addr = arp_table[i].ip;
			printf("arp entry %d is not a known telem dest\n", i);
			if (arp_table[i].flags == 0) {
				printf("skipping telem dest at %s (flags=0)\n", inet_ntoa(in));
			}
			else {
				printf("adding telem dest @ %s:%d\n", inet_ntoa(in), 14550);
				addMavlinkTarget(new TargetMavlink(arp_table[i].mac, arp_table[i].ip, 14550, 0, 0));
				//addUDP(arp_table[i].mac, arp_table[i].ip, 14550, 0, TelemDestTarget::TF_MAVLINK);
			}
		}
		else {
			//printf("arp entry %d is telem dest %d\n", i, s);
		}
	}
}