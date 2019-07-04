#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <linux/can/raw.h>

#include <sys/types.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#include <pthread.h>

#include <time.h>

#include <sys/mman.h>
#include <sys/wait.h>

#define TRUE    1
#define PORT    12000 
#define MAXLINE 1024 

int main(int argc, char *argv[]) {
	if (argc != 1) printf("Usage: ./ecu <traffic rate>\n");

	// Setup CAN Client
	struct sockaddr_can addr;
	struct ifreq ifr;

	int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);

	strcpy(ifr.ifr_name, "vcan0");
	ioctl(s, SIOCGIFINDEX, &ifr);

	addr.can_family  = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	bind(s, (struct sockaddr *)&addr, sizeof(addr));

	// Setup UDP Server

	int sockfd; 
    char buffer[MAXLINE]; 
    struct sockaddr_in servaddr; 
      
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
        perror("Socket creation failed"); 
        exit(EXIT_FAILURE); 
    }
    
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    servaddr.sin_family      = AF_INET; //IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port        = htons(PORT); 
      
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }

	// Create Traffic and Latency Calc Processes
	pid_t pid = fork();
	
	if (pid == 0) {
	    // Traffic / spam process

		struct can_frame frame;

		frame.can_id  = 0x1F4; //500
		frame.can_dlc = 2;
		frame.data[0] = 0;
		frame.data[1] = 1;

		while (TRUE) {
			break;
			int nbytes = write(s, &frame, sizeof(struct can_frame));

			if (nbytes < 0) {
				perror("write");
				return 1;
			}
		}
	} else {
	    // Latency calculation process
	    int *start_time = mmap(NULL, sizeof *start_time, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	    pid_t pid = fork();

	    if (pid == 0) {
	    	sleep(4);

		    // Send CAN packet
			struct can_frame frame;

			frame.can_id  = 0x1F5; //501
			frame.can_dlc = 2;
			frame.data[0] = 0;
			frame.data[1] = 1;

			*start_time  = (int)time(NULL);

			
			int nbytes = write(s, &frame, sizeof(struct can_frame));


			if (nbytes < 0) {
				perror("write");
				return 1;
			}

	    } else {
	    	// Receive UDP Frame

		    while (TRUE) {

			    int len, n; 
			    
			    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &servaddr, &len); 
			    buffer[n] = '\0'; 

			    char channel[3];
			    
			    strncpy(channel, buffer + 25, 4);
			    
			    printf("Buffer: %s\n", buffer);
			    printf("Channel: %s\n", channel);

			    if (strcmp(channel, "2041") == 0) {
			    	int end_time = (int)time(NULL);

			    	printf("start_time: %d\n", *start_time);
	        		munmap(start_time, sizeof *start_time);
			    	
			    	//printf("%d\n", start_time);
			    	printf("end time: %d\n", end_time);
			    	break;
			    }

			}
	    }


	}

	return 0;
}