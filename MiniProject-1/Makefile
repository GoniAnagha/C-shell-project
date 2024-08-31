main: codes/*.c
	gcc codes/*.c -o main

output: main.o commands.o hop.o input.o log.o proclore.o reveal.o seek.o
	gcc main.o commands.o hop.o input.o log.o proclore.o reveal.o seek.o

main.o: main.c
	gcc -c main.c

commands.o:commands.c
	gcc -c commands.c

hop.o: hop.c
	gcc -c hop.c

input.o: input.c
	gcc -c input.c

log.o: log.c
	gcc -c log.c

proclore.o: proclore.c
	gcc -c proclore.c

reveal.o: reveal.c
	gcc -c reveal.c

seek.o: seek.c
	gcc -c seek.c