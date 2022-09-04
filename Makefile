win:
	gcc -Wall -Werror -I./include -L./lib -o main ./src/main.c ./src/glad.c ./src/shader.c ./src/model.c ./bin/glew32.dll ./bin/glut32.dll ./bin/libcglm-0.dll

arch:
	gcc -Wall -Werror -I./include -L/usr/lib -o main ./src/main.c ./src/glad.c ./src/shader.c ./src/model.c ./src/obj_preprocessor.c -l:libcglm.so -l:libglut.so -l:libglfw.so

debug:
	gcc -g -o0 -Wall -Werror -I./include -L/usr/lib -o main ./src/main.c ./src/glad.c ./src/shader.c ./src/model.c ./src/obj_preprocessor.c -l:libcglm.so -l:libglut.so -l:libglfw.so
