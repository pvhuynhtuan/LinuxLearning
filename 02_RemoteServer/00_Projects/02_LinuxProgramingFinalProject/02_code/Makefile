CUR_DIR := .
BIN_DIR := $(CUR_DIR)/bin
INC_DIR := $(CUR_DIR)/inc
OBJ_DIR := $(CUR_DIR)/obj
SRC_DIR := $(CUR_DIR)/src

C_FLAGS := -I $(INC_DIR)

all: build_obj
	gcc -o $(BIN_DIR)/SG.out $(OBJ_DIR)/main.o \
		$(OBJ_DIR)/SG_MainProcess.o \
		$(OBJ_DIR)/SG_LogProcess.o \
		$(OBJ_DIR)/MP_ConnectionManager.o \
		$(OBJ_DIR)/MP_DataSharing.o \
		-lrt -lpthread

build_obj:
	gcc -c $(SRC_DIR)/SG_MainProcess.c -o $(OBJ_DIR)/SG_MainProcess.o $(C_FLAGS)
	gcc -c $(SRC_DIR)/SG_LogProcess.c -o $(OBJ_DIR)/SG_LogProcess.o $(C_FLAGS)
	gcc -c $(SRC_DIR)/MP_ConnectionManager.c -o $(OBJ_DIR)/MP_ConnectionManager.o $(C_FLAGS)
	gcc -c $(SRC_DIR)/MP_DataSharing.c -o $(OBJ_DIR)/MP_DataSharing.o $(C_FLAGS)
	gcc -c $(SRC_DIR)/main.c -o $(OBJ_DIR)/main.o $(C_FLAGS)

clean:
	rm -rf $(BIN_DIR)/*.out
	rm -rf $(OBJ_DIR)/*.o