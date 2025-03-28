CUR_DIR := .
BIN_DIR := $(CUR_DIR)/bin
INC_DIR := $(CUR_DIR)/inc
OBJ_DIR := $(CUR_DIR)/obj
SRC_DIR := $(CUR_DIR)/src

C_FLAGS := -I $(INC_DIR)

all: build_obj
	gcc -o $(BIN_DIR)/CA.out $(OBJ_DIR)/main.o

build_obj:
#	gcc -c $(SRC_DIR)/main.c -o $(OBJ_DIR)/main.o $(C_FLAGS)
	gcc -c $(SRC_DIR)/main.c -o $(OBJ_DIR)/main.o

clean:
	rm -rf $(BIN_DIR)/*.out
	rm -rf $(OBJ_DIR)/*.o