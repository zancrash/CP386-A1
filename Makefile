# Do not edit the contents of this file.
all: warnings_OK z_creator z_terminator

warnings_BAD: process_management2.c
	gcc -Werror -Wall -g -std=gnu99 -o process_management process_management2.c -lrt

warnings_OK: process_management2.c
	gcc -Wall -g -std=gnu99 -o process_management process_management2.c -lrt

z_creator: z_creator.c
	gcc -Wall -g -std=gnu99  -o z_creator z_creator.c

z_terminator: z_terminator.c
	gcc -Wall -g -std=gnu99  -o z_terminator z_terminator.c

runq2: warnings_OK
	./process_management sample_in.txt
runq1: z_creator z_terminator
	./z_terminator
# sample_in.txt | cat output.txt
clean: *.c
	rm -f process_management z_creator z_terminator output.txt
