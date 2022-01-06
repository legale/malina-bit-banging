#name of output file
NAME = bitbanging

#Linker flags
LIBS  = -lm

#Compiler flags
CFLAGS = 

#Compiler
CC = gcc

#SRC=$(wildcard *.c)
SRC = malina.c yarpio/yarpio.c

all: bin
all: CFLAGS += -O2

debug: CFLAGS += -DDEBUG -g
debug: bin

noO: CFLAGS += -O0
noO: bin

warn: CFLAGS += -Wall
warn: bin

bin: $(NAME)

$(NAME): $(SRC)
		$(CC) $(CFLAGS) $(LIBS) $^ -o build/$@  

clean: 
		rm -rf build/*

