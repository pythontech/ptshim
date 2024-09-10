all:	libptshim.so

libptshim.so:	ptshim.o ptlog.o
	$(CC) -g -shared -o $@ ptshim.o ptlog.o

libptsocket.so:	socket.o ptshim.o ptlog.o
	$(CC) -g -shared -o $@ $< ptshim.o ptlog.o

%.o:	%.c
	$(CC) -g -c -fpic -Wall -o $@ $<

ptshim.o:	ptshim.h
ptlog.o:	ptlog.h
socket.o:	ptshim.h ptlog.h
