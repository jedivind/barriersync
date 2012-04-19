OMPFLAGS = -fopenmp 
OMPLIBS = -lgomp -lm

CC = gcc
CPPFLAGS = -g $(OMPFLAGS)
LDFLAGS = -g $(OMPFLAGS)
LDLIBS = $(OMPLIBS)

all: tournament dissemination centralized

tournament: tournament.o
	        $(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

dissemination: dissemination.o
	        $(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

centralized: centralized.o
	        $(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean: rm -f *.o tournament dissemination centralized
