.PHONY: all
all: pi pi-gmp

pi: pi.c
	gcc -std=gnu99 -lm -O3 -o pi pi.c

pi-gmp: pi-gmp.c
	gcc -std=gnu99 -lgmp -O3 -o pi-gmp pi-gmp.c

.PHONY: clean
clean:
	rm -rf pi pi-gmp
