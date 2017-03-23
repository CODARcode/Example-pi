.PHONY: all
all: pi

pi: pi.c
	gcc -std=gnu99 -lm -O3 -o pi pi.c

.PHONY: clean
clean:
	rm -rf pi
