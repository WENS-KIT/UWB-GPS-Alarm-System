#include "uwb_serv.h"

int main(int argc, char *argv[])
{
	// GPS
	struct gps_data_t gps_data;
	struct GPS gps_data_serv;
	struct GPS gps_data_clnt;
	char gps_msg[100];
	double gps_dist;

	bool get_gps_serv;
	bool get_gps_clnt;
	bool gps_calculated;

	//// UWB Serial
	// struct termios newtio;
	// int ttyfd;
	// int res_gps;
 	// char *ttyname = "/dev/ttyACM0";
	// char uwb_msg[100];
	// int uwb_dist_server;

	bool get_uwb_clnt;
	int uwb_dist_clnt;

	// Socket
	struct Socket sock_data;
	struct ServerMsg TxData;
	struct ClientMsg RxData;

	int get_alarm = 0;
	int alarm = ALARM_NO;
	int strLen;

	char *sock_state_msg = "-";
	char *alarm_state_msg = "-";


	if (argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		return 0;
	}

	if (setup_socket(&sock_data, argv) < 0)
		error_handling("[ERROR] Setup socket");

	setup_gps(&gps_data);

	// if (setup_serial(&ttyfd, &newtio, ttyname) == -1)
	// 	error_handling("[ERROR] Setup Serial");
	

	int try = 1;

	char camera_cmd[BUF_MAX];

	RxData.check_gps = 0;
	RxData.check_uwb = 0;
	RxData.gps.latitude = 0;
	RxData.gps.longitude = 0;
	RxData.uwb_dist = 0;

	bool test = false;
	struct timeval start = {};
	struct timeval end = {};
	double time;

	bool record = false;

	while (1)
	{
		if ((get_gps_serv = read_gps(&gps_data, 1)) == 1){
			gps_data_serv.latitude = gps_data.fix.latitude;
			gps_data_serv.longitude = gps_data.fix.longitude;
		}

		// int dist_temp = read_serial(&ttyfd, uwb_msg, sizeof(uwb_msg));
		// if (dist_temp > 0)
		// 	uwb_dist_server = dist_temp;

		// strLen = read_socket_with_timeout(sock_data.clnt_sock, (struct ClientMsg*)&RxData, sizeof(RxData), 1000);


		struct timeval optVal = {1, 0};
		int optLen = sizeof(optVal);

		setsockopt(sock_data.serv_sock, SOL_SOCKET, SO_RCVTIMEO, &optVal, optLen);

		strLen = recvfrom(sock_data.serv_sock, (struct ClientMsg*)&RxData, sizeof(RxData), 0, (struct sockaddr*)&sock_data.clnt_addr, &sock_data.clnt_addr_size);
		// strLen = read(sock_data.clnt_sock, (struct ClientMsg*)&RxData, sizeof(RxData)); //TCP
 

		// 소켓 통신 데이터 획득 및 저장
		if (strLen == -1)
			sock_state_msg = "ERROR";
		else if (strLen == 0)
			sock_state_msg = "NO MESSAGE";
		else
		{
			sock_state_msg = "SUCCESS";

			get_gps_clnt = RxData.check_gps;
			if (get_gps_clnt)
				gps_data_clnt = RxData.gps;
		
			get_uwb_clnt = RxData.check_uwb;
			if (get_uwb_clnt)
				uwb_dist_clnt = RxData.uwb_dist;
			
		}

		if (get_gps_serv && get_gps_clnt)
			gps_dist = calculate_gps_dist(gps_data_serv, gps_data_clnt);


		if (sock_state_msg == "SUCCESS")
		{
			if (uwb_dist_clnt > 0 && RxData.check_uwb)
			{
				alarm = calculate_alarm(uwb_dist_clnt);
				get_alarm = 1;
				if (alarm == ALARM_NO)
					alarm_state_msg = "-";
				else if (alarm == ALARM_CLOSE)
					alarm_state_msg = "CLOSE";
				else if (alarm == ALARM_CAREFULL)
					alarm_state_msg = "CAREFULL";
				else if (alarm == ALARM_DANGER)
				{
					if (record == false)
					{
						record = true;
						calculate_camera_command(&camera_cmd);
						system(camera_cmd);
						gettimeofday(&start, NULL);
					}

					alarm_state_msg = "DANGER";
				}
				else
					alarm_state_msg = "???";

				TxData.check_alarm = true;
				TxData.alarm = alarm;
				TxData.distance = uwb_dist_clnt;
			}
			else
			{
				if (get_gps_serv && RxData.check_gps && strLen > 0)
				{
					alarm = calculate_alarm(gps_dist);
					get_alarm = 2;

					if (alarm == ALARM_NO)
						alarm_state_msg = "-";
					else if (alarm == ALARM_CLOSE)
						alarm_state_msg = "CLOSE";
					else if (alarm == ALARM_CAREFULL)
						alarm_state_msg = "CAREFULL";
					else if (alarm == ALARM_DANGER)
					{
						if (record == false)
						{
							record = true;
							calculate_camera_command(&camera_cmd);
							system(camera_cmd);
							gettimeofday(&start, NULL);
						}

						alarm_state_msg = "DANGER";
					}
					else
						alarm_state_msg = "???";

					TxData.check_alarm = true;
					TxData.alarm = alarm;
					TxData.distance = gps_dist;
				}
				else
				{
					get_alarm = -1;
				}
			}
		}
		else
		{
			get_alarm = -1;
		}

		if (record)
		{
			gettimeofday(&end, NULL);
			time = end.tv_sec + end.tv_usec / 1000000.0 - start.tv_sec + start.tv_usec / 1000000.0;
			if (time >= 6)
			{
				record = false;
				memset(camera_cmd, 0, sizeof(camera_cmd));

			}
		}

		printf("+-------------------------------------------+\n");
		printf("Try %d\n", try++);
		printf("Socket     : %s \n", sock_state_msg);
		printf("Server GPS : %d [lat:%lf][lon:%lf] \n", get_gps_serv, gps_data_serv.latitude, gps_data_serv.longitude);
		printf("Client GPS : %d [lat:%lf][lon:%lf] \n", get_gps_clnt, gps_data_clnt.latitude, gps_data_clnt.longitude);
		printf("GPS Dist   : %d [d:%lf] \n",  gps_dist);
		printf("UWB Dist   : %d [d:%lf] \n", get_uwb_clnt, (double)uwb_dist_clnt);
		printf("Alarm      : %d [%s] \n", get_alarm, alarm_state_msg);
		printf("Record     : %d [T:%lf] \n", record, time);
		printf("CMD : %s \n", camera_cmd);
		printf("+-------------------------------------------+\n\n");


		sendto(sock_data.serv_sock, (struct ServerMsg*)&TxData, sizeof(TxData), 0, (struct sockaddr*)&sock_data.clnt_addr, sizeof(sock_data.clnt_addr));
		// write(sock_data.clnt_sock, (struct ServerMsg*)&TxData, sizeof(TxData)); //TCP
	
		// if (!test)
		// {
		// 	test = true;
		// 	calculate_camera_command(&camera_cmd);
		// 	printf("[CMD] %s\n", camera_cmd);
		// 	system(camera_cmd);
		// }

		// printf("[CMD] %s\n", camera_cmd);

	}

	close(sock_data.clnt_sock);
	close(sock_data.serv_sock);

	gps_stream(&gps_data, WATCH_DISABLE, NULL);
	gps_close (&gps_data);

	return 0;
}


