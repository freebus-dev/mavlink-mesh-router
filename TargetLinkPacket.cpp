

#include "TargetLinkPacket.h"

TargetLinkPacket::TargetLinkPacket(const uint8_t* mac, const in_addr_t dest_ip,
	const short dest_port_hbo, int tos, const int chan,
	const short bindPort)
	: Target(mac, dest_ip, dest_port_hbo, tos, chan, bindPort)
{
	link = Target::TL_LINKPACKET;
}
TargetLinkPacket::TargetLinkPacket(const uint8_t* path, int baud, bool flow, const int chan)
	: Target(path, baud, flow, chan)
{
	link = Target::TL_LINKPACKET;
}

bool TargetLinkPacket::init()
{
	if (!Target::init()) {
		return false;
	}

	return true;
}
Target::MavlinkPacket* TargetLinkPacket::readMavlinkPacket() {
	return mavlinkPacket;
}

NodeStruct* TargetLinkPacket::readNodeStructPacket() {
	return nodeStructPacket;
}
NodeEdge* TargetLinkPacket::readNodeEdgePacket() {
	return edgeStructPacket;
}
mavlink_ping_t* TargetLinkPacket::readPingPacket() {
	return pingResponce;
}

Target::telem_format_t TargetLinkPacket::getNextPacketType() {

	Target::telem_format_t nextType = Target::TF_MAX;

	if (nextPacketRSSI) {
		nextPacketRSSI = false;
		nextType = Target::TF_MAVLINK;
		return nextType;
	}

	while (!rxbuf.empty()) {

		uint8_t b = rxbuf.dequeue();
		if (!synced) {
			if (b == Slip::END) {
				slipRX.reset();
				synced = true;
				lastRxByteWasEsc = false;
				printf("slip - initial sync\n");
			}
			continue;
		}

		if (lastRxByteWasEsc) {
			if (b == Slip::ESC_END) {
				b = Slip::END;
			}
			else if (b == Slip::ESC_ESC) {
				b = Slip::ESC;
			}
			else {
				stats.read.errors++;
				printf("slip error - discarding data\n");
			}
			if (!slipRX.isFull()) {
				slipRX.append(b);
			}
			lastRxByteWasEsc = false;
			continue;
		}
		switch (b) {
		case Slip::END:
			if (!slipRX.isEmpty()) {
				const uint8_t* bytes = slipRX.data();
				//printf("slip new packet %d\n", bytes[0]);
				stats.read.packets++;
				switch (bytes[0]) {
				case Target::TF_MAVLINK: {

					//memcpy(mavlinkPacket, &bytes[1], slipRX.payloadLen());
					//nextType = Target::TF_MAVLINK;
					//slipRX.reset();
					//return nextType;

					mavlink_status_t status;

					MavlinkPacketBuffer buffer;
					memcpy(&buffer, &bytes[1], slipRX.payloadLen());

					for (size_t i = 0; i < buffer.length; i++) {

						uint8_t frame_check = mavlink_parse_char(getChannel(), buffer.payload[i], &mavlinkPacket->msg, &status);

						if (frame_check != MAVLINK_FRAMING_INCOMPLETE) {
							if (frame_check == MAVLINK_FRAMING_OK) {
								mavlinkPacket->seq = buffer.seq;
								mavlinkPacket->start = buffer.start;
								//printf("MAVLINK_FRAMING_OK %d %u\n", getChannel(), mavlinkPacket->seq);
								nextType = Target::TF_MAVLINK;
								slipRX.reset();
								return nextType;
							}
							else if (frame_check == MAVLINK_FRAMING_BAD_CRC) {
								printf("MAVLINK_FRAMING_BAD_CRC %d\n", getChannel());
							}
							else if (frame_check == MAVLINK_FRAMING_BAD_SIGNATURE) {
								printf("MAVLINK_FRAMING_BAD_SIGNATURE %d\n", getChannel());
							}
						}
					}
					break;
				}
				case Target::TF_MANAGMENT_NODE: {

					memcpy(nodeStructPacket, &bytes[1], slipRX.payloadLen());
					nextType = Target::TF_MANAGMENT_NODE;
					slipRX.reset();
					return nextType;

					break;
				}
				case Target::TF_MANAGMENT_EDGE: {

					memcpy(edgeStructPacket, &bytes[1], slipRX.payloadLen());
					nextType = Target::TF_MANAGMENT_EDGE;
					slipRX.reset();
					return nextType;

					break;
				}
				case Target::TF_MANAGMENT_PING_QUERY: {

					memcpy(pingQuery, &bytes[1], slipRX.payloadLen());


					//pingQuery->time_usec = clock_gettime_us(CLOCK_MONOTONIC);

					slipTX.reset();

					slipTX.delimitSlip();
					slipTX.appendSlip(Target::TF_MANAGMENT_PING_RESPONCE);
					slipTX.appendSlip(pingQuery, sizeof(mavlink_ping_t));
					slipTX.delimitSlip();
					stats.write.packets++;

					_write(slipTX.data(), slipTX.length());
					flush();

					slipRX.reset();

					break;
				}
				case Target::TF_MANAGMENT_PING_RESPONCE: {

					memcpy(pingResponce, &bytes[1], slipRX.payloadLen());

					slipRX.reset();
					nextType = Target::TF_MANAGMENT_PING_RESPONCE;
					return nextType;
				}
				default:

					break;
				}

				slipRX.reset();
			}
			break;

		case Slip::ESC:
			// we got an ESC character - we'll treat the next char specially
			lastRxByteWasEsc = true;
			break;

		default:
			if (!slipRX.isFull()) {
				slipRX.append(b);
			}
			break;
		}

	}
	return nextType;
}

