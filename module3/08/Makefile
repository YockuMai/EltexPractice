build/main: build/main.o 
	gcc build/main.o -Iinclude -o main

build/main.o: src/main.c | build
	gcc -Wall -Wextra -Iinclude -c src/main.c -o build/main.o
	
build:
	mkdir -p build

clean:
	rm -rf build
	rm -f main