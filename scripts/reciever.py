#!/usr/bin/env python
import socket
import binascii
import struct
import os
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

socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

socket.setblocking(False)

# send initial single-byte handshake, requesting  slot 2
socket.sendto(b'\2', (ip, port))

print "registered with server, and waiting for data"

bytecount = 0
seconds_ticker = time.clock()
data = b''
newdata = 0
_len = 0
while ( 1 ):
    try:
        data,address = socket.recvfrom(1000)
	newdata = 1
    except:
	newdata = 0
	pass
    if newdata == 1 :
      _len = len(data)
      #print "data recv :", data," len:",_len
      bytecount += _len - 2; #exclude src and addr bytes

    if ( time.clock() > seconds_ticker + 1 ):
		print "receiver bytes-per-sec: " +str(bytecount)+" total bytes-per-sec: "+str(bytecount*2)+" \tpacketlen: "+str(_len-2)
		seconds_ticker = time.clock()
		bytecount = 0

