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
#define __USE_MISC 1
#include <math.h>

// UWB
#include <fcntl.h>
#include <termios.h> 

#define BUF_MAX 100
#define BUF 10

#define FAIL_SERVSOCK -1
#define FAIL_BIND     -2
#define FAIL_LISTEN   -3
#define FAIL_CLNTSOCK -4

#define ALARM_NO 		0
#define ALARM_CLOSE 	-1
#define ALARM_CAREFULL  -2
#define ALARM_DANGER 	-3

struct TIME_VAL
{
    char year[BUF];
    char month[BUF];   
    char day[BUF];
    char hour[BUF];
    char min[BUF];
    char sec[BUF];
};

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
};

void error_handling(char *message);

int setup_serial(int * ttyfd, struct termios *tio, char *ttyname);
int read_serial(int *ttyfd, void *message, size_t buf);

int setup_socket(struct Socket *sock_data, char *argv[]);
int read_socket_with_timeout(int fd, void *buf, int buf_size, int timeout_ms);

void setup_gps(struct gps_data_t *gps_data);
bool read_gps(struct gps_data_t *gps_data, int wait_time_msec);

double calculate_gps_dist(struct GPS gps1, struct GPS gps2);

int calculate_alarm(double dist);
void calculate_camera_command();