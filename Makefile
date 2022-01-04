#name of output file
NAME = bitbanging

#Linker flags
LIBS  = -lm

#Compiler flags
CFLAGS = -Wall -O2

#Compiler
CC = gcc

#SRC=$(wildcard *.c)
SRC = malina.c yarpio/yarpio.c

all: $(NAME)

$(NAME): $(SRC)
		$(CC) $(CFLAGS) $(LIBS) $^ -o build/$@  

clean: 
		rm -rf build/*

