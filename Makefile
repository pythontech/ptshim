all:	libptsocket.so

libptsocket.so:	socket.o ptshim.o
	$(CC) -shared -o $@ $< ptshim.o

%.o:	%.c
	$(CC) -c -fpic -Wall -o $@ $<
