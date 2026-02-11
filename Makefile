CC=gcc
CFLAGS=-Wall -O2
TARGET=bin/sftp-toggle
SRC=src/main.c

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p bin
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
