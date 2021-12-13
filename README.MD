# OpenGL Tests

## Compilation and Running

**Important:** In order to compile the program, you must have a C++ compiler such as g++

**\*** If using vs-code, you can run the included buildtask to automatically compile

From inside the repo directory run the following command:

`g++ -g -std=c++17 ./src/*.cpp ./src/*.c -I./include -L./lib -lglew32 -lglfw3dll -l:libfreeglut.a -l:libassimp.dll.a -l:libIrrXML.a -static-libgcc -static-libstdc++ -o ./src/main.exe`

Once the program is compiled, `main.exe` should appear in the `src` directory of the project. Run `main.exe` in order to start the program.

## Overview:

This repository serves as a codebase for my OpenGl based graphics engine, built in C++ using, most notably, assimp, GLFW, glad and glm. Currently, the project supports a variety of features such as:
 - Shader support
 - 2D Textures
 - A configurable camera
 - Model importing and loading
 - Cubemap ussage
 - Instancing
 - Blinn-Phong based lighting
 - Directional shadow mapping

## In Progress:

- Omni-directional shadow mapping