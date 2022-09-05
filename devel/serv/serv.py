import sys
import socket
import ctypes
import struct
import time
import math
import os

import threading
from datetime import datetime

ALARM_NO       =  0
ALARM_CLOSE    = -1
ALARM_CAREFULL = -2
ALARM_DANGER   = -3

GPS_DIST = 0
UWB_DIST = 1


class GPS(ctypes.Structure):
    _fields_ = [
        ("latitude", ctypes.c_double),
        ("longitude", ctypes.c_double)
    ]   

class AnchorData(ctypes.Structure):
    _fields_ = [
        ("time", ctypes.c_double),
        ("check_gps", ctypes.c_bool),
        ("gps", GPS)
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

class ServerData(ctypes.Structure):
    _fields_ = [
        ("time", ctypes.c_double),
        ("check_alarm", ctypes.c_bool),
        ("alarm", ctypes.c_int),
        ("dist_type", ctypes.c_bool),
        ("distance", ctypes.c_double),
    ]

def printMsg(sock_group, serv_data, clnt_msg_group):
    dist_type_str = ""
    if serv_data.dist_type:
        dist_type_str = "UWB"
    else:
        dist_type_str = "GPS"

    alarm_str = ""
    if serv_data.alarm == ALARM_NO:
        alarm_str = "-"
    elif serv_data.alarm == ALARM_CLOSE:
        alarm_str = "CLOSE"
    elif serv_data.alarm == ALARM_CAREFULL:
        alarm_str = "CAREFULL"
    elif serv_data.alarm == ALARM_DANGER:
        alarm_str = "DANGER"

    print("".ljust(150, '='))
    print("Anchor : {}".format(sock_group[0].getsockname()).ljust(55), end='')
    print("Tag    : {}".format(sock_group[1].getsockname()).ljust(55), end='')
    print("Server : ".ljust(55))

    print("Time   : {}".format(clnt_msg_group[0].time).ljust(55), end='')
    print("Time   : {}".format(clnt_msg_group[1].time).ljust(55), end='')
    print("Time   : {}".format(serv_data.time).ljust(55))

    print("GPS    : {} [lat:{}][long:{}]".format(clnt_msg_group[0].check_gps, clnt_msg_group[0].gps.latitude, clnt_msg_group[0].gps.longitude).ljust(55), end='')
    print("GPS    : {} [lat:{}][long:{}]".format(clnt_msg_group[1].check_gps, clnt_msg_group[1].gps.latitude, clnt_msg_group[1].gps.longitude).ljust(55), end='')
    print("Alarm  : {} [{}]".format(serv_data.check_alarm, alarm_str).ljust(55))

    print("Video  : {}".format(clnt_msg_group[0].video_name.decode()).ljust(55), end='')
    print("UWB    : {} [d:{}]".format(clnt_msg_group[1].check_uwb, clnt_msg_group[1].distance).ljust(55), end='')
    print("Dist   : {}  [d:{}]".format(dist_type_str, serv_data.distance).ljust(55))
    print("".ljust(150, '='), end='\n\n')




# Transmit server data to every client
def sendMsg(check_recv, sock_group, clnt_addr_group, clnt_msg_group):  
    print("Thread send start")

    # After receiving and anchor & tag message once, Send function operate 
    while True:
        count = 0
        for i in range(0, len(sock_group)):
            if check_recv[i] != 0:
                count = count + 1
        
        if count == len(sock_group):
            print("Thread send is now working")
            break

    time_now = time.time()
    time_in = time_now
    time_out = time_now

    while True:
        # If every client message are recieved, Execute below code
        if (check_recv[0] and check_recv[1]):
            check_recv[0] = 0
            check_recv[1] = 0
            time_now = time.time()
            time_in = time_now
            serv_data = ServerData()
            serv_data.time = time_now
            result = 0

            # Check time duration
            for i in range(0, 2):
                time_tv =  time_now - clnt_msg_group[i].time
                if time_tv > 0.2:
                    result = 1
                    break
            
            # Both data are within 0.2 seconds compared to the current time
            if result == False:
                if clnt_msg_group[1].check_uwb:
                    uwb_dist = clnt_msg_group[1].distance

                    serv_data.check_alarm = True
                    serv_data.alarm = generate_alarm(uwb_dist)
                    serv_data.dist_type = UWB_DIST
                    serv_data.distance = uwb_dist

                
                else:
                    if clnt_msg_group[0].check_gps and clnt_msg_group[1].check_gps:
                        gps_dist = calculate_dist(clnt_msg_group[0].gps, clnt_msg_group[1].gps)
                        
                        serv_data.check_alarm = True
                        serv_data.alarm = generate_alarm(gps_dist)
                        serv_data.dist_type = GPS_DIST
                        serv_data.distance = gps_dist


            TxData_list = (serv_data.time, serv_data.check_alarm, serv_data.alarm, serv_data.dist_type, serv_data.distance)
            fmt = 'dbibd'
            packer = struct.Struct(fmt)
            TxData = packer.pack(*TxData_list)

            for i in range(len(sock_group)):
                sock_group[i].sendto(TxData, clnt_addr_group[i])


            printMsg(sock_group, serv_data, clnt_msg_group)
            

        else:
            # If client message does not come in for a certain period of time, Timeout occurs and an empty message is sent
            time_out = time.time()
            timeout = 2
            if time_out - time_in > timeout:
                serv_data = ServerData()
                serv_data.time = time_out
                time_in = time_out

                print("- - - - - - - - - - - - - -")
                print("[TIMEOUT] Send Empty Msg [T:{}]".format(timeout))
                print("Time  : {}".format(serv_data.time))
                print("Alarm : {} [{}]".format(serv_data.check_alarm, serv_data.alarm))
                print("Dist  : {} [d:{}]".format(serv_data.dist_type, serv_data.distance))
                print("- - - - - - - - - - - - - -", end='\n\n')
                
                TxData_list = (serv_data.time, serv_data.check_alarm, serv_data.alarm, serv_data.dist_type, serv_data.distance)
                fmt = 'dbibd'
                packer = struct.Struct(fmt)
                TxData = packer.pack(*TxData_list)

                for i in range(len(sock_group)):
                    sock_group[i].sendto(TxData, clnt_addr_group[i])
        



def recvMsg(check_recv, sock, clnt_port_group, clnt_addr_group, clnt_msg_group):

    print("Thread recive start [{}]".format(sock.getsockname()))

    # To paste the raspberry pi video
    get_video_cmd = ["sshpass -p raspberry scp pi@10.8.0.101:/home/pi/Videos/", " /home/guseo/Downloads/"]
    current_video = ""

    while True:
        clnt_msg, clnt_addr = sock.recvfrom(1024)

        if len(clnt_msg) == 32:
            # Anchor
            check_recv[0] = 1
            clnt_addr_group[0] = clnt_addr
            clnt_data = AnchorData()
            fmt = "dbdd"
            fmt_size = struct.calcsize(fmt)
            clnt_data.time, clnt_data.check_gps, clnt_data.gps.latitude, clnt_data.gps.longitude = struct.unpack(fmt, clnt_msg[:fmt_size])
            clnt_msg_group[0] = clnt_data
            
            # print("==========================")
            # print("Anchor : {}".format(sock.getsockname()))
            # print("Time   : {}".format(clnt_data.time))
            # print("GPS    : {} [lat:{}][long:{}]".format(clnt_data.check_gps, clnt_data.gps.latitude, clnt_data.gps.longitude))
            # print("==========================", end='\n\n')

        else:
            # Tag
            check_recv[1] = 1
            clnt_addr_group[1] = clnt_addr
            clnt_data = TagData()
            fmt = "dbbddd25s"
            fmt_size = struct.calcsize(fmt)
            clnt_data.time, clnt_data.check_gps, clnt_data.check_uwb, clnt_data.gps.latitude, clnt_data.gps.longitude, clnt_data.distance, clnt_data.video_name = struct.unpack(fmt, clnt_msg[:fmt_size])
            clnt_msg_group[1] = clnt_data

            
            if clnt_data.video_name.decode() != current_video:
                current_video = clnt_data.video_name.decode()

                cmd = get_video_cmd[0] + current_video + get_video_cmd[1]
                os.system(cmd)

            # print("==========================")
            # print("Tag  : {}".format(sock.getsockname()))
            # print("Time : {}".format(clnt_data.time))
            # print("GPS  : {} [lat:{}][long:{}]".format(clnt_data.check_gps, clnt_data.gps.latitude, clnt_data.gps.longitude))
            # print("UWB  : {} [d:{}]".format(clnt_data.check_uwb, clnt_data.distance))
            # print("==========================", end='\n\n')

        


def calculate_dist(gps1, gps2):
    R = 6371e3
    phi1 = gps1.latitude * math.pi/180
    phi2 = gps2.latitude * math.pi/180
    del_phi = (gps2.latitude - gps1.latitude) * math.pi/180
    del_ramb = (gps2.longitude - gps1.longitude) * math.pi/180

    a = math.sin(del_phi/2) * math.sin(del_phi/2) + math.cos(phi1) * math.cos(phi2) * math.sin(del_ramb/2) * math.sin(del_ramb/2)
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1-a))
    d = R * c

    return d

