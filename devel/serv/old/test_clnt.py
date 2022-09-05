import socket
import struct
import ctypes

class Test(ctypes.Structure):
    _fields_ = [
        ("check_alarm", ctypes.c_bool),
        ("alarm", ctypes.c_int),
        ("distance", ctypes.c_double),
    ]

addr = ("192.168.1.6", 4000)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# sock.bind(addr)


#########################
# # 메세지 송신
# check_alarm = True
# alarm = -1
# distance = 10.0

# fmt = 'bid'

# values = (check_alarm, alarm, distance)
# packer = struct.Struct(fmt)
# sendData = packer.pack(*values)

# sock.sendto(sendData, addr)


#########################
# 메세지 수신
fmt = 'bid'

serv_msg, _ = sock.recvfrom(1024)
fmt = "bid"
fmt_size = struct.calcsize(fmt)
check_alarm, alarm, distance = struct.unpack(fmt, serv_msg[:fmt_size])

print("{} {} {}".format(check_alarm, alarm, distance))











# for i in range(1,10):
#     print("try : {}".format(i))

#     serv_msg, _ = sock.recvfrom(1024)

#     RxData = Test()

#     fmt = "bid"
#     fmt_size = struct.calcsize(fmt)
#     RxData.check_alarm, RxData.alarm, RxData.distance = struct.unpack(fmt, serv_msg[:fmt_size])

#     print("check_alarm  : {}".format(RxData.check_alarm))
#     print("alarm        : {}".format(RxData.alarm))
#     print("distance     : {}".format(RxData.distance))