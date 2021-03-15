#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/poll.h>
#include <termios.h>

#include <time.h>

#define BUF_SIZE 1024

struct termios newtio;
char read_buf[BUF_SIZE];

char device[] = "/dev/ttyTHS2";

int main() {
	int read_num;
	int nRead;
	int gps_port;
	struct pollfd poll_events[1];

	char *token;
	char *_token;
	char *rest;
	char *_rest;

	struct timespec spec;


	gps_port = open(device, O_RDWR);
	if (gps_port < 0) {
		printf("Error %i from open gps port : %s\n", errno, strerror(errno));
		return -1;
	}
	else 
		printf("Succese open %s\n", device);

	bzero(&newtio, sizeof(newtio));

	newtio.c_cflag = B9600 | CLOCAL | CREAD | CS8;

	tcflush(gps_port, TCIFLUSH);
	tcsetattr(gps_port, TCSANOW, &newtio);
	fcntl(gps_port, F_SETFL, FNDELAY);


	poll_events[0].fd = gps_port;
	poll_events[0].events = POLLIN;
	poll_events[0].revents = 0;


	while (gps_port > -1) {
		nRead = poll((struct pollfd *)&poll_events, 1, 1000);
		if (nRead >0) {
			if (poll_events[0].revents & POLLIN) {

				clock_gettime(CLOCK_REALTIME, &spec);
				printf("%ld:%ld ", spec.tv_sec, spec.tv_nsec);

				read_num = read(poll_events[0].fd, read_buf, BUF_SIZE-1);
				read_buf[read_num] = '\0';

//				printf("read %d bytes:\n%s\n\n", read_num, read_buf);

/*
$--RMC,hhmmss.sss,x,llll.lll,a,yyyyy.yyy,a,x.x,u.u,xxxxxx,,,v*hh<CR><LF>
Field 		Name 			Description
hhmmss.sss 	UTC time 		UTC time in hhmmss.sss format (000000.000 ~ 235959.999)
x 		Status 			Status
					‘V’ = Navigation receiver warning
					‘A’ = Data Valid
llll.lll 	Latitude 		Latitude in dddmm.mmmm format. Leading zeros are inserted.
A 		N/S indicator 		‘N’ = North; ‘S’ = South
yyyyy.yyy 	Longitude	 	Longitude in dddmm.mmmm format. Leading zeros are inserted.
A 		E/W Indicator 		‘E’ = East; ‘W’ = West
x.x 		Speed over ground 	Speed over ground in knots (000.0 ~ 999.9)
u.u 		Course over ground 	Course over ground in degrees (000.0 ~ 359.9)
xxxxxx 		UTC Date 		UTC date of position fix, ddmmyy format
v 		Mode indicator 		Mode indicator
					‘N’ = Data not valid
					‘A’ = Autonomous mode
					‘D’ = Differential mode
					‘E’ = Estimated (dead reckoning) mode 
*/

				rest = read_buf;
				while ((token = strtok_r(rest, "\n", &rest))) {
					_rest = token;
					_token = strtok_r(_rest, ",", &_rest);

					if (strcmp(_token, "$GNRMC") == 0) {
						printf("GNRMC : ");
						while ((_token = strtok_r(_rest, ",", &_rest))) 
								printf("%s ", _token);
						printf("\n");
					}


				}

					


			}
			else if(poll_events[0].revents & POLLERR || poll_events[0].revents & POLLHUP) {
				//perror("Serial Port: an error occur.\n");
				continue;
			}
		}
		else {
			continue;
		}
	}
/*

	while(1) {
		sleep(1);
		read(gps_port, read_buf, BUF_SIZE-1);
		printf("read %d bytes:\n%s\n\n", read_num, read_buf);


	}
*/

	close(gps_port);

}