void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

int read_socket_with_timeout(int fd, void *buf, int buf_size, int timeout_ms)
{
    int rx_len = 0;
    struct    timeval  timeout;
    fd_set  	readFds;

    // recive time out config
    // Set 1ms timeout counter
    timeout.tv_sec  = 0;
    timeout.tv_usec = timeout_ms*1000;

    FD_ZERO(&readFds);
    FD_SET(fd, &readFds);
    select(fd+1, &readFds, NULL, NULL, &timeout);

    if(FD_ISSET(fd, &readFds))
    {
        rx_len = read(fd, buf, buf_size);		
    }

	return rx_len;
}

void setup_gps(struct gps_data_t *gps_data)
{
    int rc;
    if ((rc = gps_open("localhost", "2947", gps_data)) == -1)
    {
        printf("[ERROR] GPS Open [code:%d][reason: %s]\n", rc, gps_errstr(rc));
        return;
    }

    gps_stream(gps_data, WATCH_ENABLE | WATCH_JSON, NULL);
}

bool read_gps(struct gps_data_t *gps_data, int wait_time_msec)
{;
    char msg[BUF_MAX];
	bool result = false;

    if (gps_waiting(gps_data, 1000*wait_time_msec))
	{
		if (gps_read(gps_data, msg, sizeof(msg)) > 0)
		{
			/* Display data from the GPS receiver. */
			if ((gps_data->fix.status == STATUS_FIX) &&
				(gps_data->fix.mode == MODE_2D || gps_data->fix.mode == MODE_3D) &&
				!isnan(gps_data->fix.latitude) &&
				!isnan(gps_data->fix.longitude))
			{
				// gettimeofday(&tv, NULL); EDIT: tv.tv_sec isn't actually the timestamp!
				result = true;
			}
		}
	}

	return result;
}

