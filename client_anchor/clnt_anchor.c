#include "clnt_anchor.h"

int main(int argc, char *argv[])
{
	// GPS
	struct gps_data_t gps_data;
	char gps_msg[BUF_MAX];

	// Socket
	struct Socket sock_data;
	struct ServerData RxData;
	struct ServerData RxData_storage;

	int get_alarm = 0;
	int alarm = ALARM_NO;
	int strLen;

	char *sock_state_msg = "-";
	char *alarm_state_msg = "-";


	if (argc != 3)
	{
		printf("Usage : %s <IP> <port>\n", argv[0]);
		return 0;
	}

	if (setup_socket(&sock_data, argv) < 0)
		error_handling("[ERROR] Setup socket");

	setup_gps(&gps_data);
	

	int try = 0;
	struct timeval time_now;

	char record_cmd[BUF_MAX];
	char video_name[BUF_MAX];

	char transfer_cmd[BUF_MAX];

	struct timeval time_record_start = {};
	struct timeval time_record_end = {};
	double time_record = 0;
	bool record_trigger = false;



	struct timeval time_gps_start = {};
	struct timeval time_gps_end = {};
	double time_gps = 0;

	struct AnchorData TxData = {};

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

		if (time_gps >= 5)
		{
			TxData.check_gps = false;
			TxData.gps.latitude = 0;
			TxData.gps.longitude = 0;
		}

		// 현재 시간 Tx 에 저장
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

		// If Socket communication succeed & the data is "ALARM_CLOSE", Camera record start
		if (sock_state_msg == "SUCCESS")
		{
			if (RxData_storage.check_alarm && RxData_storage.alarm < ALARM_NO)
			{
				if (!record_trigger)
				{
					record_trigger = true;
					generate_record_cmd(record_cmd, video_name);
					system(record_cmd);
					gettimeofday(&time_record_start, NULL);
				}
			}
		}

		// After 5.1 seconds (recording starts) Initialize data to prepare next record 
		if (record_trigger)
		{
			gettimeofday(&time_record_end, NULL);
			time_record = time_record_end.tv_sec + time_record_end.tv_usec / 1000000.0 - time_record_start.tv_sec + time_record_start.tv_usec / 1000000.0;
			if (time_record >= 5.1)
			{
				record_trigger = false;

				memset(record_cmd, 0, sizeof(record_cmd));
				memset(video_name, 0, sizeof(video_name));
				
				strcpy(TxData.video_name, video_name);
			}
		}

		// Show transmit data & receive data 
		// If there is no receive data, show latest receive data
		printf("+-------------------------------------------+\n");
		printf("Try %d\n", try++);
		printf("Time   : %lf \n", TxData.time);
		printf("Socket : %s \n", sock_state_msg);
		printf("GPS    : %s [lat:%lf][lon:%lf] \n", TxData.check_gps ? "True" : "False", TxData.gps.latitude, TxData.gps.longitude);
		printf("Video  : %s \n", TxData.video_name);
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

// Based on terminal command, Start recording the raspberry pi camera
void generate_record_cmd(char *command, char *file_name)
{
	time_t timer;
    struct tm* t;
    
    timer = time(NULL);
    t = localtime(&timer);

    struct TIME_VAL tm;

    char cmd[BUF_MAX] = "sudo libcamera-vid -t 5000 -n -o /home/pi/Videos/";
	char f_name[BUF_MAX] = "";
    
    sprintf(tm.year, "%d", t->tm_year + 1900);
    sprintf(tm.month, "%d", t->tm_mon + 1);
    sprintf(tm.day, "%d", t->tm_mday);
    sprintf(tm.hour, "%d", t->tm_hour);
    sprintf(tm.min, "%d", t->tm_min);
    sprintf(tm.sec, "%d", t->tm_sec);

	strcat(f_name, tm.year);
    strcat(f_name, "-");
    strcat(f_name, tm.month);
    strcat(f_name, "-");
    strcat(f_name, tm.day);
    strcat(f_name, "_");
    strcat(f_name, tm.hour);
    strcat(f_name, ":");
    strcat(f_name, tm.min);
    strcat(f_name, ":");
    strcat(f_name, tm.sec);
    strcat(f_name, ".h264");
	
	strcat(command, cmd);
	strcat(command, f_name);
	strcat(command, " &");

	strcat(file_name, f_name);
}
