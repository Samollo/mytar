CC=gcc
CFLAGS = -g -Wall
LDFLAGS = -lbsd

all: mytar

mytar:mytar.o
	$(CC) $(CFLAGS) mytar.o -o mytar $(LDFLAGS)

clean:
	rm -rf *.o mytar
