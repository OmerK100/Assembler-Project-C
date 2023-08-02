run.c: assembler.o pre_assembler.o first_run.o second_run.o output.o utility_funcs.o
	gcc -g assembler.o pre_assembler.o first_run.o second_run.o output.o utility_funcs.o -o assembler
assembler.o: assembler.c data.h
	gcc -c -Wall -ansi -pedantic assembler.c -o assembler.o
pre_assembler.o: pre_assembler.c data.h
	gcc -c -Wall -ansi -pedantic pre_assembler.c -o pre_assembler.o
first_run.o: first_run.c data.h
	gcc -c -Wall -ansi -pedantic first_run.c -o first_run.o
second_run.o: second_run.c data.h
	gcc -c -Wall -ansi -pedantic second_run.c -o second_run.o
output.o: output.c data.h
	gcc -c -Wall -ansi -pedantic output.c -o output.o
utility_funcs.o: utility_funcs.c data.h
	gcc -c -Wall -ansi -pedantic utility_funcs.c -o utility_funcs.o
