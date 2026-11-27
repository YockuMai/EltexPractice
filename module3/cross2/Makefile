build/main: build/driver.o build/strongpoint.o build/common.o build/main.o
	gcc build/driver.o build/strongpoint.o build/common.o build/main.o -Iinclude -o main

build/driver.o: src/driver.c include/common.h include/driver.h | build
	gcc -Wall -Wextra -Iinclude -c src/driver.c -o build/driver.o

build/strongpoint.o: src/strongpoint.c include/strongpoint.h include/common.h | build
	gcc -Wall -Wextra -Iinclude -c src/strongpoint.c -o build/strongpoint.o

build/common.o: src/common.c include/common.h | build
	gcc -Wall -Wextra -Iinclude -c src/common.c -o build/common.o

build/main.o: src/main.c include/strongpoint.h | build
	gcc -Wall -Wextra -Iinclude -c src/main.c -o build/main.o
	
build:
	mkdir -p build

clean:
	rm -rf build
	rm -f main