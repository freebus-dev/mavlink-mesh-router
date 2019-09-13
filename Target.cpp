
#include <math.h>
#include "Target.h"

Target::Target(const uint8_t *_mac, const in_addr_t dest_ip, const short dest_port_hbo, int _tos,
	const int chan, const short _bindPort)
{
	if (_mac != NULL)
		memcpy(&mac, _mac, MAC_LEN);
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = dest_ip;
	sa.sin_port = htons(dest_port_hbo);
	printf("dest_port_hbo %d\n", dest_port_hbo);
	tos = _tos;
	bindPort = _bindPort;
	type = TT_UDP;
	sprintf(name, "%s:%d", inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));
}
Target::Target(const uint8_t *_path, int baud, bool flow, const int chan)
{
	if (_path != NULL)
		memcpy(&path, _path, 1024);
	useFlow = flow;
	serialBaud = baud;
	type = TT_SERIAL;

	sprintf(name, "%s:%d", path, serialBaud);
}

bool Target::init()
{

	int res = -1;

	if (type == TT_UDP) {
		res = bindUDP();
	}
	else if (type == TT_SERIAL) {
		res = bindSerial();

		uint8_t tmp_buf[200];
		int count = 0;
		while (readFD(tmp_buf, 200) != 0 && count++ < 50) {
			usleep(10000);
		}

	}
	else {
		printf("Target::init() fail: else\n");
		return false;
	}
	if (res <= 0) {
		printf("Target::init() fail: %d\n", res);
		return false;
	}
	return true;
}

void Target::dumpStats()
{
	uint64_t now_us = clock_gettime_us(CLOCK_MONOTONIC);

	uint64_t delta_us = now_us - statsLastDump_us;

	statsLastDump_us = now_us;

	double readPct, writePct;
	if (type == TT_SERIAL) {

		printf("TT_SERIAL  ;%s:%d\n", path, serialBaud);

		readPct = (stats.read.bytes * 1e9) / ((double)(delta_us * serialBaud));
		writePct = (stats.read.bytes * 1e9) / ((double)(delta_us * serialBaud));
	}
	else if (type == TT_UDP) {
		printf("TT_UDP     ;%s:%d\n", inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));
		readPct = (stats.read.bytes * 1e9) / ((double)(delta_us * 1000000));
		writePct = (stats.write.bytes * 1e9) / ((double)(delta_us * 1000000));
	}
	else {
		printf("TT_NULL    ;NULL:NULL\n");
		readPct = 0;
		writePct = 0;
	}

	printf("   read : %d m/s, %d p/s, %d b/s, %d%%, %dms err:%d;\n",
		int(round(stats.read.msgs * 1e6 / delta_us)), int(round(stats.read.packets * 1e6 / delta_us)),
		int(stats.read.bytes * 1e6 / delta_us), int(readPct), int(stats.read.blocked / 1000),
		stats.read.errors);
	printf("   write: %d m/s, %d p/s, %d b/s, %d%%, %dms err:%d;\n",
		int(round(stats.write.msgs * 1e6 / delta_us)), int(round(stats.write.packets * 1e6 / delta_us)),
		int(stats.write.bytes * 1e6 / delta_us), int(writePct), int(stats.write.blocked / 1000),
		stats.write.errors);

	/* printf( "dumpStats %d %d %d %d %d", stats.read.packets, stats.read.bytes,
	   stats.read.msgs, stats.read.blocked, delta_us);*/

	stats = {};
}
int Target::readFD(uint8_t *buf, int len)
{

	int res = -1;

	uint64_t t1_us = clock_gettime_us(CLOCK_MONOTONIC);
	if (type == TT_UDP) {
		struct sockaddr_in sa;
		socklen_t sa_len = sizeof(sa);
		res = recvfrom(fd, buf, len, 0, (struct sockaddr *)&sa, &sa_len);
	}
	else if (type == TT_SERIAL) {
		res = read(fd, buf, len);
	}
	else {
		printf("type unknown for type %d\n", type);
	}

	uint64_t t2_us = clock_gettime_us(CLOCK_MONOTONIC);

	if (res > 0) {
		stats.read.bytes += res;
	}
	stats.read.blocked += t2_us - t1_us;

	if ((t2_us - t1_us) > 10000)
		printf("readFD: blocked %u usec\n", (unsigned)(t2_us - t1_us));

	return res;
}
int Target::writeFD(uint8_t *buf, int len)
{
	int res = -1;
	uint64_t t1_us = clock_gettime_us(CLOCK_MONOTONIC);
	//printf("Target::writeFD: %d\n", len);
	if (type == TT_SERIAL) {
		int txBufUsage = 0;
		ioctl(fd, TIOCOUTQ, &txBufUsage);

		if ((4000 - txBufUsage) < len) {
			// Congestion; serial port tx buffer is filling up
			//printf("serial port tx full; waiting for room %d\n", txBufUsage);
			const unsigned serial_timeout_us = 1000000; // 1 sec
			uint64_t start_us = clock_gettime_us(CLOCK_MONOTONIC);
			while (true) {
				uint64_t poll_us = clock_gettime_us(CLOCK_MONOTONIC);
				ioctl(fd, TIOCOUTQ, &txBufUsage);
				if ((4000 - txBufUsage) >= len)
					break;
				unsigned wait_us = poll_us - start_us;
				if (wait_us > serial_timeout_us) {
					printf("serial port tx stuck; exiting\n");
					exit(1);
				}
				else {
					usleep(5000); // 5 msec
				}
			}
			//printf("serial port tx was full; now resuming %d\n", txBufUsage);
		}

		if ((res = write(fd, buf, len)) != len) {
			stats.write.errors++;
			printf("pkt: could not write to serial port\n");
		}
	}
	else if (type == TT_UDP) {
		if ((res = sendto(fd, buf, len, MSG_DONTWAIT, (struct sockaddr *)&(sa), sizeof(sa))) < 0) {
			stats.write.errors++;
			//printf("sendto(%s:%d:%d:%d): %s\n", inet_ntoa(sa.sin_addr), ntohs(sa.sin_port), res,
			//	len, strerror(errno));
		}
		else {

			// printf("Sent to socket(%d) %s:%d\n", fd, inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));
		}
	}
	else {
		printf("Type unknown socket(%d) %s:%d\n", fd, inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));
	}

	uint64_t t2_us = clock_gettime_us(CLOCK_MONOTONIC);

	stats.write.bytes += len;
	stats.write.blocked += t2_us - t1_us;

	if ((t2_us - t1_us) > 10000)
		printf("writeFD: blocked %u usec\n", (unsigned)(t2_us - t1_us));
	return res;
}

