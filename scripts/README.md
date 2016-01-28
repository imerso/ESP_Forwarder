Minimalistic Wifi Packet Forwarder using ESP8266 Wifi Transceiver v1.0

this is the README for the 'scripts' folder.  Please also see the main README.

The sender.py and reciever.py work together, one is not very useful without the other ( or equivalent).

The "sender.py" connects to the esp8266 as a client ( with SRC ID 1 ) , then builds arbitrary packets of test data ( with TGT ID 2 ) , starting at length 255 bytes, and going upto the maximum 510bytes ( the largest the module will accept), and sends them to the module as fast as it ( and the PC you are running it on ) can.

The "reciever.py" connects to the esp8266 as a client ( with SRC ID 2 ), and listens for packets. when it receives them ( they come from "sender.py") , it reports on stdout the bytes-per-second thruput of the client and packet size.

You need to setup your ESPForwarder so it's UDP port is 5554 ( done via the esp8266's web interface) before these will work, and you also need whatever client ( laptop) they are run from to be connected to the AccessPoint that the esp8266 provides.

I was able to get this as my maximum WIFI throughput of approx 9.8 megabit per second (Mbps) . ( see script output below ). ( with 1/2 that bandwidth being received, and 1/2 that being sent )

receiver bytes-per-sec: 612000 total bytes-per-sec: 1224000 packetlen: 510
receiver bytes-per-sec: 610470 total bytes-per-sec: 1220940 packetlen: 510

Scripts authored by David "Buzz" Bussenschutt.  Copyright (c) 2016.  Scripts Released under the GNU GPL V3. 