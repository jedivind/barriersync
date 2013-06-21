#include <stdio.h>
#include "mpi.h"
#include <stdlib.h>
#define NUM_BARRIERS 1

void centralized_barrier( int *rank, int *numprocs, int *sense ) {
	int done = 1, i;
  
	if( *rank != *(numprocs)-1 ){
		MPI_Send( &done, 1, MPI_INT, *(numprocs)-1, 1 , MPI_COMM_WORLD );
	}
  
	if( *rank == *(numprocs)-1 ){
		for( i=0 ; i<*(numprocs)-1; i++ )
			MPI_Recv( &done, 1 ,MPI_INT, i, 1, MPI_COMM_WORLD, NULL );
	}

	if( *rank == *(numprocs)-1 )*sense = !*sense;
	MPI_Bcast(sense, 1, MPI_INT, *(numprocs)-1, MPI_COMM_WORLD);

	while(!*sense);
	*sense =!*sense;
}

int main(int argc, char **argv) {

	int i, rank, numprocs;
	double time1, time2, time;
	int sense =0;
	MPI_Init(&argc, &argv);

	MPI_Comm_size( MPI_COMM_WORLD, &numprocs );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );

	for( i=0; i<NUM_BARRIERS; i++ ){
		time1 = MPI_Wtime();
		//MPI_Barrier(MPI_COMM_WORLD);
		centralized_barrier(&rank, &numprocs, &sense);
		time2 = MPI_Wtime();
		time += (time2 - time1);
		printf("Proc %d reached barrier %d\n", rank, i);
	}
	printf("Time spent in barrier by process %d is %f\n", rank, time/NUM_BARRIERS);
	MPI_Finalize();
	return 0;
}

