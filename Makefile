SYSNAME := $(shell uname -s)

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
TEST_DIR = tests

CC 		 = gcc
CC_FLAGS = -Wall -g 
LIBS     = -lssl -lcrypto

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
TARGET  = $(BIN_DIR)/starlight

TEST_SRC = $(wildcard $(TEST_DIR)/*.c)
TEST_SRC += $(SRC_DIR)/uri.c
TEST_OBJ = $(addprefix $(OBJ_DIR)/tests/, $(addsuffix .o, $(basename $(notdir $(TEST_SRC)))))
TEST_TGT = $(BIN_DIR)/tests/starlightTests

.PHONY: always tests clean

all: always $(TARGET) $(TEST_TGT)

tests: always $(TEST_TGT)

$(TEST_TGT): $(TEST_OBJ)
	$(CXX) -o $@ $^ $(CXX_FLAGS) $(LIBS)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CXX) $(CXX_FLAGS) $(SDL_FLAGS) -c -o $@ $<

$(OBJ_DIR)/tests/%.o: $(TEST_DIR)/%.c
	$(CXX) $(CXX_FLAGS) $(SDL_FLAGS) -c -o $@ $<

$(OBJ_DIR)/tests/%.o: $(SRC_DIR)/%.c
	$(CXX) $(CXX_FLAGS) $(SDL_FLAGS) -c -o $@ $<

always: 
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(OBJ_DIR)/tests
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(BIN_DIR)/tests

clean:
	@rm -rf $(OBJ_DIR)
	@rm -rf $(BIN_DIR)
