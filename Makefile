compile:
	gcc -Wall -Werror -I./include -L./lib -o main ./src/main.c ./src/glad.c ./bin/glew32.dll ./bin/glut32.dll ./bin/libcglm-0.dll  
