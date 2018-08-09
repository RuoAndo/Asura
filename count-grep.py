import sys
import re
import linecache
from numpy import *

import socket, struct
import sys 
from binascii import hexlify

re_addr = re.compile("((?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?))")

def ip2int(addr):
        return struct.unpack("!I", socket.inet_aton(addr))[0]

def int2ip(addr):
        return socket.inet_ntoa(struct.pack("!I", addr))    

argvs = sys.argv
argc = len(argvs)

f = open(argvs[1])
line = f.readline() 

number = {}
percent = {}
size = {}
counter = 0

while line:
    tmp = line.split(",")
    number[counter] = tmp[1].strip()
    percent[counter] = tmp[4].strip()
    size[counter] = tmp[3].strip()
    counter = counter + 1
    line = f.readline()
f.close()
 
f = open(argvs[2])
line = f.readline() 
while line:
        tmp = line.split(",")

        counter = 0
        for x in number:        
                if int(tmp[0]) == int(x):
                        print int2ip(int(tmp[1])) + "," + int2ip(int(tmp[2])) + "," + size[counter] + "," + percent[counter]
                counter = counter + 1
                        
        line = f.readline() 

f.close()

