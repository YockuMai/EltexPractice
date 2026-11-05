all: main

lib: build/libphonebook.a


# $@ имя цели (build/libphonebook.a); $^ все зависимости (build/PhoneBook.o)
build/libphonebook.a: build/PhoneBook.o
	ar rcs $@ $^

# wildcard src/*.c - все си файлы из папки src
# patsubst src/%.c,build/%.o - заменяет путь и расширение src/X.c -> build/X.o
# filter-out build/PhoneBook.o - выкидывает PhoneBook.o, так как будет использоваться статическая библиотека
main: build/libphonebook.a \
      $(filter-out build/PhoneBook.o,$(patsubst src/%.c,build/%.o,$(wildcard src/*.c)))
	gcc $(filter-out build/PhoneBook.o,$(patsubst src/%.c,build/%.o,$(wildcard src/*.c))) \
	    -Lbuild -lphonebook -o $@


# $< - src/%.c
# $@ - build/%.o
build/%.o: src/%.c
	@mkdir -p build
	gcc -std=c11 -g -O0 -MMD -MP -Iinclude -c $< -o $@

clean:
	rm -rf build main

rebuild: clean all

tests/test_strings: tests/test_strings.c tests/unity/unity.c src/Strings.c
	gcc -std=c11 -g -O0 -Iinclude -Itests/unity -MMD -MP \
	    $^ -o $@

tests/test_contact: tests/test_contact.c tests/unity/unity.c src/Contact.c src/Strings.c
	gcc -std=c11 -g -O0 -Iinclude -Itests/unity -MMD -MP \
	    $^ -o $@

tests/test_phonebook: tests/test_phonebook.c tests/unity/unity.c \
                     src/Contact.c src/Strings.c build/libphonebook.a
	gcc -std=c11 -g -O0 -Iinclude -Itests/unity -MMD -MP \
	    $^ -o $@

test: tests/test_strings tests/test_contact tests/test_phonebook
	@echo "ЗАПУСК ТЕСТОВ"
	@failed=0; \
	for t in tests/test_strings tests/test_contact tests/test_phonebook; do \
	    echo ">>> $$t"; \
	    ./$$t || failed=1; \
	done; \
	if [ $$failed -eq 0 ]; then echo "ВСЕ ТЕСТЫ ПРОЙДЕНЫ"; \
	else echo "ЕСТЬ ПАДЕНИЯ"; fi; \
	exit $$failed

-include $(wildcard build/*.d) $(wildcard tests/*.d)