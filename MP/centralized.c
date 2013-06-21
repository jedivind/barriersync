#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

//#define NUM_THREADS 8
int NUM_THREADS, NUM_BARRIERS;

static void centralized_barrier( int *count, int *sense, int *local_sense ){
	for( int i = 0; i < 5000; i++ );
	*local_sense = ! *local_sense;

	#pragma omp critical{
		*count = (*count) - 1;
	}

	if( *count == 0 ){
        *count = NUM_THREADS;
        *sense = *local_sense;
	}

	else {
       	while( *sense != *local_sense ) {}
	}
}



int main(int argc, char **argv)
{
  if( argc == 3 ){
	  NUM_THREADS = atoi( argv[1] );
	  NUM_BARRIERS = atoi( argv[2] );
  }

  else{ 
	  printf("Syntax:\n./centralized num_threads num_barriers\n");
	  exit(-1);
  }

  // Serial code
  printf("This is the serial section\n");
  omp_set_num_threads( NUM_THREADS );
  int sense = 1, count = NUM_THREADS;
  double time1, time2;

	#pragma omp parallel shared(sense, count) {
    // Now we're in the parallel section
  	// 	time1 = omp_get_wtime();
    	int num_threads = omp_get_num_threads();
    	int thread_num = omp_get_thread_num();
    	int local_sense = 1;
    	long i,j;

    	for(j=0; j<NUM_BARRIERS; j++){
    		for(i=0; i<50; i++);
    		printf("Hello World from thread %d of %d.\n", thread_num, num_threads);
    
    		time1 = omp_get_wtime();
    		//Centralized barrier
    		centralized_barrier(&count, &sense, &local_sense);
    		time2 = omp_get_wtime();
    		//Centralized barrier reached by all threads. Continue.
	
    		for(i=0; i<50; i++);
    		printf("Hello World from thread %d of %d after barrier\n", thread_num, num_threads);
    	}

    	printf("Time spent in barrier by thread %d is %f\n", thread_num, time2 - time1);
	}
//printf("The number of processors on this system is %d", omp_get_numprocs());
  
// Resume serial code
  printf("Back in the serial section again\n");
  return 0;
}

