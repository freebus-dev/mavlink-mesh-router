

#include "TargetSlip.h"

TargetSlip::TargetSlip(const uint8_t* mac, const in_addr_t dest_ip,
	const short dest_port_hbo, int tos, const int chan,
	const short bindPort)
	: Target(mac, dest_ip, dest_port_hbo, tos, chan, bindPort)
{
	link = Target::TL_SLIP;
	rxbuf.init();
}
TargetSlip::TargetSlip(const uint8_t* path, int baud, bool flow, const int chan)
	: Target(path, baud, flow, chan)
{
	link = Target::TL_SLIP;
	rxbuf.init();
}


bool TargetSlip::init()
{
	if (!Target::init()) {
		return false;
	}

	return true;
}
Target::MavlinkPacket* TargetSlip::readMavlinkPacket() {

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


					for (size_t i = 0; i < slipRX.payloadLen(); i++) {

						uint8_t frame_check = mavlink_parse_char(getChannel(), bytes[i + 1], &packet->msg, &status);

						if (frame_check != MAVLINK_FRAMING_INCOMPLETE) {
							if (frame_check == MAVLINK_FRAMING_OK) {
								if (seq == 0) {
									packet->start = clock_gettime_us(CLOCK_MONOTONIC) / 1000;
								}
								packet->seq = seq++;
								packet->source = getSysID();

								slipRX.reset();
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
					break;
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
	return NULL;

}
int TargetSlip::writePacket(Target::MavlinkPacket* packet) {
	uint8_t sendBuf[MAVLINK_MAX_PACKET_LEN];
	int len = mavlink_msg_to_send_buffer(sendBuf, &packet->msg);

	return _write(sendBuf, len);
}
int TargetSlip::_read()
{
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
int TargetSlip::_write(uint8_t* buf, int len)
{

	SLIPPacket* packet = &slipTX;

	packet->reset();

	packet->delimitSlip();
	packet->appendSlip(Target::TF_MAVLINK);
	packet->appendSlip(buf, len);
	packet->delimitSlip();
	stats.write.packets++;
	return writeFD(packet->data(), packet->length());
}

void TargetSlip::fiftyHzLoop(uint64_t now_us)
{
}
void TargetSlip::tenHzLoop(uint64_t now_us)
{

}
void TargetSlip::oneHzLoop(uint64_t now_us)
{
}
void TargetSlip::flush() {

}
int TargetSlip::parseSlipPacket(uint8_t* in_buf, int length, uint8_t* out_buf)
{

	//printf("parseSlipPacket %d\n", length);

	int bufferIndex = 0;

	for (size_t i = 0; i < length; i++) {
		uint8_t b = in_buf[i];
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
				printf("slip error - discarding data %d\n", i);
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

				case Target::TF_MAVLINK:
					//printf( "Target::TF_MAVLINK\n");
					memcpy(out_buf, &bytes[1], slipRX.payloadLen());
					out_buf += slipRX.payloadLen();
					bufferIndex += slipRX.payloadLen();
					break;
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

	return bufferIndex;
}