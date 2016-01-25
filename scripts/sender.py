#!/usr/bin/env python
import socket
import binascii
import struct
import time

#  Copyright David "Buzz" Bussenschutt  20 Jan 2016
#
#  This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#


port = 5554
ip = "192.168.0.1"

# assemble a byte array of arbitrary binary values for testing
MESSAGE = bytearray('')
for i in range ( 0 , 255):
#for i in range ( 0 , 510):
	MESSAGE.append(int(i%128))
CURRENT = MESSAGE

socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
socket.setblocking(False)

# send initial single-byte handshake, requesting  slot 1
socket.sendto(b'\1', (ip, port))

i = 0;
seconds_ticker = time.clock()
while ( 1 ):
	i = i + 1
	# first two bytes are SRC and TGT, then 100 bytes of DATA
	socket.sendto(b'\1' + b'\2' +MESSAGE, (ip, port))

	# every few seconds upp the packet size by 10 from 100-240
        if ( time.clock() > seconds_ticker + 1 ):
                seconds_ticker = time.clock()
		if len(CURRENT) < 510:
        	  CURRENT.append(int(42))
		print "packetlen now: " + str(len(CURRENT))
		

