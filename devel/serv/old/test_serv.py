from re import I
import socket
import struct
import ctypes
import time

class ClientData(ctypes.Structure):
    _fields_ = [
        ("time", ctypes.c_double),
        ("check_gps", ctypes.c_bool),
        ("check_uwb", ctypes.c_bool),
        ("latitude", ctypes.c_double),
        ("longitude", ctypes.c_double),
        ("uwb_dist", ctypes.c_int)
    ]

addr = ("192.168.1.6", 4000)

# 소캣 생성 및 바인드
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(addr)

###################################
# 메세지 수신 및 가공
clnt_msg, clnt_addr = sock.recvfrom(1024)

RxData = ClientData()

fmt = "dbbddi"
fmt_size = struct.calcsize(fmt)
RxData.time, RxData.check_gps, RxData.check_uwb, RxData.latitude, RxData.longitude, RxData.uwb_dist = struct.unpack(fmt, clnt_msg[:fmt_size])


print("time        : {}".format(RxData.time))
print("check_gps   : {}".format(RxData.check_gps))
print("check_uwb   : {}".format(RxData.check_uwb))
print("latitude    : {}".format(RxData.latitude))
print("longitude   : {}".format(RxData.longitude))
print("uwb_dis     : {}".format(RxData.uwb_dist))
###################################



###################################
# 메세지 송신
check_alarm = True
alarm = -1
distance = 10.0

fmt = "bid"
values = (check_alarm, alarm, distance)
packer = struct.Struct(fmt)
TxData = packer.pack(*values)

print("send data")
print("check_alarm  : {}".format(check_alarm))
print("alamr        : {}".format(alarm))
print("distance     : {}".format(distance))



sock.sendto(TxData, clnt_addr)
###################################



#########################
# # 메세지 수신
# fmt = 'bid'

# serv_msg, _ = sock.recvfrom(1024)
# fmt = "bid"
# fmt_size = struct.calcsize(fmt)
# check_alarm, alarm, distance = struct.unpack(fmt, serv_msg[:fmt_size])

# print("{} {} {}".format(check_alarm, alarm, distance))

#########################
# # 메세지 송신
# check_alarm = True
# alarm = -1
# distance = 10.0

# fmt = 'bid'

# values = (check_alarm, alarm, distance)
# packer = struct.Struct(fmt)
# sendData = packer.pack(*values)

# while True:
#     sock.sendto(sendData, addr)