#include "clnt_tag.h"

int main(int argc, char *argv[])
{
	// GPS
	struct gps_data_t gps_data;
	char gps_msg[BUF_MAX];

	// UWB Serial
	struct termios newtio;
	int ttyfd;
 	char *ttyname = "/dev/ttyACM0";
	char uwb_msg[BUF_MAX];

	// Socket
	struct Socket sock_data;
	struct ServerData RxData = {};
	struct ServerData RxData_storage = {};

	char *sock_state_msg;
	char *alarm_state_msg;

    int strLen;

    if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
        return 0;
	}

	if (setup_socket(&sock_data, argv) < 0)
		error_handling("[ERROR] Setup Socket");


	if (setup_serial(&ttyfd, &newtio, ttyname) == -1)
		error_handling("[ERROR] Setup Serial");

	setup_gps(&gps_data);
	
	int try = 0;
	struct timeval time_serial_start = {};
	struct timeval time_serial_end = {};
	double time_serial = 0;

	struct timeval time_gps_start = {};
	struct timeval time_gps_end = {};
	double time_gps = 0;

	struct timeval time_now;

	struct TagData TxData = {};

	while (1)
	{
		if(read_gps(&gps_data, 100) != -1){
			TxData.check_gps = true;
			TxData.gps.latitude = gps_data.fix.latitude;
			TxData.gps.longitude = gps_data.fix.longitude;
			gettimeofday(&time_gps_start, NULL);
		}

		gettimeofday(&time_gps_end, NULL);
		time_gps = (time_gps_end.tv_sec - time_gps_start.tv_sec) + ((time_gps_end.tv_usec - time_gps_start.tv_usec) / 1000000); 
		
		// If gps data is not received for 5 seconds, Reset gps data
		if (time_gps >= 5)
		{
			TxData.check_gps = false;
			TxData.gps.latitude = 0;
			TxData.gps.longitude = 0;
		}

		
		// Read UWB serial data and data update
		if (read_serial(&ttyfd, uwb_msg, sizeof(uwb_msg)) > 0)
		{
			TxData.check_uwb = true;
			TxData.uwb_dist = atof(strtok(uwb_msg, "\n")) / 100; // 단위 변환 cm -> m
			gettimeofday(&time_serial_start, NULL);
		}

		gettimeofday(&time_serial_end, NULL);
		time_serial = (time_serial_end.tv_sec - time_serial_start.tv_sec) + ((time_serial_end.tv_usec - time_serial_start.tv_usec) / 1000000);

		// If uwb data is not received for 5 seconds, Reset uwb data
		if (time_serial >= 5)
		{
			TxData.check_uwb = false;
			TxData.uwb_dist = 0;
		}


		gettimeofday(&time_now, NULL);
		TxData.time = time_now.tv_sec + time_now.tv_usec / 1000000.0;
		
		sendto(sock_data.clnt_sock, (struct TagData*)&TxData, sizeof(TxData), 0, (struct sockaddr*)&sock_data.serv_addr, sizeof(sock_data.serv_addr)); //UDP
		// write(sock_data.clnt_sock, (struct ClientMsg*)&TxData, sizeof(TxData)); //TCP


		strLen = recvfrom(sock_data.clnt_sock, (struct ServerData*)&RxData, sizeof(RxData), 0, (struct sockaddr*)&sock_data.serv_addr, &sock_data.serv_addr_size);
	    // strLen = read(sock_data.clnt_sock, (struct ServerMsg*)&RxData, sizeof(RxData)); //TCP


		// Check socket communication state and store data
	    if(strLen == -1)
            sock_state_msg = "ERROR";
        else if(strLen == 0)
			sock_state_msg = "NO MESSAGE";
        else
        {
			RxData_storage = RxData;
			sock_state_msg = "SUCCESS";

			if (RxData.check_alarm){
				switch (RxData.alarm){
					case ALARM_NO:
						alarm_state_msg = "-"; break;
					case ALARM_CLOSE:
						alarm_state_msg = "CLOSE"; break;
					case ALARM_CAREFULL:
						alarm_state_msg = "CAREFULL"; break;
					case ALARM_DANGER:
						alarm_state_msg = "DANGER"; break;
					default:
						alarm_state_msg = "???"; break;
				}
			}
        }

		// After 5.1 seconds (recording starts) Initialize data to prepare next record
		printf("+-------------------------------------------+\n");
		printf("Try %d\n", try++);
		printf("Time   : %lf \n", TxData.time);
		printf("Socket : %s \n", sock_state_msg);
		printf("GPS    : %s [lat:%lf][lon:%lf] \n", TxData.check_gps ? "True" : "False", TxData.gps.latitude, TxData.gps.longitude);
		printf("UWB    : %s [d:%lf] \n", TxData.check_uwb ? "True" : "False", TxData.uwb_dist);
		printf("+-------------------------------------------+\n");
		printf("Time   : %lf \n", RxData_storage.time);
		printf("Dist   : %s [d:%lf] \n", RxData_storage.dist_type ? "UWB" : "GPS", RxData_storage.distance);
		printf("Alarm  : %s %s \n", RxData_storage.check_alarm ? "True" : "False", alarm_state_msg);
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

void setup_gps(struct gps_data_t *gps_data)
{
    int rc;
    if ((rc = gps_open("localhost", "2947", gps_data)) == -1)
    {
        printf("[ERROR] GPS Open [code: %d][reason: %s]\n", rc, gps_errstr(rc));
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
		if (gps_read(gps_data, msg, sizeof(msg)) != -1)
		{
			/* Display data from the GPS receiver. */
			if (((gps_data->fix.status == STATUS_FIX) || (gps_data->fix.status == STATUS_NO_FIX)) &&
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
	int result = 0;
	
	if(res < 0)
	{
		// printf("[ERROR] Read Seiral\n");
		result = -1;
	}
	else if(res == 0)
	{
		// printf("[WARN] No Serial Data\n");
		result = 0;
	}
	else
	{
		// printf("[SUCCESS] Distance : %d\n", distance);
		result = 1;
	}

	return result;
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
