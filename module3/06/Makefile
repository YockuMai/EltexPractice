build/main: build/main.o build/chat.o build/my_threads.o
	gcc build/main.o build/chat.o build/my_threads.o -Iinclude -pthread -o main

build/main.o: src/main.c include/chat.h include/my_threads.h | build
	gcc -Wall -Wextra -Iinclude -c src/main.c -o build/main.o

build/chat.o: src/chat.c include/common.h | build
	gcc -Wall -Wextra -Iinclude -c src/chat.c -o build/chat.o

build/my_threads.o: src/my_threads.c include/chat.h include/common.h | build
	gcc -Wall -Wextra -Iinclude -c src/my_threads.c -o build/my_threads.o

build:
	mkdir -p build

clean:
	rm -rf build
	rm -f main