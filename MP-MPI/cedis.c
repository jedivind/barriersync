#include <stdio.h>
#include <stdlib.h>
#include "omp.h"
#include "mpi.h"
#include <math.h>
#include <stdint.h>
#define NUM_BARRIERS 100
#define MAXTHREADS 10000
#define P 2 //numthreads

typedef struct flags {
	int myflags[2][MAXTHREADS];
	int *partnerflags[2][MAXTHREADS];
}flags;


void centralized_barrier(int *rank, int *numprocs, int *sense){
	int done = 1, i;
 
	if( *rank != *(numprocs)-1 ){
		MPI_Send(&done, 1, MPI_INT, *(numprocs)-1, 1 , MPI_COMM_WORLD);
	}
 
	if( *rank == *(numprocs)-1 ){
		for(i=0 ; i<*(numprocs)-1; i++)
			MPI_Recv(&done,1,MPI_INT, i, 1, MPI_COMM_WORLD, NULL);
	}

	if( *rank == *(numprocs)-1 )
		*sense = !*sense;
	MPI_Bcast(sense, 1, MPI_INT, *(numprocs)-1, MPI_COMM_WORLD);
 
	while(!*sense);
	*sense =!*sense;
}

void dissemination_barrier(flags *localflags, int *sense, int *parity, int *proc) {
	int p = *parity, i;

	for( i=0; i<*proc; i++ ) {
		#pragma omp critical {
		*localflags->partnerflags[p][i] = *sense;
		}

		while(localflags->myflags[p][i] != *sense) {}
	}

	if(*parity == 1)
		*sense = !*sense;
		*parity = 1 - *parity;
}


int main(int argc, char **argv) {
	//MPI PART - INITIALIZATIONS
	int i, rank, numprocs;
	double time3, time4, timemp=0, timempi=0;
	int psense =0;
	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	//MPI INITIALIZATIONS END

	//MP PART///////////////////////////////////////////////////////
	double time1, time2;
	flags allnodes[P]; //shared
	omp_set_num_threads(P);
	int proc = ceil(log(P)/log(2));
	int x;

	#pragma omp parallel shared(allnodes, proc) {
		int thread_num = omp_get_thread_num();
		int numthreads = omp_get_num_threads();

		int i,r, k, j,x;
		int parity = 0; //processor private
		int sense = 1; //processor private
		flags *localflags = &allnodes[thread_num]; //processor private
		int temp, y;

		#pragma omp critical
		for( x=0; x<P; x++ )
			for( r=0; r<2; r++ )
				for( k=0; k<proc; k++ )
					allnodes[x].myflags[r][k] = 0;

		for( y=0; y<NUM_BARRIERS; y++ ){ //FOR LOOP FOR MP BARRIER BEGINS
			printf("Hello world from thread %d of %d in proc %d\n", thread_num, numthreads, rank);

			//time1 = omp_get_wtime();
			#pragma omp critical
				for( j=0; j<P; j++ )
					for( k=0; k<proc; k++){
						temp = ceil( pow(2,k) );
						if( j==(thread_num+temp)%P ) {
							allnodes[thread_num].partnerflags[0][k] =  &allnodes[j].myflags[0][k];
							allnodes[thread_num].partnerflags[1][k] =  &allnodes[j].myflags[1][k];
						}
					}

				for( x=0; x<NUM_BARRIERS; x++ ){
					time1 = omp_get_wtime();
					//#pragma omp barrier
					dissemination_barrier(localflags, &sense, &parity, &proc);
					time2 = omp_get_wtime();
					timemp = timemp + (time2-time1);
				}
				printf("Hello world from thread %d of %d in proc %d after barrier\n", thread_num, numthreads, rank);

		} //FOR LOOP FOR MP BARRIER ENDS
		printf("Time spent in barrier by thread %d is %f\n", thread_num, time2-time1);
	}
	printf("Average time spent by threads in mp barrier = %f\n", timemp/(P*NUM_BARRIERS));
	//MP PART ENDS AFTER SYNCHRONIZATION WITH ALLTHREADS IN PROCESSOR
	//MPI SYNCHRONIZATION BEGINS AFTER ALL THREADS ON EACH PROCESSOR HAVE FINISHED

	for( i=0; i<NUM_BARRIERS; i++ ){
		time3 = MPI_Wtime();
		centralized_barrier(&rank, &numprocs, &psense);
		time4 = MPI_Wtime();
		timempi += time4-time3;
		printf("Proc %d reached barrier %d\n", rank, i);
	}
	printf("Time spent in mpibarrier = %f\n", timempi/NUM_BARRIERS);
	MPI_Finalize();
	return 0;
	//MPI BARRIER REACHED
}
