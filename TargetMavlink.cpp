

#include "TargetMavlink.h"

TargetMavlink::TargetMavlink(const uint8_t* mac, const in_addr_t dest_ip,
	const short dest_port_hbo, int tos, const int chan,
	const short bindPort)
	: Target(mac, dest_ip, dest_port_hbo, tos, chan, bindPort)
{
	link = Target::TL_MAVLINK;
}
TargetMavlink::TargetMavlink(const uint8_t* path, int baud, bool flow, const int chan)
	: Target(path, baud, flow, chan)
{
	link = Target::TL_MAVLINK;
}


bool TargetMavlink::init()
{
	if (!Target::init()) {
		return false;
	}

	return true;
}

Target::MavlinkPacket* TargetMavlink::readMavlinkPacket() {



	while (!rxbuf.empty()) {

		uint8_t b = rxbuf.dequeue();

		uint8_t frame_check = mavlink_parse_char(getChannel(), b, &packet->msg, &status);

		if (frame_check != MAVLINK_FRAMING_INCOMPLETE) {
			if (frame_check == MAVLINK_FRAMING_OK) {

				if (start == 0) {
					start = clock_gettime_us(CLOCK_MONOTONIC) / 1000;
				}

				packet->seq = seq++;
				packet->start = start;
				packet->source = getSysID();

				return packet;
			}
			else if (frame_check == MAVLINK_FRAMING_BAD_CRC) {
				printf("MAVLINK_FRAMING_BAD_CRC %d\n", getChannel());
			}
			else if (frame_check == MAVLINK_FRAMING_BAD_SIGNATURE) {
				printf("MAVLINK_FRAMING_BAD_SIGNATURE %d\n", getChannel());
			}
		}
	}
	return NULL;
}
int TargetMavlink::writePacket(Target::MavlinkPacket* packet) {
	uint8_t sendBuf[MAVLINK_MAX_PACKET_LEN];
	int len = mavlink_msg_to_send_buffer(sendBuf, &packet->msg);
	return writeFD(sendBuf, len);
}

int TargetMavlink::_read()
{
	if (rxbuf.writeAvailable() < MAVLINK_MAX_PACKET_LEN) {
		return 0;
	}

	int res = -1;

	uint8_t tmp_buf[MAX_BUFFER_LEN];

	res = readFD(tmp_buf, MAX_BUFFER_LEN);
	if (res <= 0) {
		return res;
	}

	for (size_t i = 0; i < res; i++) {
		if (!rxbuf.full()) {
			rxbuf.enqueue(tmp_buf[i]);
		}
	}

	return res;
}
void TargetMavlink::fiftyHzLoop(uint64_t now_us)
{
}
void TargetMavlink::tenHzLoop(uint64_t now_us)
{

}
void TargetMavlink::oneHzLoop(uint64_t now_us)
{
}

void TargetMavlink::flush() {

}