int TargetLinkPacket::writeNodeStructPacket(NodeStruct* packet) {

	slipTX.reset();

	slipTX.delimitSlip();
	slipTX.appendSlip(Target::TF_MANAGMENT_NODE);
	slipTX.appendSlip(packet, sizeof(NodeStruct));
	slipTX.delimitSlip();
	stats.write.packets++;

	_write(slipTX.data(), slipTX.length());

}
int TargetLinkPacket::writeNodeEdgePacket(NodeEdge* packet) {

	slipTX.reset();

	slipTX.delimitSlip();
	slipTX.appendSlip(Target::TF_MANAGMENT_EDGE);
	slipTX.appendSlip(packet, sizeof(NodeEdge));
	slipTX.delimitSlip();
	stats.write.packets++;

	_write(slipTX.data(), slipTX.length());

}
int TargetLinkPacket::writeMavlinkPacket(Target::MavlinkPacket* packet) {


	MavlinkPacketBuffer buffer;


	buffer.length = mavlink_msg_to_send_buffer(buffer.payload, &packet->msg);
	if (packet->msg.magic == MAVLINK_STX_MAVLINK1) {
		buffer.payload[2] = packet->msg.seq;
	}
	else {
		buffer.payload[4] = packet->msg.seq;
	}

	buffer.seq = packet->seq;
	buffer.start = packet->start;

	slipTX.reset();

	slipTX.delimitSlip();
	slipTX.appendSlip(Target::TF_MAVLINK);
	slipTX.appendSlip(&buffer, MavlinkPacketBuffer::HDR_LEN + buffer.length);
	slipTX.delimitSlip();
	stats.write.packets++;


	_write(slipTX.data(), slipTX.length());

}
int TargetLinkPacket::_read()
{

	const int len = LinkPacket::MAX_PAYLOAD + LinkPacket::HDR_LEN;

	uint8_t tmp_buf[len];
	std::memset(tmp_buf, 0, sizeof tmp_buf);
	int res = readFD(tmp_buf, len);

	//printf("TargetLinkPacket::_read %d\n", res);
	if (res <= 0) {
		return res;
	}

	if (tmp_buf[0] == 253 && tmp_buf[5] == 51 && tmp_buf[7] == 166) {
		nextPacketRSSI = true;
		mavlink_status_t status;
		for (size_t i = 0; i < res; i++) {

			uint8_t frame_check = mavlink_parse_char(getChannel(), tmp_buf[i], &mavlinkPacket->msg, &status);

			if (frame_check != MAVLINK_FRAMING_INCOMPLETE) {
				if (frame_check == MAVLINK_FRAMING_OK) {
					mavlinkPacket->seq = nextPacketRSSISeq++;
					mavlinkPacket->start = nextPacketRSSIStart;
					return 1;
				}
				else if (frame_check == MAVLINK_FRAMING_BAD_CRC) {
					printf("MAVLINK_FRAMING_BAD_CRC %d\n", getChannel());
				}
				else if (frame_check == MAVLINK_FRAMING_BAD_SIGNATURE) {
					printf("MAVLINK_FRAMING_BAD_SIGNATURE %d\n", getChannel());
				}
			}
		}

		return 0;
	}
	stats.read.packets++;
	LinkPacket* packet = &packetRX;

	memcpy(packet, tmp_buf, res);

	unsigned dropped = packet->seq - packetRXNextSeq;
	packetRXNextSeq = packet->seq + 1;

	if (dropped > 0) {
		stats.read.dropped += dropped;
		printf("dropped %u \n", dropped);
	}


	if (res < LinkPacket::HDR_LEN) {
		stats.read.errors++;
		printf("[%u] received runt packet (%d bytes)\n", stats.read.errors, res);
		return 0;
	}
	else {


		for (size_t i = 0; i < packet->length; i++) {
			if (!rxbuf.full()) {
				rxbuf.enqueue(packet->payload[i]);
			}
		}

		remoteSysid = packet->sysid;
		remoteCompid = packet->compid;
		setRemRSSI(packet->rssi);



		return (int)packet->length;
	}
}
int TargetLinkPacket::_write(uint8_t* buf, int len)
{

	if (len > (packetTX.MAX_PAYLOAD - packetTXUsed)) {
		/* Force a send */
		printf("Message(%d) too large for aggregate buffer(%d), "
			"sending LinkPacket first\n", len, (packetTXUsed));
		writeLinkPacket();
	}

	memcpy(packetTXPayload, buf, len);
	packetTXPayload += len;
	packetTXUsed += len;

	return 0;
}
void TargetLinkPacket::fiftyHzLoop(uint64_t now_us) {
	if (packetTXUsed > 0) {
		writeLinkPacket();
	}
}
void TargetLinkPacket::oneHzLoop(uint64_t now_us) {

}
void TargetLinkPacket::tenHzLoop(uint64_t now_us) {

	pingQuery->seq = pingQuerySeq++;
	pingQuery->time_usec = clock_gettime_us(CLOCK_MONOTONIC);
	slipTX.reset();

	slipTX.delimitSlip();
	slipTX.appendSlip(Target::TF_MANAGMENT_PING_QUERY);
	slipTX.appendSlip(pingQuery, sizeof(mavlink_ping_t));
	slipTX.delimitSlip();
	stats.write.packets++;

	_write(slipTX.data(), slipTX.length());
	flush();
}

void TargetLinkPacket::flush() {
	writeLinkPacket();
}

void TargetLinkPacket::writeLinkPacket() {
	LinkPacket* packet;


	packet = &packetTX;
	packet->format = Target::TF_MAVLINK;

	packet->rssi = getRSSI();
	packet->sysid = sysid;
	packet->compid = compid;
	packet->seq = packetTXNextSeq++;

	packet->length = packetTXUsed;
	//printf("TargetLinkPacket::writeLinkPacket %d\n", LinkPacket::HDR_LEN + packet->length);


	stats.write.packets++;
	writeFD((uint8_t*)packet, LinkPacket::HDR_LEN + packet->length);


	packetTXUsed = 0;
	packetTXPayload = packet->payload;
	packetTXSentTime = clock_gettime_us(CLOCK_MONOTONIC);

}
