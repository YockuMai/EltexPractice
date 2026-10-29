CC := gcc
STD := -std=c11

# Продакшен
CFLAGS := $(STD) -D_DEFAULT_SOURCE \
          -Wall -Wextra -Wpedantic -Wshadow \
          -Wconversion -Wsign-conversion \
          -g -O0 -MMD -MP

LDFLAGS :=
LDLIBS  :=

SRC := main.c \
       getPermissions.c \
       setPermissions.c \
       parsers.c \
       ui.c \
	   Strings.c

BUILD_DIR := build
TARGET    := main

OBJ := $(addprefix $(BUILD_DIR)/, $(SRC:.c=.o))
DEP := $(OBJ:.o=.d)

# Тесты
TEST_DIR  := tests
UNITY_DIR := $(TEST_DIR)/unity
UNITY_SRC := $(UNITY_DIR)/unity.c


BASE_TEST_CFLAGS := $(STD) -D_DEFAULT_SOURCE -Wall -Wextra -Wshadow -g -O0 \
                    -I. -I$(UNITY_DIR)

TEST_CFLAGS  := $(BASE_TEST_CFLAGS) -MMD -MP
TEST_LDFLAGS :=

TEST_SRC := $(wildcard $(TEST_DIR)/test_*.c)
TEST_BIN := $(TEST_SRC:.c=)
TEST_DEP := $(TEST_BIN:=.d)

# От каких .c проекта зависит каждый тест.
PROJECT_DEPS_test_parsers  := parsers.c Strings.c
PROJECT_DEPS_test_get_permissions := getPermissions.c Strings.c parsers.c
PROJECT_DEPS_test_set_permissions := setPermissions.c Strings.c parsers.c

# Санитайзеры
SAN_FLAGS := -fsanitize=address,undefined -fno-omit-frame-pointer

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

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