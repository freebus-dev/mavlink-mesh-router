#ifndef TELEM_TARGET_H
#define TELEM_TARGET_H

#include <vector>

#include <arpa/inet.h>
#include <cstddef>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>


#include "c_library/ardupilotmega/mavlink.h"
#include "c_library/common/mavlink.h"

//#include "LinkPacket.h"
#include "SlipRingBuffer.h"
#include "arp_table.h"
#include "util.h"
#include "Smoothing.h"

#define MAX_BUFFER_LEN 1466
#define MAX_PACKET_LEN 500

typedef SlipRingBuffer< MAX_BUFFER_LEN > SLIPPacket;

class Target
{
public:
	typedef enum { TT_UDP, TT_SERIAL } fd_type_t;

	typedef enum {
		TF_MAVLINK = 4,
		TF_MANAGMENT,
		TF_MANAGMENT_NODE,
		TF_MANAGMENT_EDGE,
		TF_MANAGMENT_PING_QUERY,
		TF_MANAGMENT_PING_RESPONCE,
		TF_VIDEO,
		TF_RC,
		TF_MAX // (last)
	} telem_format_t;

	typedef enum {
		TL_MAVLINK,
		TL_LINKPACKET,
		TL_SLIP,
		TL_MAX // (last)
	} telem_link_t;

	struct MavlinkPacket {
		uint8_t source = 0;
		uint32_t seq = 0;
		uint32_t start = 0;
		mavlink_message_t msg;
	};

	Target(const uint8_t* _mac, const in_addr_t dest_ip, const short dest_port_hbo, int tos,
		const int chan, const short bindPort = 0);
	Target(const uint8_t* _path, int baud, bool flow, const int chan);

	bool init();
	void dumpStats();

	virtual MavlinkPacket* readMavlinkPacket() = 0;
	virtual int writePacket(MavlinkPacket* packet) = 0;

	virtual int _read() = 0;

	int readFD(uint8_t* buf, int len);
	int writeFD(uint8_t* buf, int len);

	int bindSerial();
	int bindUDP();


	virtual void fiftyHzLoop(uint64_t now_us) = 0;
	virtual void tenHzLoop(uint64_t now_us) = 0;
	virtual void oneHzLoop(uint64_t now_us) = 0;


	virtual void flush() = 0;



	void setSysID(uint8_t id) {
		sysid = id;
	}
	uint8_t getSysID() {
		return sysid;
	}
	void setCompID(uint8_t id) {
		compid = id;
	}
	uint8_t getCompID() {
		return compid;
	}

	uint8_t getRemoteSysID() {
		return remoteSysid;
	}
	uint8_t getRemoteCompID() {
		return remoteCompid;
	}


	bool isLocal() {
		return link != TL_LINKPACKET;
	}

	void setChannel(int chan)
	{
		channel = chan;
	}
	int getChannel()
	{
		return channel;
	}
	void setFD(int f)
	{
		fd = f;
	}
	int getFD()
	{
		return fd;
	}
	void setHost(const in_addr_t dest_ip, const short dest_port_hbo)
	{
		sa.sin_addr.s_addr = dest_ip;
		sa.sin_port = htons(dest_port_hbo);
	}
	sockaddr_in* getHost()
	{
		return &sa;
	}


	void incReadMSG()
	{
		stats.read.msgs++;
	}
	void incWriteMSG()
	{
		stats.write.msgs++;
	}


	void setRSSI(uint8_t r, int index) {
		//printf("setRSSI %u:%d\n", r, index);
		rssi[index] = r;
	}
	uint8_t getRSSI() {
		return uint8_t((rssi[0] + rssi[1]) / 2);
	}
	void setRemRSSI(uint8_t r) {
		//printf("setRemRSSI %d %d\n", remoteSysid, r);
		remrssi = r;
	}
	uint8_t getRemRSSI() {
		return remrssi;
	}

	struct {
		struct {
			uint32_t packets = 0;
			uint32_t msgs = 0;
			uint64_t bytes = 0;
			uint32_t total = 0;
			uint32_t errors = 0;
			uint32_t dropped = 0;
			uint64_t blocked = 0;
		} read;
		struct {
			uint32_t packets = 0;
			uint32_t msgs = 0;
			uint64_t bytes = 0;
			uint32_t total = 0;
			uint32_t errors = 0;
			uint64_t blocked = 0;
		} write;
	} stats;
	uint64_t statsLastDump_us = 0;

	char name[1024];

	telem_link_t link;

	uint8_t sysid = 0;
	uint8_t compid = 0;


	uint8_t remoteSysid = 0;
	uint8_t remoteCompid = 0;

private:
	fd_type_t type;
	uint8_t rssi[2]{ 0 };
	uint8_t remrssi = 0;

	int fd;
	int channel;

	sockaddr_in sa;
	uint8_t mac[MAC_LEN];
	int tos;
	int bindPort;

	uint8_t path[1024];
	int serialBaud;
	bool useFlow;

};

#endif // TELEM_TARGET_H