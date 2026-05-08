CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -g
TARGET = memscan
SRC = memscan.c
IMAGE_SCRIPT = make_test_image.py
IMAGE = disk.img
WORDLIST = wordlist.txt

.PHONY: all image run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

image:
	python3 $(IMAGE_SCRIPT)

run: $(TARGET)
	./$(TARGET) $(IMAGE) $(WORDLIST)

clean:
	rm -f $(TARGET)
