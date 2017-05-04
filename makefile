CPPC=g++ -std=c++11
CC=gcc
MKDIR=mkdir
OUTPUT=exe.out

BIN_PATH=bin
SRC_PATH=src
INC_PATH=inc
OBJ_PATH=obj

output : $(OBJ_PATH)/main.o $(BIN_PATH)
	$(CPPC) $(OBJ_PATH)/main.o -o $(BIN_PATH)/$(OUTPUT)

$(OBJ_PATH)/main.o : $(SRC_PATH)/main.cpp $(OBJ_PATH)
	$(CPPC) -c $(SRC_PATH)/main.cpp -o $(OBJ_PATH)/main.o

$(OBJ_PATH) :
	$(MKDIR) $(OBJ_PATH)

$(BIN_PATH) :
	$(MKDIR) $(BIN_PATH)