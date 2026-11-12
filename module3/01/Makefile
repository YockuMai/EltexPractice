build/copy_files: build/copy_files.o build/main.o
	gcc -fsanitize=address build/copy_files.o build/main.o -o main

build/copy_files.o: src/copy_files.c include/copy_files.h | build
	gcc -Wall -Wextra -Iinclude -c src/copy_files.c -o build/copy_files.o

build/main.o: src/main.c include/copy_files.h | build
	gcc -Wall -Wextra -Iinclude -c src/main.c -o build/main.o

build:
	mkdir -p build

clean:
	rm -rf build