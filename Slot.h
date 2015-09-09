#ifndef _SLOT_H_
#define _SLOT_H_

#include <ESP8266WiFi.h>

class Slot
{
	public:

		Slot() { used = false; }
		~Slot() {}

		void Use(IPAddress ip, int port) { this->ip = IPAddress(ip[0], ip[1], ip[2], ip[3]); this->port = port; this->used = true; }

		IPAddress ip;
		int port;
		bool used;
};

#endif

