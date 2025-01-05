# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Source files
SRC = main.c libs/cJSON.c
OBJ = $(SRC:.c=.o)

# Target executable name
TARGET = bioinf_projA

# Build rules
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

# Clean up object files and the executable
clean:
	rm -f $(OBJ) $(TARGET)