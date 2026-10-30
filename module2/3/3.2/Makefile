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
       ipv4_work.c \
       ui.c

BUILD_DIR := build
TARGET    := main

OBJ := $(addprefix $(BUILD_DIR)/, $(SRC:.c=.o))
DEP := $(OBJ:.o=.d)

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

# Автозависимости
-include $(DEP)
-include $(TEST_DEP)