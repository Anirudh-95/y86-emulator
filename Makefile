CC = gcc
CCFLAGS = -Wall -Werror -lm -g -Werror 

all : y86

y86 : y86emul.o
	$(CC) $(CCFLAGS) -o y86 y86emul.o

y86emul.o : y86emul.c
	$(CC) $(CCFLAGS) -c y86emul.c

clean: 
	rm -f y86 y86emul.o