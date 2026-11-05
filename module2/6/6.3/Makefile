CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wshadow -g -O0 -fPIC -Iinclude -MMD -MP

OPS := add subtract multiply divide
LIBS := $(addprefix build/libs/lib, $(addsuffix .so, $(OPS)))

all: main

lib: $(LIBS)

build/libs/lib%.so: build/%.o
	@mkdir -p build/libs
	$(CC) -shared -o $@ $<

main: $(LIBS) build/Interface.o build/main.o
	$(CC) build/Interface.o build/main.o -ldl -o $@

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

run: main
	./main build/libs

clean:
	rm -rf build main

rebuild: clean all

-include $(wildcard build/*.d)