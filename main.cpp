

#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>
#include <cstddef>
#include <string>

#include <netinet/in.h>
#include <unistd.h>

#include "Router.h"

#include "Target.h"
#include "TargetLinkPacket.h"
#include "TargetMavlink.h"
#include "TargetSlip.h"
//#include "TargetVideo.h"


#include "Node.h"

#include "c_library/ardupilotmega/mavlink.h"
#include "c_library/common/mavlink.h"
#include "INIReader/cpp/INIReader.h"


Router router;
//
//int main(void)
//{
//
//	printf("controller starting: built " __DATE__ " " __TIME__ "\n");
//
//	INIReader reader("/sololink.conf");
//
//	if (reader.ParseError() < 0) {
//		printf("can't parse ./sololink.conf\n");
//		return -1;
//	}
//
//	int sysid = reader.GetInteger("solo", "sysid", 16);
//	router.init(sysid, 1);
//
//	if (reader.GetBoolean("solo", "stm32Use", false)) {
//		std::string serialPortName = reader.Get("solo", "stm32Dev", "/dev/ttymxc1");
//		int serialBaud = reader.GetInteger("solo", "stm32Baud", 57600);
//		bool serialFlow = reader.GetBoolean("solo", "stm32Flow", false);
//		printf("serial port STM32: %s\n", serialPortName.c_str());
//		printf("serial baudrate: %d\n", serialBaud);
//		printf("serial hw flow: %s\n", serialFlow ? "true" : "false");
//		printf("tos: 0x%02x\n", 0);
//		const char* name = serialPortName.c_str();
//		TargetSlip* s1 = new TargetSlip((const uint8_t*)name, serialBaud, serialFlow, 0);
//		router.addTarget(s1);
//	}
//
//	if (reader.GetBoolean("solo", "telemUse", false)) {
//		std::string serialPortName = reader.Get("solo", "telemDev", "/dev/ttymxc1");
//		int serialBaud = reader.GetInteger("solo", "telemBaud", 57600);
//		bool serialFlow = reader.GetBoolean("solo", "telemFlow", false);
//		printf("serial port TELEM: %s\n", serialPortName.c_str());
//		printf("serial baudrate: %d\n", serialBaud);
//		printf("serial hw flow: %s\n", serialFlow ? "true" : "false");
//		printf("tos: 0x%02x\n", 0);
//		const char* name = serialPortName.c_str();
//		TargetMavlink* s2 = new TargetMavlink((const uint8_t*)name, serialBaud, serialFlow, 0);
//		router.addTarget(s2);
//	}
//
//	if (reader.GetBoolean("solo", "controller", false)) {
//		TargetLinkPacket* t10 = new TargetLinkPacket(NULL, inet_addr("10.1.1.10"), 13560, 0, 0, 13560);
//		router.addTarget(t10);
//		TargetLinkPacket* t11 = new TargetLinkPacket(NULL, inet_addr("10.1.1.11"), 13540, 0, 0, 13540);
//		router.addTarget(t11);
//	}
//
//	if (reader.GetBoolean("solo", "solo", false)) {
//		int soloPort = reader.GetInteger("solo", "soloPort", 14560);
//		TargetLinkPacket* t1 = new TargetLinkPacket(NULL, inet_addr("10.1.1.1"), soloPort, 0, 0, soloPort);
//		router.addTarget(t1);
//	}
//
//	TargetMavlink* l1 = new TargetMavlink(NULL, inet_addr("192.168.1.16"), 14550, 0, 0, 14550);
//	router.addTarget(l1);
//
//	int links = reader.GetInteger("solo", "links", 0);
//
//	if (links > 0) {
//		links = links * 2;
//		for (int i = 0; i < links; i++) {
//			int txPort = 14000 + i;
//			int rxPort = 14000 + ++i;
//			TargetLinkPacket* link =
//				new TargetLinkPacket(NULL, inet_addr("127.0.0.1"), txPort, 0, 0, rxPort);
//			router.addTarget(link);
//		}
//	}
//
//	bool arpTableUse = reader.GetBoolean("solo", "arpTableUse", false);
//	std::string arpTableIface = reader.Get("solo", "arpTableIface", "wlan0");
//	const char* arpTableIface_c = arpTableIface.c_str();
//
//	//router.useArpTable(arpTableUse, arpTableIface_c);
//
//	while (true) {
//		router.task();
//	}
//
//	return -1;
//}

//void printHelloPacket(Node::HelloPacket hello) {
	//for (size_t i = 0; i < hello.edgeCount; i++) {
	//	if (hello.nodeid == hello.edges[i].nodeid) {
			//printf("Node:%d has edge:%d:%d:%d:%d\n", hello.nodeid, hello.edges[i].nodeid, hello.edges[i].sysid, hello.edges[i].compid, hello.edges[i].channel);
	//	}
	//	else {
			//printf("Node:%d knows of edge:%d:%d:%d:%d\n", hello.nodeid, hello.edges[i].nodeid, hello.edges[i].sysid, hello.edges[i].compid, hello.edges[i].channel);
	//	}
	//}
	//for (size_t i = 0; i < hello.neighborsCount; i++) {

	//	printf("Node:%d has neighbor:%d hope count:%d replay:%d active:%s rssi:%d remrssi:%d\n", hello.nodeid, hello.neighbors[i].id, hello.neighbors[i].hopCount, hello.neighbors[i].mpr,
	////		hello.neighbors[i].active ? "true" : "false", hello.neighbors[i].rssi, hello.neighbors[i].remrssi);


	//}
	//printf("\n");
