Minimalistic Wifi Packet Forwarder using ESP8266 Wifi Transceiver v1.0

This is *not* a traditional router. This is a packet forwarder with
a simple binary protocol to send and receive packets between
connected devices using the ESP8266 as intermediary.

It can handle up to 254 connected devices, although the limit is
currently set to only 5 in the code ahead. You can change that limit
easily just by setting the amount on the MAX_SLOTS define.

The protocol is as follows:

On first sent packet, a device will identify itself with a single-byte
packet to the Forwarder telling which device index it is going to use.
Each connected device must have a unique index from 1-254
(index 0 = the Forwarder itself).

If a device wants to have its index automatically assigned, it should
then specify its index as 255 on the first packet.

The Forwarder will always answer the first packet with a new
single-byte packet back to the device containing its assigned index,
or a single-byte packet with 255 if no index was assigned (index
collision or not available slots).

After having been identified on the network, a device can start
sending and receiving packets.

To send packets to another device, a device sends then a packet with
the following format:

[SRC][TGT][PKT]

[SRC]: 1 byte - tells which device is contacting the Forwarder.
[TGT]: 1 byte - target device to receive the packet.
[PKT]: variable width - the packet itself.

The Forwarder will then send the very same packet (including the two
header bytes) to the target device identified by that TGT index.

There is also a simple LED feedback to know when it's in AP (slow blinking)
and Station mode (faster blinking). When it receives packets, the LED also
is kept on for a few seconds.

On my own tests, I'm getting about 10mbps throughput with this forwarder.

Written by Vander 'imerso' Nunes | imersiva.com
