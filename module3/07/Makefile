build/main: build/main.o build/server.o build/client.o build/protocol.o
	gcc build/main.o build/server.o build/client.o build/protocol.o -Iinclude -o main

build/main.o: src/main.c include/server.h include/client.h | build
	gcc -Wall -Wextra -Iinclude -c src/main.c -o build/main.o

build/server.o: src/server.c include/common.h include/protocol.h | build
	gcc -Wall -Wextra -Iinclude -c src/server.c -o build/server.o

build/client.o: src/client.c include/common.h include/client.h | build
	gcc -Wall -Wextra -Iinclude -c src/client.c -o build/client.o

build/protocol.o: src/protocol.c include/common.h include/protocol.h | build
	gcc -Wall -Wextra -Iinclude -c src/protocol.c -o build/protocol.o

build:
	mkdir -p build

clean:
	rm -rf build
	rm -f main