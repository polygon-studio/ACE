#OBJS specifies which files to compile as part of the project
OBJS = Chip-8/src/main.cpp

#CC specifies which compiler to use
CC = g++

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w superpresses all warnings
COMPILER_FLAGS = -w

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -lSDL2

#OBJ_NAME specifies the name of the executable
OBJ_NAME = Chip-8

#This is the target that compiles the executable
all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
