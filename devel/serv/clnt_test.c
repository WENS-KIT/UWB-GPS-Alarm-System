#include "clnt_test.h"

int main(int argc, char *argv[])
{
	// Socket
	struct Socket sock_data;

    int strLen;

    if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
        return 0;
	}

	if (setup_socket(&sock_data, argv) < 0)
		error_handling("[ERROR] Setup Socket");

	
	int try = 0;

	while (try < 1)
	{
		printf("Try %d\n", try++);
		
		struct ServerData RxData = {};
		struct TagData TxData = {};
		struct AnchorData test = {};

		struct timeval time_now = {};
		gettimeofday(&time_now, NULL);
		TxData.time = time_now.tv_sec + time_now.tv_usec / 1000000.0;

		// printf("timenow : %lf \n", TxData.time);

		TxData.time =987654321;
		TxData.check_gps = 1;
		TxData.check_uwb = 1;
		TxData.gps.latitude = 123.456789;
		TxData.gps.longitude = 1234.56789;
		TxData.uwb_dist = 10.0;

 		strcpy(TxData.video_name, "2022-07-08_10:11:46.h264");	

		printf("%s\n", TxData.video_name);

		sendto(sock_data.clnt_sock, (struct TagData*)&TxData, sizeof(TxData), 0, (struct sockaddr*)&sock_data.serv_addr, sizeof(sock_data.serv_addr)); //UDP
		// // sendto(sock_data.clnt_sock, (struct AnchorData*)&test, sizeof(test), 0, (struct sockaddr*)&sock_data.serv_addr, sizeof(sock_data.serv_addr)); //UDP
		
		
		// printf("send message\n");

		// strLen = recvfrom(sock_data.clnt_sock, (struct ServerData*)&RxData, sizeof(RxData), 0, (struct sockaddr*)&sock_data.serv_addr, &sock_data.serv_addr_size);
	    
		// printf("strLen = %d\n", strLen);

		// printf("Time : %lf\n", RxData.time);
		// printf("alarm  : %s %d\n", RxData.check_alarm ? "True" : "False", RxData.alarm);
		// printf("dist   : %s %lf\n", RxData.dist_type ? "UWB" : "GPS", RxData.distance);

		// char hi[] = "1234";
		// printf("Test : %s\n", hi);
	
	}

	close(sock_data.clnt_sock);
	close(sock_data.serv_sock);
	return 0;
}


void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}


int setup_socket(struct Socket *sock_data, char *argv[])
{
	sock_data->clnt_sock = socket(PF_INET, SOCK_DGRAM, 0); // UDP

	if (sock_data->clnt_sock == -1){
        printf("[ERROR] Create Client Socket Fail\n");
		return FAIL_CLNTSOCK;
    }


	memset(&sock_data->serv_addr, 0, sizeof(sock_data->serv_addr));
	sock_data->serv_addr.sin_family = AF_INET;
	sock_data->serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	sock_data->serv_addr.sin_port = htons(atoi(argv[2]));

	sock_data->serv_addr_size = sizeof(sock_data->serv_addr);

	return 0;
}
