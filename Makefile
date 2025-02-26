.DEFAULT_GOAL := starlight

SYSNAME := $(shell uname -s)

SRC_DIR  := src
OBJ_DIR  := obj
BIN_DIR  := bin
TEST_DIR := tests
CERT_DIR := certs

CC 		 := gcc
CC_FLAGS := -Wall -Wextra -std=c99 -pedantic -g 
LIBS     := -lssl -lcrypto

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
TARGET  = $(BIN_DIR)/starlight

TEST_SRC = $(wildcard $(TEST_DIR)/*.c)
TEST_SRC += $(SRC_DIR)/uri.c
TEST_SRC += $(SRC_DIR)/util.c
TEST_OBJ = $(addprefix $(OBJ_DIR)/tests/, $(addsuffix .o, $(basename $(notdir $(TEST_SRC)))))
TEST_TGT = $(BIN_DIR)/tests/starlightTests

.PHONY: all always tests clean debug certs starlight

all: starlight tests

starlight: always $(TARGET)

debug: always $(TARGET) certs
	@cp $(CERT_DIR)/starlight.crt bin/starlight.crt
	@cp $(CERT_DIR)/starlight.key bin/starlight.key
	@cd $(BIN_DIR) && ./starlight

runtests: tests
	cd $(BIN_DIR)/tests && ./starlightTests

certs: 
	@test -s $(CERT_DIR)/starlight.crt || { echo Certificate file missing! Please generate the required SSL certificates.; exit 1; }
	@test -s $(CERT_DIR)/starlight.key || { echo Key file missing! Please generate the required SSL certificates.; exit 1; }

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
