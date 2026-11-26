CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = ocsh
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
.PHONY: all clean
