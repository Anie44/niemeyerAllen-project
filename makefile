CC = gcc
CFLAGS = -Wall

all: manager

manager: manager.c
	$(CC) $(CFLAGS) -o a.out manager.c

clean:
	rm -f a.out