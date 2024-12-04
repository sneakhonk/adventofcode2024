# Makefile

CC = gcc
CFLAGS = -Wall -O3
TARGET = program
SOURCES = main.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)
