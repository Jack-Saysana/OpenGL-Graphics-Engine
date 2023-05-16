FILES = main.c glad.c shader.c model_loader.c model.c entity.c obj_preprocessor.c material.c helpers.c animation.c oct_tree.c physics.c simulation.c init.c
OBJ_FILES = shader.c model_loader.c model.c entity.c obj_preprocessor.c material.c helpers.c animation.c oct_tree.c physics.c simulation.c init.c
FLAGS = -Wall -Werror
FLAGS_DEBUG = -Wall -Werror -O0 -g

# OS-SPECIFIC SETTINGS
ifeq ($(OS),Windows_NT)
  # Link libgcc and libstdc++ statically on Windows
  LDFLAGS += -static-libgcc -static-libstdc++
  LIBS += -LC:/msys64/mingw64/bin -L:/msys64/mingw64/lib
  LINK += -l:glfw3.dll -l:libcglm.a
  INCLUDE += -I../include
else
  detected_OS := $(shell uname)
  ifeq ($(detected_OS),Linux)
    # LINUX-DEPENDENT SETTINGS
    LIBS += -L /usr/lib/x86_64-linux-gnu/
    INCLUDE += -I ../include
    LINK += -l:libcglm.so.0 -l:libglfw.so.3 -lGL -lX11 -lpthread -l:libXrandr.so.2 -l:libXi.so.6 -ldl -lm
  endif
endif


.PHONY: win
win:
	gcc $(FLAGS) $(INCLUDE) $(LIBS) -o Jack $(FILES) $(LINK)

.PHONY: linux
linux:
	gcc $(FLAGS) $(INCLUDE) $(LIBS) -o Jack $(FILES) $(LINK)

.PHONY: debug_linux
debug_linux:
	gcc $(FLAGS_DEBUG) $(INCLUDE) $(LIBS) -o Jack $(FILES) $(LINK)

.PHONY: debug_win
debug_win:
	gcc $(FLAGS_DEBUG) $(INCLUDE) $(LIBS) -o Jack $(FILES) $(LINK)

.PHONY: object_files
object_files:
	gcc -c $(FLAGS_DEBUG) $(INCLUDE) $(LIBS) -fpic $(OBJ_FILES) $(LINK)
