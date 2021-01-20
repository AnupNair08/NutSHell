
#shell.c --> format.c      invoked via format.h
#	--> utility.c	  invoked via  utility.h

dnsh: shell.o utility.o format.o
	gcc shell.o utility.o format.o -o dnsh

shell.o: shell.c format.h utility.h
	gcc -c shell.c

format.o: format.c 
	gcc -c format.c

utility.o: utility.c
	gcc -c utility.c

clean:
	@rm *.o

# Preceding comments with an @ will suppress the echoing of the commands 
install:
	@cp dnsh /bin/ > /dev/null 2>&1
remove:
	@rm /bin/dnsh

docs:
	doxygen Doxyfile

