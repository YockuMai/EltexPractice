build/main: build/main.o build/queue.o build/my_threads.o
	gcc build/main.o build/queue.o build/my_threads.o -Iinclude -pthread -o main

build/main.o: src/main.c include/queue.h include/my_threads.h | build
	gcc -Wall -Wextra -Iinclude -c src/main.c -o build/main.o

build/queue.o: src/queue.c include/queue.h | build
	gcc -Wall -Wextra -Iinclude -c src/queue.c -o build/queue.o

build/my_threads.o: src/my_threads.c include/my_threads.h include/queue.h | build
	gcc -Wall -Wextra -Iinclude -c src/my_threads.c -o build/my_threads.o

build:
	mkdir -p build

clean:
	rm -rf build