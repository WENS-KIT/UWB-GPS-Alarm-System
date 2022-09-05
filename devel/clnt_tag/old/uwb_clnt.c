#include "uwb_clnt.h"

int main(int argc, char *argv[])
{
	// GPS
	struct gps_data_t gps_data;
	struct GPS gps_data_clnt;

	char gps_msg[100];
	bool get_gps = false;

	// UWB Serial
	struct termios newtio;
	int ttyfd;
	int res_gps;
 	char *ttyname = "/dev/ttyACM0";
	char uwb_msg[100];
	int uwb_dist_clnt = 0;
	bool get_serial;

	// Socket
	struct Socket sock_data;
	struct ClientMsg TxData;
	struct ServerMsg RxData;

	char *sock_state_msg;
	char *alarm_state_msg;

    int strLen;
    int alarm = 0;

    if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
        return 0;
	}

	if (setup_socket(&sock_data, argv) < 0)
		return 0;


	if (setup_serial(&ttyfd, &newtio, ttyname) == -1)
		error_handling("[ERROR] Setup Serial");

	setup_gps(&gps_data);

    TxData.gps.longitude = 0;
	TxData.gps.latitude =  0;
	TxData.uwb_dist = 0;
	
	int try = 1;
	int receive_perfect = 0;
	int receive_bad = 0;

	struct timeval start = {};
	struct timeval end = {};

	double time;

	while (1)
	{

		get_gps = read_gps(&gps_data, 1);
		if(get_gps){
			TxData.check_gps = get_gps;
			TxData.gps.latitude = gps_data.fix.latitude;
			TxData.gps.longitude = gps_data.fix.longitude;
		}

		int dist_temp = read_serial(&ttyfd, uwb_msg, sizeof(uwb_msg));
		if (dist_temp > 0){
			
			gettimeofday(&start, NULL);

			uwb_dist_clnt = dist_temp;

			get_serial = true;
			TxData.check_uwb = get_serial;
			TxData.uwb_dist = uwb_dist_clnt;
		}

		gettimeofday(&end, NULL);
		double time = end.tv_sec + end.tv_usec / 1000000.0 - start.tv_sec + start.tv_usec/1000000.0;
		
		if (time >= 5)
		{
			get_serial = 0;
			uwb_dist_clnt = 0;

			TxData.check_uwb = get_serial;
			TxData.uwb_dist = uwb_dist_clnt;
		}
		
		sendto(sock_data.clnt_sock, (struct ClientMsg*)&TxData, sizeof(TxData), 0, (struct sockaddr*)&sock_data.serv_addr, sizeof(sock_data.serv_addr)); //UDP
		// write(sock_data.clnt_sock, (struct ClientMsg*)&TxData, sizeof(TxData)); //TCP


		strLen = recvfrom(sock_data.clnt_sock, (struct ServerMsg*)&RxData, sizeof(RxData), 0, (struct sockaddr*)&sock_data.serv_addr, &sock_data.serv_addr_size);
	    // strLen = read(sock_data.clnt_sock, (struct ServerMsg*)&RxData, sizeof(RxData)); //TCP


	    if(strLen == -1)
            sock_state_msg = "ERROR";
        else if(strLen == 0)
			sock_state_msg = "NO MESSAGE";
        else
        {
			sock_state_msg = "SUCCESS";
			if (RxData.check_alarm){
				alarm = RxData.alarm;

				if (alarm == ALARM_NO)
					alarm_state_msg = "-";
				else if (alarm == ALARM_CLOSE)
					alarm_state_msg = "CLOSE";
				else if (alarm == ALARM_CAREFULL)
					alarm_state_msg = "CAREFULL";
				else if (alarm == ALARM_DANGER)
					alarm_state_msg = "DANGER";
				else
					alarm_state_msg = "???";
			}
        }


		printf("+-------------------------------------------+\n");
		printf("Try %d\n", try++);
		printf("Socket     : %s \n", sock_state_msg);
		printf("Client GPS : %d [lat:%lf][lon:%lf] \n", get_gps, gps_data_clnt.latitude, gps_data_clnt.longitude);
		printf("UWB Data   : %d [d:%lf] \n", get_serial, (double)uwb_dist_clnt);
		printf("Alarm      : %s \n", alarm_state_msg);
		printf("+-------------------------------------------+\n\n");
		
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

int read_with_timeout(int fd, void *buf, int buf_size, int timeout_ms)
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
        printf("code: %d, reason: %s\n", rc, gps_errstr(rc));
        return;
    }

    gps_stream(gps_data, WATCH_ENABLE | WATCH_JSON, NULL);
}

bool read_gps(struct gps_data_t *gps_data, int wait_time_msec)
{
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
		printf( ">> tty Open Fail [%s]\r\n ", ttyname);
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
		// printf("[ERROR] Read Seiral\n");
		return -1;
	}
	else if(res == 0)
	{
		// printf("[WARN] No Serial Data\n");
		return 0;
	}
	else
	{
		sub = strtok(message, "\n");
        distance = atoi(sub);
		// printf("[SUCCESS] Distance : %d\n", distance);
	}
	return distance;
}

int setup_socket(struct Socket *sock_data, char *argv[])
{
	sock_data->clnt_sock = socket(PF_INET, SOCK_DGRAM, 0); // UDP
	//sock_data->clnt_sock = socket(PF_INET, SOCK_STREAM, 0); // TCP

	// Commmon ==========
	if (sock_data->clnt_sock == -1){
        printf("[ERROR] Create Client Socket Fail\n");
		return FAIL_CLNTSOCK;
    }


	memset(&sock_data->serv_addr, 0, sizeof(sock_data->serv_addr));
	sock_data->serv_addr.sin_family = AF_INET;
	sock_data->serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	sock_data->serv_addr.sin_port = htons(atoi(argv[2]));

	sock_data->serv_addr_size = sizeof(sock_data->serv_addr);
	// ==================

    /*
	// TCP
	if(connect(sock_data->clnt_sock, (struct sockaddr*) &sock_data->serv_addr, sizeof(sock_data->serv_addr))==-1 ){
        printf("[ERROR] Socket Connect\n");
        return FAIL_CONNECT;
    }
	*/
	
	return 0;
}
