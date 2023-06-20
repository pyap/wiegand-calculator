CC = gcc
CFLAGS = -m64 -O2 -march=native -Wall -s
FILES = hid
ALL: ${FILES}
clean:
	rm -f ${FILES}
