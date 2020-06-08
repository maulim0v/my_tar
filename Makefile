CC = gcc
CFLAGS = -g3 -Wall -O3 -std=c99 -fsanitize=address

SOURCES = main.c my_tar_main.c
HEADERS = my_tar_main.h
OBJECTS = $(SOURCES:.c=.o) $(HEADERS)
TARGET = my_tar

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o $(TARGET)

.PHONY: clean

clean:
	@rm -f $(OBJECTS) $(TARGET) core