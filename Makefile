
TARGET := build/350_no_scope
SRC_DIR := ./src
BUILD_DIR := ./build
TEST_DIR := ./tests

SRC_FILES := $(shell find $(SRC_DIR) -name '*.c')
OBJ_FILES := $(SRC_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEP_FILES := $(OBJ_FILES:.o=.d)
SUBMIT_FILES := record.c table.c isam.c lsm.c
SUBMIT_TARBALL := submission.tgz
TESTS := $(shell find $(TEST_DIR) -name '*.test' | sort)

CFLAGS := -O3 -MMD -MP -std=gnu17 -g

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

submit:
	@echo "\033[94mBuilding submission file $(SUBMIT_TARBALL)\033[0m"
	tar -zcvf $(SUBMIT_TARBALL) $(patsubst %,$(SRC_DIR)/%,$(SUBMIT_FILES))

repl_doc: REPL.md

REPL.md: src/main.c
	cat src/main.c | ruby -n -e 'case $$_ when /IF_COMMAND\("([^"]+)"\)/ then print("## #{$$1}\n");found_command=true;\
	                                      when /\/\/ *(.+)/ then if found_command then print("#{$$1}\n") end\
	                                      else if found_command then print("\n"); found_command=false end end' > REPL.md

.PHONY: clean all run test doc repl_doc submit

clean: 
	@echo "\033[94mCleaning\033[0m"
	@rm -rf $(BUILD_DIR)

-include $(DEP_FILES)


