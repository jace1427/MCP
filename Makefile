flags = -g -W -Wall

all: iobound cpubound part1 part2 part3 part4

part1 : part1.c
	gcc $(flags) -o part1 part1.c string_parser.c

part2 : part2.c
	gcc $(flags) -o part2 part2.c string_parser.c

part3 : part3.c
	gcc $(flags) -o part3 part3.c string_parser.c

part4 : part4.c
	gcc $(flags) -o part4 part4.c string_parser.c

clean:
	rm part1 part2 part3 part4
