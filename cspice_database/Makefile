all:
	gcc -c -ansi -I /home/firephyz/Downloads/cspice/include spice.c -o spice.o
	gcc -L /home/firephyz/Downloads/cspice/lib spice.o -l:cspice.a -lm -o run
