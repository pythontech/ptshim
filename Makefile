all:	libptsocket.so

libptsocket.so:	socket.o ptshim.o ptlog.o
	$(CC) -g -shared -o $@ $< ptshim.o ptlog.o

%.o:	%.c
	$(CC) -g -c -fpic -Wall -o $@ $<
