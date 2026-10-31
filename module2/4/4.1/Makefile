CC := gcc
STD := -std=c11

# Основная программа
CFLAGS := $(STD) -Wall -Wextra -Wpedantic -Wshadow \
          -Wconversion -Wsign-conversion \
          -g -O0 -MMD -MP -Iinclude

SRC := $(wildcard src/*.c)
BUILD_DIR := build
TARGET := main

OBJ := $(patsubst src/%.c, $(BUILD_DIR)/%.o, $(SRC))
DEP := $(OBJ:.o=.d)

# Тесты
TEST_DIR  := tests
UNITY_DIR := $(TEST_DIR)/unity
UNITY_SRC := $(UNITY_DIR)/unity.c

# -Iinclude — заголовки проекта; -I$(UNITY_DIR) — unity.h
BASE_TEST_CFLAGS := $(STD) -Wall -Wextra -Wshadow -g -O0 \
                    -Iinclude -I$(UNITY_DIR)
TEST_CFLAGS  := $(BASE_TEST_CFLAGS) -MMD -MP
TEST_LDFLAGS :=

TEST_SRC := $(wildcard $(TEST_DIR)/test_*.c)
TEST_BIN := $(TEST_SRC:.c=)
TEST_DEP := $(TEST_BIN:=.d)

# От каких .c проекта зависит каждый тест
PROJECT_DEPS_test_strings   := src/Strings.c
PROJECT_DEPS_test_contact   := src/Contact.c   src/Strings.c
PROJECT_DEPS_test_phonebook := src/PhoneBook.c src/Contact.c src/Strings.c

# Сборка
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $^ -o $@

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)

rebuild: clean all

# Правила для тестов
define TEST_RULE
$(TEST_DIR)/$(1): $(TEST_DIR)/$(1).c $(UNITY_SRC) $$(PROJECT_DEPS_$(1))
	$$(CC) $$(TEST_CFLAGS) $$(TEST_LDFLAGS) $$^ -o $$@
endef

$(foreach t,$(notdir $(basename $(TEST_SRC))),$(eval $(call TEST_RULE,$(t))))

test: $(TEST_BIN)
	@echo "ЗАПУСК ТЕСТОВ"
	@failed=0; \
	for t in $(TEST_BIN); do \
	    echo ">>> $$t"; \
	    ./$$t || failed=1; \
	done; \
	if [ $$failed -eq 0 ]; then echo "ВСЕ ТЕСТЫ ПРОЙДЕНЫ"; \
	else echo "ЕСТЬ ПАДЕНИЯ"; fi; \
	exit $$failed

clean-test:
	rm -f $(TEST_BIN) $(TEST_DEP)

# Автозависимости
-include $(DEP)
-include $(TEST_DEP)