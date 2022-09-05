/**
 * @file server_gps_v3.
 * @author Kwon DaeHyeon (dh0708@kumoh.ac.kr)
 * @brief 
 * @version 0.3
 * @date 2022-07-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

// General
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Socket Communication
#include <arpa/inet.h> 
#include <sys/types.h>
#include <sys/socket.h>

// GPS
#include <gps.h>
#include <math.h>

// UWB
#include <fcntl.h>
#include <termios.h> 

#define BUF_MAX 100

#define FAIL_CLNTSOCK -1
#define FAIL_CONNECT  -2

#define ALARM_NO		 0
#define ALARM_CLOSE 	-1
#define ALARM_CAREFULL  -2
#define ALARM_DANGER 	-3

struct GPS{
	double longitude; //경도
	double latitude; //위도
};

struct ServerMsg{
	bool check_alarm;
	int alarm;
	double distance;
};

struct ClientMsg{
	bool check_gps;
	bool check_uwb;
	struct GPS gps;
	int uwb_dist; 
};


struct Socket{
    int serv_sock;
	int clnt_sock;

    struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;
	socklen_t serv_addr_size;
};

void error_handling(char *message);

int setup_serial(int * ttyfd, struct termios *tio, char *ttyname);
int read_serial(int *ttyfd, void *message, size_t buf);

int setup_socket(struct Socket *sock_data, char *argv[]);
int read_socket(int fd, void *buf, int buf_size);
int read_socket_with_timeout(int fd, void *buf, int buf_size, int timeout_ms);

void setup_gps(struct gps_data_t *gps_data);
bool read_gps(struct gps_data_t *gps_data, int wait_time_msec);