#include <stdio.h>
#include "mpi.h"

#include <omp.h>
#include <math.h>
#include<stdlib.h>
#include <malloc.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#define rounds_first ceil(log(numprocessors)/log(2))
#define rounds1 rounds_first
#define winner 0
#define loser 1
#define bye 2
#define champion 3
#define dropout 4
#define NUM_BARRIERS 1

struct round_struct {
	int role;
	int vpid;
	int tb_round;
	// struct round_struct* opponent;
	int opponent;
};
int numprocessors;

void barrier( struct round_struct array[numprocessors][10], int rank, int rounds ) {
	int round=1, tag=1, my_msg=1;
	int i;
	MPI_Status *status;

	while(1) {
		if( array[rank][round].role == winner ) {
			int opponent_rank_loser_to_wait_on = array[rank][round].opponent;
			MPI_Recv( &my_msg, 1, MPI_INT, opponent_rank_loser_to_wait_on, tag, MPI_COMM_WORLD,NULL );
		}

		if( array[rank][round].role == loser ) {
			int opponent_winner = array[rank][round].opponent;
			MPI_Send(&my_msg,1,MPI_INT,opponent_winner,tag,MPI_COMM_WORLD);
			MPI_Recv(&my_msg,1,MPI_INT,opponent_winner,tag,MPI_COMM_WORLD,NULL);
			break;
		}

		if(array[rank][round].role == champion) {
			int opponent_loser_winner = array[rank][round].opponent;
			MPI_Recv(&my_msg,1,MPI_INT,opponent_loser_winner,tag,MPI_COMM_WORLD,NULL);
			MPI_Send(&my_msg,1,MPI_INT,opponent_loser_winner,tag,MPI_COMM_WORLD);
			break;
		}

		if( round <= rounds )
			round = round +1;
	}

	while(1) {
		if( round > 0 )
			round = round - 1;

		if( array[rank][round].role == winner ) {
			int opponent_loser_waken = array[rank][round].opponent;
			MPI_Send(&my_msg,1,MPI_INT,opponent_loser_waken,tag,MPI_COMM_WORLD);
		}

		if( array[rank][round].role == dropout ){
			break;
		}
	}
}

int main( int argc, char **argv ) {
	double t0, t1, t_code;
	int rank, my_dst, my_src;
	int tag = 1;

	char filename[20];
	MPI_Status mpi_result;
	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD,&numprocessors);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int counter = 0;
	int rounds = ceil( log(numprocessors)/log(2) );

	struct round_struct array[numprocessors][10];
	//printf("This is the serial section\n");
	double time1, time2;
	int i, j, k, l;
	j = rank;

	for( k=0; k<=rounds; k++ ) {
		array[j][k].role = -1;
		array[j][k].opponent = -1;
	}

	i=0;
	int temp=0,temp2,g=0,comp,comp_second=0;
	l = rank;

	for( k=0; k<=rounds; k++ ) {
		temp = k;
		temp2=l;
		comp = ceil(pow(2,k));
		comp_second = ceil(pow(2,k-1));

		if((k > 0) && (l%comp==0) && ((l + (comp_second))< numprocessors) && (comp < numprocessors)){
			array[l][k].role = winner;
		}

		if((k > 0) && (l%comp == 0) && ((l + comp_second)) >= numprocessors){
			array[l][k].role = bye;
		}

		if((k > 0) && ((l%comp == comp_second))){
			array[l][k].role = loser;
		}

		if((k > 0) && (l==0) && (comp >= numprocessors)){
			//printf("\nCHAMPION\n");
			array[l][k].role = champion;
		}

		if( k==0 ) {
			array[l][k].role = dropout;
		}

		if( array[l][k].role == loser ){
			array[l][k].opponent = l - comp_second;
		}

		if(array[l][k].role == winner || array[l][k].role == champion){//comp = ceil(pow(2,k-1));
			array[l][k].opponent = l+comp_second;
		}
	}

	i = rank;
	int f; double time=0;

	MPI_Barrier(MPI_COMM_WORLD);
	// t0 = MPI_Wtime();
    for( f=0; f<NUM_BARRIERS; f++ ){
    	printf("Proc %d waiting at barrier %d\n",rank, f);

    	t0 = MPI_Wtime();
    	barrier(array,rank,rounds);
    	t1 = MPI_Wtime();
        time+=t1-t0;

        /*if (rank < numprocessors) {
        		sprintf(filename, "file_%d.out", rank );
        		fp = fopen(filename, "w");
        		fprintf(fp, "P%d: time taken by processor %d in Barrier is %f\n", rank,t1-t0);
        	fclose(fp);
   	 	 }*/
        //printf("time taken by one processor%d is %f\n\n",rank,t1-t0);
        //calculate(t1-t0);
        //printf("Proc %d after barrier %d\n\n", rank, f);
        // usleep(100000);
    }
	//t1 = MPI_Wtime();
	printf("time taken by one processor%d is %f\n\n",rank,time/NUM_BARRIERS);
	//calculate(t1-t0);
	//printf("rank of processor after barrier%d\n",rank);

	MPI_Finalize();
	return 0;
}


