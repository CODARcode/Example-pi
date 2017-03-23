.PHONY: all
all: pi

pi: pi.c
	gcc -std=c99 -lm -o pi pi.c
