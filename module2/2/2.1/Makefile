CC := gcc
STD := -std=c11

# Флаги компиляции
CFLAGS := $(STD) -Wall -Wextra -Wpedantic -Wshadow \
          -Wconversion -Wsign-conversion \
          -g -O0 -MMD -MP

SRC := main.c \
       Contact.c \
       PhoneBook.c \
       Strings.c \
       Interface.c

BUILD_DIR := build
TARGET := main

OBJ := $(addprefix $(BUILD_DIR)/, $(SRC:.c=.o))
DEP := $(OBJ:.o=.d)

# Тесты
TEST_DIR := tests
UNITY_DIR := $(TEST_DIR)/unity
UNITY_SRC := $(UNITY_DIR)/unity.c

# С -I. — чтобы в тестах писать #include "Contact.h", #include "Strings.h".
BASE_TEST_CFLAGS := $(STD) -Wall -Wextra -Wshadow -g -O0 \
                    -I. -I$(UNITY_DIR)

TEST_CFLAGS := $(BASE_TEST_CFLAGS) -MMD -MP
TEST_LDFLAGS :=

TEST_SRC := $(wildcard $(TEST_DIR)/test_*.c)
TEST_BIN := $(TEST_SRC:.c=)
TEST_DEP := $(TEST_BIN:=.d)

# От каких .c проекта зависит каждый тест.
PROJECT_DEPS_test_strings := Strings.c
PROJECT_DEPS_test_contact := Contact.c Strings.c
PROJECT_DEPS_test_phonebook := PhoneBook.c Contact.c Strings.c

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $^ -o $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)

rebuild: clean all

# Тесты: правило через шаблон
define TEST_RULE
$(TEST_DIR)/$(1): $(TEST_DIR)/$(1).c $(UNITY_SRC) $$(PROJECT_DEPS_$(1))
	$$(CC) $$(TEST_CFLAGS) $$(TEST_LDFLAGS) $$^ -o $$@
endef

$(foreach t,$(notdir $(basename $(TEST_SRC))),$(eval $(call TEST_RULE,$(t))))

# Запуск всех тестов; падение любого → exit 1, но остальные всё равно выполняются
test: $(TEST_BIN)
	@echo "ЗАПУСК ТЕСТОВ"
	@failed=0; \
	for t in $(TEST_BIN); do \
	    echo "$$t"; \
	    ./$$t || failed=1; \
	done; \
	exit $$failed

clean-test:
	rm -f $(TEST_BIN) $(TEST_DEP)

# Автозависимости
-include $(DEP)
-include $(TEST_DEP)