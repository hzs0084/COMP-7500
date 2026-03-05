CC=gcc
CFLAGS=-Wall -Wextra -O2 -pthread

OBJS=aubatch.o queue.o cmd_parser.o scheduler.o dispatcher.o

all: aubatch

aubatch: $(OBJS)
	$(CC) $(CFLAGS) -o aubatch $(OBJS)

aubatch.o: aubatch.c aubatch.h queue.h scheduler.h dispatcher.h
	$(CC) $(CFLAGS) -c aubatch.c

queue.o: queue.c queue.h job.h
	$(CC) $(CFLAGS) -c queue.c

cmd_parser.o: cmd_parser.c cmd_parser.h
	$(CC) $(CFLAGS) -c cmd_parser.c

scheduler.o: scheduler.c scheduler.h aubatch.h cmd_parser.h
	$(CC) $(CFLAGS) -c scheduler.c

dispatcher.o: dispatcher.c dispatcher.h aubatch.h job.h
	$(CC) $(CFLAGS) -c dispatcher.c

clean:
	rm -f *.o aubatch
