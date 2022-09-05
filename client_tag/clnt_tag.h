/**
 * @file clnt_tag
 * @author Kwon DaeHyeon (dh0708@kumoh.ac.kr)
 * @brief Raspberry Pi - UWB tag code
 * @version 1.0.0
 * @date 2022-07-11
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

#define GPS_DIST 0
#define UWB_DIS 1

struct GPS{
	double latitude; 
	double longitude; 
};

struct ServerData{
	double time;
	bool check_alarm;
	int alarm;
	bool dist_type;
	double distance;
};

struct TagData{
	double time;
	bool check_gps;
	bool check_uwb;
	struct GPS gps;
	double uwb_dist; 
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

void setup_gps(struct gps_data_t *gps_data);
bool read_gps(struct gps_data_t *gps_data, int wait_time_msec);

int setup_serial(int * ttyfd, struct termios *tio, char *ttyname);
int read_serial(int *ttyfd, void *message, size_t buf);

int setup_socket(struct Socket *sock_data, char *argv[]);
