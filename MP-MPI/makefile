OMPLIBS = -lgomp

CC = gcc
CPPFLAGS = -g 
LDFLAGS = -g 
LDLIBS = $(OMPLIBS)

MPICC = mpicc
MPICH = /usr/lib64/openmpi/1.4-gcc
CFLAGS = -I$(MPICH)/include

all: tournament centralized cedis

tournament: tournament.o
	  $(MPICC) -o $@ $(LDFLAGS) $^ $(LDLIBS)

centralized: centralized.o
	 $(MPICC) -o $@ $(LDFLAGS) $^ $(LDLIBS)

cedis: cedis.o        
	$(MPICC) -o $@ $(LDFLAGS) $^ $(LDLIBS) 

clean:rm -f *.o tournament centralompi cedis
