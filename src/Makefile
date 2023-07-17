FILES = main.c glad.c shader.c model_loader.c model.c entity.c obj_preprocessor.c material.c helpers.c animation.c oct_tree.c physics.c simulation.c init.c spatial_algebra.c featherstone.c
TEST_FILES = test.c glad.c shader.c model_loader.c model.c entity.c obj_preprocessor.c material.c helpers.c animation.c oct_tree.c physics.c simulation.c init.c spatial_algebra.c featherstone.c

win:
	gcc -Wall -Werror -I../include -LC:/msys64/mingw64/bin -L:/msys64/mingw64/lib -o Jack $(FILES) -l:glfw3.dll -l:libcglm.a

linux:
	gcc -Wall -Werror -I../include -L/usr/lib/x86_64-linux-gnu -o Jack $(FILES) -l:libcglm.so.0 -l:libglfw.so.3 -lGL -lX11 -lpthread -l:libXrandr.so.2 -l:libXi.so.6 -ldl -lm

debug_linux:
	gcc -g -O0 -Wall -Werror -I../include -L/usr/lib/x86_64-linux-gnu -o Jack $(FILES) -l:libcglm.so.0 -l:libglfw.so.3 -lGL -lX11 -lpthread -l:libXrandr.so.2 -l:libXi.so.6 -ldl -lm

debug_win:
	gcc -g -Wall -Werror -I../include -LC:/msys64/mingw64/bin -L:/msys64/mingw64/lib -o Jack $(FILES) -l:glfw3.dll -l:libcglm.a

test_linux:
	gcc -g -O0 -Wall -Werror -I../include -L/usr/lib/x86_64-linux-gnu -o Jack $(TEST_FILES) -l:libcglm.so.0 -l:libglfw.so.3 -lGL -lX11 -lpthread -l:libXrandr.so.2 -l:libXi.so.6 -ldl -lm

test_win:
	gcc -g -Wall -Werror -I../include -LC:/msys64/mingw64/bin -L:/msys64/mingw64/lib -o Jack $(TEST_FILES) -l:glfw3.dll -l:libcglm.a

