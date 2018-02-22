CFLAGS=-std=c99
OBJS=display.o

all: display.o
	gcc $(OBJS) -o run

display.o:
	gcc -c $(CFLAGS) display.c -o display.o
