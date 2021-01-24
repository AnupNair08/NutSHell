
#shell.c --> format.c      invoked via format.h
#	--> utility.c	  invoked via  utility.h

dnsh: shell.o  command.o jobs.o stack.o
	gcc shell.o command.o jobs.o stack.o -o dnsh

jobs.o: jobs.c shell.h
	gcc -c jobs.c

shell.o: shell.c 
	gcc -c shell.c

command.o: command.c shell.h
	gcc -c command.c


stack.o: stack.c shell.h
	gcc -c stack.c

clean:
	@rm *.o

# Preceding comments with an @ will suppress the echoing of the commands 
install:
	@cp dnsh /bin/ > /dev/null 2>&1
remove:
	@rm /bin/dnsh

docs:
	doxygen Doxyfile