def generate_alarm(dist_meter):
    alarm = 0
    if dist_meter < 0.3:
        alarm = ALARM_DANGER
    elif dist_meter < 1:
        alarm = ALARM_CAREFULL
    elif dist_meter < 1.5:
        alarm = ALARM_CLOSE

    return alarm



if __name__ == '__main__':
    # Store IP & Port using arguments
    if len(sys.argv) != 4:
        print("Usage : <IP> <anchor_port> <tag_port>")
        sys.exit()

    check_recv = [0 for i in range(len(sys.argv)-2)]
    sock_group = [0 for i in range(len(sys.argv)-2)]
    clnt_port_group = [0 for i in range(len(sys.argv)-2)]
    clnt_addr_group = [(" ",0),(" ",0)]
    clnt_msg_group  = [AnchorData(), TagData()]
    thread_group = []

    serv_ip = sys.argv[1]

    for i in (2 , len(sys.argv) - 1):
        j = i-2
        clnt_port_group[j] = int(sys.argv[i])
        sock_group[j] = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock_group[j].bind((serv_ip, clnt_port_group[j]))

    thread0 = threading.Thread(target=sendMsg,args=(check_recv, sock_group, clnt_addr_group, clnt_msg_group))
    thread1 = threading.Thread(target=recvMsg,args=(check_recv, sock_group[0], clnt_port_group, clnt_addr_group, clnt_msg_group,))
    thread2 = threading.Thread(target=recvMsg,args=(check_recv, sock_group[1], clnt_port_group, clnt_addr_group, clnt_msg_group,))

    thread0.start()
    thread1.start()
    thread2.start()
