#include <sys/time.h>		/* for gettimeofday() */
#include <unistd.h>		/* for gettimeofday() and fork() */
#include <stdio.h>		/* for printf() */
#include <sys/types.h>		/* for wait() */
#include <sys/wait.h>		/* for wait() */
#include <stdlib.h>
#include <string.h>


int main( int argc, char *argv[] )
{

	struct timeval start_time;
	struct timeval stop_time;

     	float drift;
      	int count = 3;
      	int child_no;

	int sleep_time = atoi(argv[1]);
	int no_iterations = atoi(argv[2]);
	int micro_second = atoi(argv[3]);
	printf("%d", micro_second);

	child_no = count;

	gettimeofday( &start_time, NULL );

			
	for( count = 0; count < no_iterations; count++ ) {
		usleep(sleep_time);
	}

		
	gettimeofday( &stop_time, NULL );

	drift = (float)(stop_time.tv_sec  - start_time.tv_sec);
	drift += (stop_time.tv_usec - start_time.tv_usec)/(float)micro_second;
		
	printf("Filho #%d -- desvio total: %.8f -- desvio medio: %.8f\n",
		child_no, drift - no_iterations*sleep_time/micro_second,
		(drift - no_iterations*sleep_time/micro_second)/no_iterations);
		
	exit(0);
}
