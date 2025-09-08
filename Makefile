PROJ_NAME = Jack
CC = gcc
BUILD_DIR = ./bin
SRC_DIR = ./src
FILES = $(wildcard ./src/*.c ./src/*/*.c)
OBJS = $(files:%=$(BUILD_DIR)/%.o)
#SRC_FILES = $(filter-out ./src/test.c, $(FILES))
#TEST_FILES = $(filter-out ./src/main.c, $(FILES))
#LIB_FILES = $(filter-out ./src/main.c ./src/test.c ./src/test_init.c, $(FILES))
#SRC_OBJS = $(SRC_FILES:%=$(BUILD_DIR)/$(notdir %).o)
#SRC_OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC_FILES))
#TEST_OBJS = $(TEST_FILES:%=$(BUILD_DIR)/$(notdir %).o)
#TEST_OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(TEST_FILES))
LIB_OBJS = $(FILES:%=$(BUILD_DIR)/$(notdir %).o)
LIB_OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(FILES))
DEPS = $(OBJS:.o=.d)
DFLAGS = -g -O0 -Wall -MMD -MP

export LD_LIBRARY_PATH = $(PWD)/lib

ifeq ($(OS),Windows_NT)
	DEVICE += -DWINDOWS
	LIBS += -L ./lib
	INCLUDE += -I ./include
	LINK += -l:glfw3.dll -l:libcglm.a -lopengl32 -lpthread
else
	detected_OS = $(shell uname)
	ifeq ($(detected_OS),Linux)
		LIBS += -L usr/lib/x86_64-linux-gnu/
		LIBS += -L ./lib
		INCLUDE += -I ./include
		LINK += -l:libglfw.so.3 -lGL -lX11 -l:libXrandr.so.2 -l:libXi.so.6
		LINK += -ldl -lm -l:libcglm.so.0 -lpthread
	endif
endif

.PHONY: clean run debug test main

all: ./bin/src ./bin/include $(BUILD_DIR)/$(PROJ_NAME)

test: ./bin/src $(BUILD_DIR)/$(PROJ_NAME)_TEST

main: ./bin/src $(BUILD_DIR)/$(PROJ_NAME)_MAIN

$(BUILD_DIR)/$(PROJ_NAME): $(LIB_OBJS)
	@ar rcs $(BUILD_DIR)/libengine.a $(LIB_OBJS)
	@cp -r ./include/interface/* $(BUILD_DIR)/include/engine
	@echo "Created library"

$(BUILD_DIR)/%.o: %.c
	@$(CC) $(DFLAGS) $(INCLUDE) -c -fPIC -o $@ $<
	@echo "--> Compiled: " $<

./bin/src:
	@mkdir -p ./bin/src
	@mkdir -p ./bin/src/math
	@mkdir -p ./bin/src/physics
	@mkdir -p ./bin/src/2d
	@mkdir -p ./bin/src/2d/physics
	@echo "Created build directory"

./bin/include:
	@mkdir ./bin/include
	@mkdir ./bin/include/engine
	@echo "Created library header directories"

clean:
	@rm -rf ./resources/*/*.obj.bin*
	@rm -rf $(BUILD_DIR)
	@echo "Cleaned ./bin"

-include $(DEPS)

