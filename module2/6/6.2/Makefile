all: main

lib: build/libphonebook.so

# Динамическая библиотека, содержащая PhoneBook и его зависимости (Contact, Strings)
build/libphonebook.so: build/PhoneBook.o build/Contact.o build/Strings.o
	gcc -shared -o $@ $^

# Основная программа использует динамическую библиотеку
main: build/libphonebook.so build/Interface.o build/main.o
	gcc build/Interface.o build/main.o -Lbuild -lphonebook \
	    -Wl,-rpath,'$$ORIGIN/build' -o $@

# Все объектные файлы собираются с -fPIC (необходимо для динамической библиотеки)
build/%.o: src/%.c
	@mkdir -p build
	gcc -std=c11 -fPIC -g -O0 -MMD -MP -Iinclude -c $< -o $@

clean:
	rm -rf build main

rebuild: clean all

tests/test_strings: tests/test_strings.c tests/unity/unity.c src/Strings.c
	gcc -std=c11 -Wall -Wextra -Wshadow -g -O0 -Iinclude -Itests/unity -MMD -MP \
	    $^ -o $@

tests/test_contact: tests/test_contact.c tests/unity/unity.c src/Contact.c src/Strings.c
	gcc -std=c11 -Wall -Wextra -Wshadow -g -O0 -Iinclude -Itests/unity -MMD -MP \
	    $^ -o $@

tests/test_phonebook: tests/test_phonebook.c tests/unity/unity.c build/libphonebook.so
	gcc -std=c11 -Wall -Wextra -Wshadow -g -O0 -Iinclude -Itests/unity -MMD -MP \
	    tests/test_phonebook.c tests/unity/unity.c \
	    -Lbuild -lphonebook -Wl,-rpath,'$$ORIGIN/../build' -o $@

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