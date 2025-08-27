
TARGET := build/350_no_scope
SRC_DIR := ./src
BUILD_DIR := ./build
TEST_DIR := ./tests

SRC_FILES := $(shell find $(SRC_DIR) -name '*.c')
OBJ_FILES := $(SRC_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEP_FILES := $(OBJ_FILES:.o=.d)
TESTS := $(shell find $(TEST_DIR) -name '*.test' | sort)

CFLAGS := -O3 -MMD -MP -std=gnu17

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	@echo "\033[94m"Linking $@ from $(OBJ_FILES) "\033[0m"o
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $@ $(LD_FLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "\033[94m"Building $@ from $< "\033[0m"
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	@echo "\033[94mRunning" $(TARGET) "\033[0m"
	./$(TARGET) -

test: $(TARGET)
	@echo "\033[94mRunning tests...\033[0m"
	@for i in $(TESTS); do echo "\033[94m  $$i\033[0m"; ./$(TARGET) $$i; done

doc: .doxygen $(SRC_FILES)
	@echo "\033[94mBuilding docs\033[0m"
	doxygen .doxygen > /dev/null

.PHONY: clean all run test doc

clean: 
	@echo "\033[94mCleaning\033[0m"
	@rm -rf $(BUILD_DIR)

-include $(DEP_FILES)
