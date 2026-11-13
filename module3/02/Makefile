build/main: build/broker.o build/publisher.o build/subscriber.o build/my_string.o build/main.o
	gcc build/broker.o build/publisher.o build/subscriber.o build/my_string.o build/main.o -Iinclude -o main

build/my_string.o: src/my_string.c include/my_string.h | build
	gcc -Wall -Wextra -Iinclude -c src/my_string.c -o build/my_string.o

build/broker.o: src/broker.c include/broker.h | build
	gcc -Wall -Wextra -Iinclude -c src/broker.c -o build/broker.o

build/publisher.o: src/publisher.c include/publisher.h | build
	gcc -Wall -Wextra -Iinclude -c src/publisher.c -o build/publisher.o

build/subscriber.o: src/subscriber.c include/subscriber.h | build
	gcc -Wall -Wextra -Iinclude -c src/subscriber.c -o build/subscriber.o

build/main.o: src/main.c | build
	gcc -Wall -Wextra -Iinclude -c src/main.c -o build/main.o

build:
	mkdir -p build

clean:
	rm -rf build