int setup_serial(int * ttyfd, struct termios *tio, char *ttyname)
{
	*ttyfd = open(ttyname, O_RDONLY | O_NOCTTY);

	if(ttyfd < 0)
	{
		printf("[ERROR] tty Open Fail [Name:%s]\r\n ", ttyname);
		return -1;
	}

	memset(tio, 0, sizeof(tio));

	tio->c_cflag = B115200 | CS8 | CLOCAL | CREAD;
	tio->c_iflag = IGNPAR;
	tio->c_oflag = 0;

	//set input mode (non-canonical, no echo,.....)
	tio->c_lflag     = 0;     // LF recive filter unused
	tio->c_cc[VTIME] = 0;     // inter charater timer unused
	tio->c_cc[VMIN]  = 0;     // blocking read until 1 character arrives

	tcflush(*ttyfd, TCIFLUSH ); // inital serial port
	tcsetattr(*ttyfd, TCSANOW, tio ); // setting serial communication

	return 0;
}

int read_serial(int *ttyfd, void *message, size_t buf)
{
	int res = read(*ttyfd, message, buf);
	char *sub;
	int distance = 0;
	
	if(res < 0)
	{
		printf("[ERROR] Read Seiral\n");
		return -1;
	}
	else if(res == 0)
	{
		printf("[WARN] No Serial Data\n");
		return 0;
	}
	else
	{
		sub = strtok(message, "\n");
        distance = atoi(sub);
		printf("[SUCCESS] Distance : %d\n", distance);
	}
	return distance;
}

int setup_socket(struct Socket *sock_data, char *argv[])
{
	sock_data->serv_sock = socket(PF_INET, SOCK_DGRAM, 0); // UDP
	// sock_data->serv_sock = socket(PF_INET, SOCK_STREAM, 0); //TCP


	// Common
	if (sock_data->serv_sock == -1)
		return -1;

	memset(&sock_data->serv_addr, 0, sizeof(sock_data->serv_addr));
	sock_data->serv_addr.sin_family = AF_INET;
	sock_data->serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	sock_data->serv_addr.sin_port = htons(atoi(argv[1]));

	sock_data->clnt_addr_size = sizeof(sock_data->clnt_addr);

	if(bind(sock_data->serv_sock, (struct sockaddr*) &sock_data->serv_addr, sizeof(sock_data->serv_addr))== -1)
	{
		printf("[ERROR] Socket Bind\n");
		return FAIL_BIND;
	}
	//===========

	/*
	//TCP
	if (listen(sock_data->serv_sock, 5) == -1)
	{
		printf("[ERROR] Socket Listen\n");
		return FAIL_LISTEN;
	}

	sock_data->clnt_sock = accept(sock_data->serv_sock, (struct sockaddr *)&sock_data->clnt_addr, &sock_data->clnt_addr_size);
	if (sock_data->clnt_sock == -1)
		error_handling("accept() error");
	*/
	
	return 0;
}

double calculate_gps_dist(struct GPS gps1, struct GPS gps2)
{
	double R = 6371e3;
	double phi1 = gps1.latitude * M_PI/180;
	double phi2 = gps2.latitude * M_PI/180;
	double del_phi = (gps2.latitude - gps1.latitude) * M_PI/180;
	double del_ramb = (gps2.longitude - gps1.longitude) * M_PI/180;

	double a = sin(del_phi/2) * sin(del_phi/2) + cos(phi1) * cos(phi2) * sin(del_ramb/2) * sin(del_ramb/2);
	double c = 2 * atan2(sqrt(a), sqrt(1-a));

	double d = R * c;
	
	return d;
}

int calculate_alarm(double dist)
{
	// 1.5m ,  1m, 30cm
	if (dist < 30)
		return ALARM_DANGER;
	else if (dist < 100)
		return ALARM_CAREFULL;
	else if (dist < 150)
		return ALARM_CLOSE;

	return 0;
}

void calculate_camera_command(char *command)
{
	time_t timer;
    struct tm* t;
    
    timer = time(NULL);
    t = localtime(&timer);

    struct TIME_VAL tm;

    char cmd[BUF_MAX] = "sudo libcamera-vid -t 5000 -n -o /home/pi/Videos/";
    

    sprintf(tm.year, "%d", t->tm_year + 1900);
    sprintf(tm.month, "%d", t->tm_mon + 1);
    sprintf(tm.day, "%d", t->tm_mday);
    sprintf(tm.hour, "%d", t->tm_hour);
    sprintf(tm.min, "%d", t->tm_min);
    sprintf(tm.sec, "%d", t->tm_sec);

    strcat(cmd, tm.year);
    strcat(cmd, "-");
    strcat(cmd, tm.month);
    strcat(cmd, "-");
    strcat(cmd, tm.day);
    strcat(cmd, "_");
    strcat(cmd, tm.hour);
    strcat(cmd, ":");
    strcat(cmd, tm.min);
    strcat(cmd, ":");
    strcat(cmd, tm.sec);
    strcat(cmd, ".h264 &");


	strcat(command, cmd);
	// command = cmd;
}