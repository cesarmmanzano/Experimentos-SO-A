
#include <sys/time.h>		/* for gettimeofday() */
#include <unistd.h>		/* for gettimeofday() and fork() */
#include <stdio.h>		/* for printf() */
#include <sys/types.h>		/* for wait() */
#include <sys/wait.h>		/* for wait() */
#include <stdlib.h>
#include <string.h>

#define NO_OF_ITERATIONS	1000


#define NO_OF_CHILDREN	3


#define SLEEP_TIME 1000



#define MICRO_PER_SECOND	1000000


int main( int argc, char *argv[] )
{


	struct timeval start_time;
	struct timeval stop_time;
    int temp_sleep = atoi (argv[0]);
          usleep(temp_sleep);
     

      float drift;
      int count;
      int child_no;
    //child_no++;
    //printf("%s", *argv);
	//child_no = 3;

	gettimeofday( &start_time, NULL );

			
	for( count = 0; count < NO_OF_ITERATIONS; count++ ) {
      usleep(temp_sleep);

	}

		
	gettimeofday( &stop_time, NULL );

	drift = (float)(stop_time.tv_sec  - start_time.tv_sec);
	drift += (stop_time.tv_usec - start_time.tv_usec)/(float)MICRO_PER_SECOND;
		
	printf("Filho #%d -- desvio total: %.8f -- desvio medio: %.8f\n",
		child_no, drift - NO_OF_ITERATIONS*temp_sleep/MICRO_PER_SECOND,
		(drift - NO_OF_ITERATIONS*temp_sleep/MICRO_PER_SECOND)/NO_OF_ITERATIONS);
		
	exit(0);
}
