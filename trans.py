# python 1.py tools/libxl/libxl.c libxl 372

import socket, struct
import sys 
from binascii import hexlify

import re
re_addr = re.compile("((?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?))")

def ip2int(addr):
        return struct.unpack("!I", socket.inet_aton(addr))[0]


def int2ip(addr):
        return socket.inet_ntoa(struct.pack("!I", addr))    

argvs = sys.argv  
argc = len(argvs) 

f = open(argvs[1])

line = f.readline() 

while line:

        tmp = line.split(",")

        m = re_addr.search(tmp[0])
        n = re_addr.search(tmp[1])

        if m is not None and n is not None:
       
                tmp_int_1 = ip2int(tmp[0])
                tmp_int_2 = ip2int(tmp[1])

                print(str(tmp_int_1) + "," + str(tmp_int_2) + "," + str(tmp[2]) + "," + str(tmp[3]) + "," + str(tmp[4]) + "," + str(tmp[5]).rstrip())

        else:
                print("0,0,0,0,0,0")


        line = f.readline()
    
