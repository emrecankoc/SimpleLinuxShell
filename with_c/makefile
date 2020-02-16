

all: build  run

build:
	gcc -I ./include -o ./lib/Tokenizer.o -c ./src/Tokenizer.c
	gcc -I ./include -o ./lib/Shell.o -c ./src/Shell.c	
	gcc -I ./include -o ./bin/shell  ./lib/Tokenizer.o ./lib/Shell.o ./src/Main.c 
run:
	./bin/shell
