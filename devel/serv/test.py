import os
import sys
import socket
import ctypes
import struct
import time
import math
from datetime import datetime

import os

class GPS(ctypes.Structure):
    _fields_ = [
        ("latitude", ctypes.c_double),
        ("longitude", ctypes.c_double)
    ]   

class ServerData(ctypes.Structure):
    _fields_ = [
        ("time", ctypes.c_double),
        ("check_alarm", ctypes.c_bool),
        ("alarm", ctypes.c_int),
        ("dist_type", ctypes.c_bool),
        ("distance", ctypes.c_double),
        ("video_name", ctypes.c_char_p)
    ]

class TagData(ctypes.Structure):
    _fields_ = [
        ("time", ctypes.c_double),
        ("check_gps", ctypes.c_bool),
        ("check_uwb", ctypes.c_bool),
        ("gps", GPS),
        ("distance", ctypes.c_double),
        ("video_name", ctypes.c_char_p)
    ]

# IP = "10.8.0.100"
# PORT = 4000

# sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# sock.bind((IP, PORT))

# serv_data = ServerData()
# current_time = datetime.now()

# name = current_time.strftime("%Y-%m-%d_%H:%M:%S")

# clnt_data = TagData()

# print("Wait ...")
# clnt_msg, clnt_addr = sock.recvfrom(1024)



# fmt = "dbbddd25s"
# fmt_size = struct.calcsize(fmt)
# clnt_data.time, clnt_data.check_gps, clnt_data.check_uwb, clnt_data.gps.latitude, clnt_data.gps.longitude, clnt_data.distance, clnt_data.video_name = struct.unpack(fmt, clnt_msg[:fmt_size])

# video_name = clnt_data.video_name.decode()

# print(clnt_data.time)
# print(clnt_data.check_gps)
# print(clnt_data.check_uwb)
# print(clnt_data.gps.latitude)
# print(clnt_data.gps.longitude)
# print(clnt_data.distance)
# print(clnt_data.video_name)
# print(clnt_data.video_name.decode())


get_video_cmd = ["sshpass -p raspberry scp pi@10.8.0.101:/home/pi/Videos/", " /home/guseo/Downloads/"]
file_name = "scptest"

cmd = get_video_cmd[0] + file_name + get_video_cmd[1]
os.system(cmd)



# serv_data.time = 10
# serv_data.check_alarm = True
# serv_data.alarm = -1
# serv_data.dist_type = 1
# serv_data.distance = 1000
# serv_data.video_name = "1".encode()

# TxData_list = (serv_data.time, serv_data.check_alarm, serv_data.alarm, serv_data.dist_type, serv_data.distance, serv_data.video_name)
# fmt = 'dbibd19s'
# packer = struct.Struct(fmt)
# TxData = packer.pack(*TxData_list)

# print(TxData)
# sock.sendto(TxData, clnt_addr)