int Target::bindSerial()
{

	struct termios options;

	fd = open((const char *)path, O_RDWR | O_NOCTTY | O_NONBLOCK);

	if (fd < 0) {
		printf("unable to open serial port %s\n", path);
		return 0;
	}

	// Configure port for 8N1 transmission
	tcgetattr(fd, &options); // Gets the current options for the port
	// Set the output baud rate
	switch (serialBaud) {
	case 1200:
		cfsetspeed(&options, B1200);
		break;
	case 2400:
		cfsetspeed(&options, B2400);
		break;
	case 4800:
		cfsetspeed(&options, B4800);
		break;
	case 9600:
		cfsetspeed(&options, B9600);
		break;
	case 19200:
		cfsetspeed(&options, B19200);
		break;
	case 38400:
		cfsetspeed(&options, B38400);
		break;
	case 57600:
		cfsetspeed(&options, B57600);
		break;
	case 115200:
		cfsetspeed(&options, B115200);
		break;
	case 500000:
		cfsetspeed(&options, B500000);
		break;
	case 921600:
		cfsetspeed(&options, B921600);
		break;
	case 1500000:
		cfsetspeed(&options, B1500000);
		break;
	default:
		printf("unsupported baud rate %d\n", serialBaud);
		return 0;
	}
	options.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
	options.c_oflag &= ~(OCRNL | ONLCR | ONLRET | ONOCR | OFILL | OPOST);
	options.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
	options.c_cflag &= ~(CSIZE | PARENB);
	options.c_cflag |= (CS8 | CLOCAL);

	if (useFlow)
		options.c_cflag |= CRTSCTS; // hardware flow control
	else
		options.c_cflag &= ~(CRTSCTS); // no hardware flow control

	// At 115k (87 us per char), reading 1 char at a time results in increased
	// CPU usage, since we actually can keep up with getting a small number of
	// characters per loop. At 921k (11 us per char), we get more characters
	// each time through the loop, so there is less advantage to setting VMIN
	// to more than 1.
	//
	//          CPU Usage at
	// VMIN     115k    921k
	//    1     7.0%    1.8%
	//   10     2.7%    1.6%
	//  100     1.2%    1.2%
	//
	// The problem with asking for more than 1 character per read is that each
	// message will usually not be received until some bytes in the following
	// message are available. That is often not a problem, but there are
	// sometimes gaps of several 10s of milliseconds in the telemetry stream,
	// and it is preferable to process messages as soon as they are available.
	if (serialBaud <= 115200)
		options.c_cc[VMIN] = 10;
	else
		options.c_cc[VMIN] = 1;
	options.c_cc[VTIME] = 0;

	tcsetattr(fd, TCSANOW, &options); // Set the new options for the port "NOW"

	printf("opened serial port %s\n", path);

	return fd;
}

int Target::bindUDP()
{

	struct sockaddr_in bind_sa;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket\n");
		return -1;
	}

	memset(&bind_sa, 0, sizeof(bind_sa));
	bind_sa.sin_family = AF_INET;
	bind_sa.sin_addr.s_addr = htonl(INADDR_ANY);
	bind_sa.sin_port = htons(bindPort);
	if (bind(fd, (struct sockaddr *)&bind_sa, sizeof(bind_sa)) < 0) {
		perror("bind\n");
		close(fd);
		return -1;
	}

	if (setsockopt(fd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos)) != 0) {
		perror("setsockopt\n");
		close(fd);
		return -1;
	}
	printf("Bound to socket(%d) %s:%d\n", fd, inet_ntoa(bind_sa.sin_addr), bindPort);

	return fd;
}
