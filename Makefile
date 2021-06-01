CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -g
LDLIBS = -lnfb -lfdt

TARGET = hw2

.PHONY: all clean

all: $(TARGET)

clean:
	rm -f $(TARGET)