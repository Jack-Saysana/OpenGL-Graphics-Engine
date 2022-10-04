win:
	gcc -Wall -Werror -I./include -LC:/msys64/mingw64/bin -L:/msys64/mingw64/lib -o main ./src/main.c ./src/glad.c ./src/shader.c ./src/model_loader.c ./src/model.c ./src/obj_preprocessor.c -l:glfw3.dll -l:libcglm.a

arch:
	gcc -Wall -Werror -I./include -L/usr/lib -o main ./src/main.c ./src/glad.c ./src/shader.c ./src/model_loader.c ./src/model.c ./src/obj_preprocessor.c -l:libcglm.so -l:libglfw.so -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm

debug:
	gcc -g -o0 -Wall -Werror -I./include -L/usr/lib -o main ./src/main.c ./src/glad.c ./src/shader.c ./src/model_loader.c ./src/model.c ./src/obj_preprocessor.c -l:libcglm.so -l:libglfw.so -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm
