all: update decipher qsort tribonacci

update: src/update.c
	gcc -o update src/update.c

decipher: src/decipher.c
	gcc -o bin/decipher src/decipher.c -lm

qsort: src/qsort.c
	gcc -o bin/qsort src/qsort.c

tribonacci: src/tribonacci.c
	gcc -o bin/tribonacci src/tribonacci.c

clean: 
	rm bin/*

