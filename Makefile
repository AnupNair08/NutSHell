CC=gcc
IPATH=-Isrc/
SRC_DIR=src/
BIN_DIR=src/bin/

.PHONY: init
	
init:
	$(CC) -c $(SRC_DIR)shell.c
	@mv shell.o src/bin

	$(CC) $(IPATH) -c $(SRC_DIR)command.c
	@mv command.o $(BIN_DIR)
	
	$(CC) $(IPATH) -c $(SRC_DIR)jobs.c
	@mv jobs.o $(BIN_DIR)

	$(CC)  $(BIN_DIR)shell.o $(BIN_DIR)command.o $(BIN_DIR)jobs.o -g -Wall -o nsh
	@./nsh

jobs.o: $(SRC_DIR)jobs.c $(SRC_DIR)shell.h
	$(CC) $(IPATH) -c $(SRC_DIR)jobs.c
	@mv jobs.o $(BIN_DIR)

shell.o: $(SRC_DIR)shell.c 
	$(CC) -c $(SRC_DIR)shell.c
	@mv shell.o $(BIN_DIR)

command.o: $(SRC_DIR)command.c $(SRC_DIR)shell.h
	$(CC) $(IPATH) -c $(SRC_DIR)command.c
	@mv command.o $(BIN_DIR)

nsh: $(BIN_DIR)shell.o  $(BIN_DIR)command.o $(BIN_DIR)jobs.o  
	$(CC)  $(BIN_DIR)shell.o $(BIN_DIR)command.o $(BIN_DIR)jobs.o -g -o nsh
	@mv nsh $(BIN_DIR)

run:
	@make
	@mv nsh $(BIN_DIR)
	@echo Starting Shell
	@src/bin/nsh

clean:
	@rm $(BIN_DIR)*.o $(BIN_DIR)nsh > /dev/null 2>&1

install:
	@cp $(BIN_DIR)nsh /bin/ > /dev/null 2>&1
remove:
	@rm /bin/nsh

docs:
	cd src && doxygen Doxyfile

