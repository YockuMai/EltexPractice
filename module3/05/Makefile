build/main: build/main.o build/producer.o build/consumer.o
	gcc build/main.o build/producer.o build/consumer.o -Iinclude -o main

build/main.o: src/main.c include/producer.h include/consumer.h | build
	gcc -Wall -Wextra -Iinclude -c src/main.c -o build/main.o

build/producer.o: src/producer.c include/producer.h include/common.h | build
	gcc -Wall -Wextra -Iinclude -c src/producer.c -o build/producer.o

build/consumer.o: src/consumer.c include/consumer.h include/common.h | build
	gcc -Wall -Wextra -Iinclude -c src/consumer.c -o build/consumer.o

build:
	mkdir -p build

clean:
	rm -rf build
	rm -f main