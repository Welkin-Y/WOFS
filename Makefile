# Makefile

# Compiler and flags
CC := g++
CFLAGS := -Wall -std=c++2a -O3 -D_FILE_OFFSET_BITS=64
LIBS := `pkg-config fuse --cflags --libs`

# Directories
SRC_DIR := src
BUILD_DIR := build
TEST_DIR := test
OBJ_DIR := $(BUILD_DIR)/obj
DEP_DIR := $(BUILD_DIR)/deps

# Ensure the directories exist
$(shell mkdir -p $(OBJ_DIR) $(DEP_DIR) >/dev/null)

# Source files and their corresponding object and dependency files
COMMON_SRCS := $(wildcard $(SRC_DIR)/*.cpp $(SRC_DIR)/*.c)
TEST_SRCS := $(wildcard $(TEST_DIR)/*.cpp)
COMMON_OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(COMMON_SRCS)))
TEST_OBJS := $(patsubst $(TEST_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(TEST_SRCS))
DEPS := $(COMMON_OBJS:.o=.d) $(TEST_OBJS:.o=.d)

# Executables
WOFS_EXE := $(BUILD_DIR)/wofs
TEST_EXE := $(BUILD_DIR)/test

# Include the dependency files
-include $(DEPS)


.PHONY: all clean wofs test

all: wofs

wofs: $(WOFS_EXE)

test: CFLAGS += -I$(SRC_DIR)
test: $(TEST_EXE)

# Compilation rules
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D) $(dir $(DEP_DIR)/$*.d)
	$(CC) $(CFLAGS) -MMD -c $< -o $@ -MF $(patsubst $(OBJ_DIR)/%.o,$(DEP_DIR)/%.d,$@)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D) $(dir $(DEP_DIR)/$*.d)
	$(CC) $(CFLAGS) -MMD -c $< -o $@ -MF $(patsubst $(OBJ_DIR)/%.o,$(DEP_DIR)/%.d,$@)

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(@D) $(dir $(DEP_DIR)/$*.d)
	$(CC) $(CFLAGS) -MMD -c $< -o $@ -MF $(patsubst $(OBJ_DIR)/%.o,$(DEP_DIR)/%.d,$@)

# Linking rules
$(WOFS_EXE): $(COMMON_OBJS)
	$(CC) $^ $(LIBS) -o $@

EXCLUDE_OBJS := $(OBJ_DIR)/wofs.o $(OBJ_DIR)/try.o

$(TEST_EXE): $(TEST_OBJS) $(filter-out $(EXCLUDE_OBJS), $(COMMON_OBJS))
	$(CC) $^ $(LIBS) -o $@

clean:
	rm -rf $(BUILD_DIR)

