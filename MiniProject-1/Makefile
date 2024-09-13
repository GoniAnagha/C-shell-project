main: codes/*.c
	gcc codes/*.c -o main

output: main.o commands.o hop.o input.o log.o proclore.o reveal.o seek.o pipes.o
	gcc main.o commands.o hop.o input.o log.o proclore.o reveal.o seek.o pipes.o

main.o: main.c
	gcc -c main.c

commands.o:commands.c
	gcc -c commands.c commands.h

hop.o: hop.c
	gcc -c hop.c hop.h

input.o: input.c
	gcc -c input.c input.h

log.o: log.c
	gcc -c log.c log.h

proclore.o: proclore.c
	gcc -c proclore.c proclore.h

reveal.o: reveal.c
	gcc -c reveal.c reveal.h

seek.o: seek.c
	gcc -c seek.c seek.h

pipes.o: pipes.c
	gcc -c pipes.c pipes.h