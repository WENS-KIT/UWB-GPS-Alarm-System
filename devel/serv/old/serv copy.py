from dataclasses import dataclass
from pickletools import int4
import sys
import socket
import ctypes
import struct

class GPS(ctypes.Structure):
    _fields_ = [
        ("latitude", ctypes.c_double),
        ("longitude", ctypes.c_double)
    ]   


class ClientData(ctypes.Structure):
    _fields_ = [
        ("time", ctypes.c_double),
        ("check_gps", ctypes.c_bool),
        ("check_uwb", ctypes.c_bool),
        ("gps", GPS),
        # ("latitude", ctypes.c_double),
        # ("longitude", ctypes.c_double),
        ("uwb_dist", ctypes.c_int)
    ]

class ServerData(ctypes.Structure):
    _fields_ = [
        ("time", ctypes.c_double),
        ("check_alarm", ctypes.c_bool),
        ("alarm", ctypes.c_int),
        ("dist_type", ctypes.c_bool),
        ("distance", ctypes.c_double)
    ]

addr = ("192.168.1.6", 4000)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(addr)



#########################
# 메세지 수신
clnt_msg, clnt_addr= sock.recvfrom(1024)
clnt_data = ClientData()
fmt = "dbbddi"
fmt_size = struct.calcsize(fmt)
clnt_data.time, clnt_data.check_gps, clnt_data.check_uwb, clnt_data.gps.latitude, clnt_data.gps.longitude, clnt_data.uwb_dist = struct.unpack(fmt, clnt_msg[:fmt_size])

print("Time : {}".format(clnt_data.time))
print("GPS  : {} [la:{}][long:{}]".format(clnt_data.check_gps, clnt_data.gps.latitude, clnt_data.gps.longitude))
print("UWB  : {} [d:{}]".format(clnt_data.check_uwb, clnt_data.uwb_dist))


#########################
# 메세지 송신
check_alarm = True
alarm = -1
distance = 10.0

fmt = 'dbibd'

serv_data = ServerData()
serv_data.time = 123456789
serv_data.check_alarm = True
serv_data.alarm = -2
serv_data.dist_type = 1
serv_data.distance = 123.345

TxData_list = (serv_data.time, serv_data.check_alarm, serv_data.alarm, serv_data.dist_type, serv_data.distance)
packer = struct.Struct(fmt)
TxData = packer.pack(*TxData_list)

sock.sendto(TxData, clnt_addr)


