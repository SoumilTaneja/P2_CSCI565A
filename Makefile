# Makefile for Maekawa's Mutual Exclusion Project

# Compiler to use
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Source files
SRC = main.c maekawa.c vector_clock.c

# Header files
HDR = maekawa.h vector_clock.h

# Object files
OBJ = main.o maekawa.o vector_clock.o

# Output executable
OUT = maekawa

# Default target to build the executable
all: $(OUT)

# Rule to build the executable
$(OUT): $(OBJ)
	$(CC) $(OBJ) -o $(OUT)

# Rule to compile the source files
%.o: %.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and the executable
clean:
	rm -f $(OBJ) $(OUT)
