build/main: build/server.o build/client.o build/protocol.o build/main.o
	gcc build/server.o build/client.o build/protocol.o build/main.o -Iinclude -o main

build/server.o: src/server.c include/protocol.h include/server.h | build
	gcc -Wall -Wextra -Iinclude -c src/server.c -o build/server.o

build/client.o: src/client.c include/protocol.h include/client.h | build
	gcc -Wall -Wextra -Iinclude -c src/client.c -o build/client.o

build/protocol.o: src/protocol.c include/protocol.h | build
	gcc -Wall -Wextra -Iinclude -c src/protocol.c -o build/protocol.o

build/main.o: src/main.c | build
	gcc -Wall -Wextra -Iinclude -c src/main.c -o build/main.o
	
build:
	mkdir -p build

clean:
	rm -rf build
	rm -f main