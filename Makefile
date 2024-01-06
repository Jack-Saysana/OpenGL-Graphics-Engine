PROJ_NAME = Jack
CC = gcc
BUILD_DIR = ./bin
SRC_DIR = ./src
FILES = $(wildcard ./src/*.c)
SRC_FILES = $(filter-out ./src/test.c, $(FILES))
TEST_FILES = $(filter-out ./src/main.c, $(FILES))
SRC_OBJS = $(SRC_FILES:%=$(BUILD_DIR)/%.o)
TEST_OBJS = $(TEST_FILES:%=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)
DFLAGS = -g -O0 -Wall -Werror -MMD -MP

ifeq ($(OS),Windows_NT)
	LIBS += -L ./lib
	INCLUDE += -I ./include
	LINK += -l:glfw3.dll -l:libcglm.a
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

.PHONY: clean run debug test

all: ./bin/src $(BUILD_DIR)/$(PROJ_NAME)

test: ./bin/src $(BUILD_DIR)/$(PROJ_NAME)_TEST

$(BUILD_DIR)/$(PROJ_NAME): $(SRC_OBJS)
	$(CC) $(LIBS) $(SRC_OBJS) -o $@ $(LINK)

$(BUILD_DIR)/$(PROJ_NAME)_TEST: $(TEST_OBJS)
	$(CC) $(LIBS) $(TEST_OBJS) -o $(BUILD_DIR)/$(PROJ_NAME) $(LINK)

$(BUILD_DIR)/%.c.o: %.c
	$(CC) $(DFLAGS) $(INCLUDE) -c $< -o $@

./bin/src:
	mkdir -p ./bin/src

clean:
	rm -rf $(BUILD_DIR)

run:
	@./bin/$(PROJ_NAME)

debug:
	gdb ./bin/$(PROJ_NAME)

-include $(DEPS)