//}
int share(Node* node1, Node* node2) {
	//node1->processHelloPackt(node2->getHelloPacket());
	//node2->processHelloPackt(node1->getHelloPacket());
}
int maina(void) {

	Node* node1 = new Node;
	node1->setID(1);
	Node* node2 = new Node;
	node2->setID(2);


	node1->addLocalNode(2);
	node1->addHopNode(3, 2);
	node1->addHopNode(4, 2);


	node1->getRoute(3);




	return 0;
}
//
//int mainl(void) {
//
//
//	printf("%d\n", MAVLINK_MAX_PACKET_LEN);
//	printf("%u\n", clock_gettime_us(CLOCK_MONOTONIC) / 1000);
//	printf("%u\n", clock_gettime_us(CLOCK_MONOTONIC));
//
//
//	router.init(11, 1);
//	router2.init(12, 1);
//	router3.init(13, 1);
//	router4.init(14, 1);
//	router5.init(15, 1);
//
//	router.addLinkTarget(new TargetLinkPacket(NULL, inet_addr("127.0.0.1"), 5000, 0, 0, 5001));
//	router.addMavlinkTarget(new TargetMavlink((const uint8_t*)"/dev/ttyACM0", 921600, true, 0));
//
//	router2.addLinkTarget(new TargetLinkPacket(NULL, inet_addr("127.0.0.1"), 5001, 0, 0, 5000));
//	router2.addLinkTarget(new TargetLinkPacket(NULL, inet_addr("127.0.0.1"), 5002, 0, 0, 5003));
//
//
//	router3.addLinkTarget(new TargetLinkPacket(NULL, inet_addr("127.0.0.1"), 5003, 0, 0, 5002));
//	router3.addMavlinkTarget(new TargetMavlink(NULL, inet_addr("192.168.1.16"), 14550, 0, 0));
//
//
//
//	while (true) {
//		router.task();
//		router2.task();
//		router3.task();
//		//router4.task();
//		//router5.task();
//	}
//
//	return -1;
//}
int main(void)
{

	printf("controller starting: built " __DATE__ " " __TIME__ "\n");

	INIReader reader("/sololink.conf");

	if (reader.ParseError() < 0) {
		printf("can't parse ./sololink.conf\n");
		return -1;
	}

	int sysid = reader.GetInteger("solo", "sysid", 16);
	router.init(sysid, 1);
	if (reader.GetBoolean("solo", "controller", false)) {
		router.addLinkTarget(new TargetLinkPacket(NULL, inet_addr("10.1.1.10"), 13560, 0, 0, 13560));
		router.addLinkTarget(new TargetLinkPacket(NULL, inet_addr("10.1.1.11"), 13540, 0, 0, 13540));
	}

	if (reader.GetBoolean("solo", "solo", false)) {
		int soloPort = reader.GetInteger("solo", "soloPort", 14560);
		router.addLinkTarget(new TargetLinkPacket(NULL, inet_addr("10.1.1.1"), soloPort, 0, 0, soloPort));
	}
	//router.addMavlinkTarget(new TargetMavlink(NULL, inet_addr("192.168.1.16"), 14550, 0, 0, 14550));

	if (reader.GetBoolean("solo", "stm32Use", false)) {
		std::string serialPortName = reader.Get("solo", "stm32Dev", "/dev/ttymxc1");
		int serialBaud = reader.GetInteger("solo", "stm32Baud", 57600);
		bool serialFlow = reader.GetBoolean("solo", "stm32Flow", false);
		printf("serial port STM32: %s\n", serialPortName.c_str());
		printf("serial baudrate: %d\n", serialBaud);
		printf("serial hw flow: %s\n", serialFlow ? "true" : "false");
		printf("tos: 0x%02x\n", 0);
		const char* name = serialPortName.c_str();
		router.addMavlinkTarget(new TargetSlip((const uint8_t*)name, serialBaud, serialFlow, 0));
	}
	if (reader.GetBoolean("solo", "telemUse", false)) {
		std::string serialPortName = reader.Get("solo", "telemDev", "/dev/ttymxc1");
		int serialBaud = reader.GetInteger("solo", "telemBaud", 57600);
		bool serialFlow = reader.GetBoolean("solo", "telemFlow", false);
		printf("serial port TELEM: %s\n", serialPortName.c_str());
		printf("serial baudrate: %d\n", serialBaud);
		printf("serial hw flow: %s\n", serialFlow ? "true" : "false");
		printf("tos: 0x%02x\n", 0);
		const char* name = serialPortName.c_str();
		router.addMavlinkTarget(new TargetMavlink((const uint8_t*)name, serialBaud, serialFlow, 0));
	}
	int links = reader.GetInteger("solo", "links", 0);

	if (links > 0) {
		links = links * 2;
		for (int i = 0; i < links; i++) {
			int txPort = 14000 + i;
			int rxPort = 14000 + ++i;
			router.addLinkTarget(new TargetLinkPacket(NULL, inet_addr("127.0.0.1"), txPort, 0, 0, rxPort));
		}
	}

	bool arpTableUse = reader.GetBoolean("solo", "arpTableUse", false);
	std::string arpTableIface = reader.Get("solo", "arpTableIface", "wlan0");
	const char* arpTableIface_c = arpTableIface.c_str();

	router.useArpTable(arpTableUse, arpTableIface_c);

	while (true) {
		router.task();
	}

	return -1;
}
