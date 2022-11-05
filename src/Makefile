win:
	gcc -Wall -Werror -I../include -LC:/msys64/mingw64/bin -L:/msys64/mingw64/lib -o Jack main.c glad.c shader.c model_loader.c model.c obj_preprocessor.c material.c helpers.c -l:glfw3.dll -l:libcglm.a

linux:
	gcc -Wall -Werror -I../include -L/usr/lib -o Jack main.c glad.c shader.c model_loader.c model.c obj_preprocessor.c material.c helpers.c -l:libcglm.so -l:libglfw.so -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm

debug_linux:
	gcc -g -o0 -Wall -Werror -I../include -L/usr/lib -o Jack main.c glad.c shader.c model_loader.c model.c obj_preprocessor.c material.c helpers.c -l:libcglm.so -l:libglfw.so -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm

debug_win:
	gcc -g -Wall -Werror -I../include -LC:/msys64/mingw64/bin -L:/msys64/mingw64/lib -o Jack main.c glad.c shader.c model_loader.c model.c obj_preprocessor.c material.c helpers.c -l:glfw3.dll -l:libcglm.a

