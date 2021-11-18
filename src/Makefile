CC = gcc 
CFLAGS = -g -Wall
OBJECTS = manufacturingDaemon.o locs.o changes.o backup.o transfers.o queue.o
HEADERS = manufacturingDaemon.c locs.h changes.h backup.h transfers.h queue.h

manufacturingDaemon : $(OBJECTS)
	$(CC) $(CLAGS) -o manufacturingDaemon $(OBJECTS) -lm -lrt

manufacturingDaemon.o : manufacturingDaemon.c $(HEADERS)
	$(CC) $(CFLAGS) -c manufacturingDaemon.c -lrt
								
locs.o : locs.c
	$(CC) $(CFLAGS) -c locs.c -lm

changes.o : changes.c
	$(CC) $(CFLAGS) -c changes.c -lm
						
backup.o : backup.c
	$(CC) $(CFLAGS) -c backup.c -lm

transfers.o : transfers.c
	$(CC) $(CFLAGS) -c transfers.c -lm
				
queue.o : queue.c
	$(CC) $(CFLAGS) -c queue.c -lm

clean:
	rm  manufacturingDaemon $(OBJECTS)
