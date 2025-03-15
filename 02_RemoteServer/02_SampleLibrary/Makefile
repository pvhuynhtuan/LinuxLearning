CUR_DIR := ./
BIN_DIR := $(CUR_DIR)/bin
INC_DIR := $(CUR_DIR)/inc
LIB_DIR := $(CUR_DIR)/lib
LIB_STATIC_DIR := $(LIB_DIR)/static
LIB_SHARE_DIR := $(LIB_DIR)/share
OBJ_DIR := $(CUR_DIR)/obj
SRC_DIR := $(CUR_DIR)/src

C_FLAGS := -I $(INC_DIR)

all: build_obj build_lib
#	gcc -o $(BIN_DIR)/main.out $(OBJ_DIR)/main.o $(OBJ_DIR)/Hello.o
	gcc $(OBJ_DIR)/main.o -L$(LIB_SHARE_DIR) -lHello -o $(BIN_DIR)/use-shared-library

build_obj:
	gcc -c -fPIC $(SRC_DIR)/Hello.c -o $(OBJ_DIR)/Hello.o $(C_FLAGS)
	gcc -c $(SRC_DIR)/main.c -o $(OBJ_DIR)/main.o $(C_FLAGS)

build_lib:
	gcc -shared $(OBJ_DIR)/Hello.o -o $(LIB_SHARE_DIR)/libHello.so

clean:
	rm -rf $(BIN_DIR)/*
	rm -rf $(OBJ_DIR)/*
	rm -rf $(LIB_SHARE_DIR)/*
	rm -rf $(LIB_STATIC_DIR)/*