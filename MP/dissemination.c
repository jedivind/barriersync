#include <stdio.h>
#include <stdlib.h>
#include "omp.h"
#include <math.h>
#include <stdint.h>
#define MAXTHREADS 10000
//#define P 10 //numthreads
//#define proc ceil(log2(P))
int NUM_THREADS, NUM_BARRIERS;


typedef struct flags
{
int myflags[2][MAXTHREADS];
int *partnerflags[2][MAXTHREADS];
}flags;



void dissemination_barrier(flags *localflags, int *sense, int *parity, int *proc)
{
int p = *parity, i;

for(i=0; i<*proc; i++)
{
#pragma omp critical
{
*localflags->partnerflags[p][i] = *sense;
}
while(localflags->myflags[p][i] != *sense){}
}

if(*parity == 1)
*sense = !*sense;
*parity = 1 - *parity;
}


int main(int argc, char **argv)
{

	if(argc==3)
	{
	NUM_THREADS = atoi(argv[1]);
	NUM_BARRIERS = atoi(argv[2]);
	}
	
	else{
        printf("Syntax:\n./dissemination num_threads num_barriers\n");
        exit(-1);
        }



double time1, time2;
flags allnodes[NUM_THREADS]; //shared
omp_set_num_threads(NUM_THREADS);
int proc = ceil(log(NUM_THREADS)/log(2));
#pragma omp parallel shared(allnodes, proc)
{
int thread_num = omp_get_thread_num();
int numthreads = omp_get_num_threads();

int i,r, k, j,x;
int parity = 0; //processor private
int sense = 1; //processor private
flags *localflags = &allnodes[thread_num]; //processor private
int temp, y;

#pragma omp critical
for(x=0; x<NUM_THREADS; x++)
for(r=0; r<2; r++)
for(k=0; k<proc; k++)
allnodes[x].myflags[r][k] = 0;

for(i=0; i<NUM_BARRIERS; i++){
printf("Hello world from thread %d of %d\n", thread_num, numthreads);


//time1 = omp_get_wtime();
#pragma omp critical
for(j=0; j<NUM_THREADS; j++)
for(k=0; k<proc; k++){temp=ceil(pow(2,k));
if(j==(thread_num+temp)%NUM_THREADS)
{
allnodes[thread_num].partnerflags[0][k] =  &allnodes[j].myflags[0][k];
allnodes[thread_num].partnerflags[1][k] =  &allnodes[j].myflags[1][k];
}}


time1 = omp_get_wtime();
dissemination_barrier(localflags, &sense, &parity, &proc);
time2 = omp_get_wtime();

printf("Hello world from thread %d of %d after barrier\n", thread_num, numthreads);

}
printf("Time spent in barrier by thread %d is %f\n", thread_num, time2-time1);
}

}
