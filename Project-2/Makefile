# COMP 7500/7506 - Project 2: pWordcount
# Auburn University

CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -g
TARGET  = pwordcount
SRC     = pwordcount.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) input.txt

clean:
	rm -f $(TARGET)
