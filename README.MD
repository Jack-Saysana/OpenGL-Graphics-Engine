
# OpenGL Rendering Engine

## Introduction/Overview:

This repository serves as a codebase for my OpenGL based graphics engine, initially built using C++ via a variety of libraries, but now refactored for full development in C. Currently, this project supports a variety of features to speed up graphical development such as:

- Model loading

- Animation

- Collision detection

- Impulse-based collision resolution

- UI Menu building

- 6D Vector and algebra

## Building and usage

From inside the root of the repo, build the library using:

```make```

A new directory in the root of the repo should be created called `bin`. This new directory should include the shared binary and headers for the library.

Simply link against the library and include the headers to use the library.

## Getting Started

With the library built and ready to use, be sure to read the documentation to learn more about its usage.

### Important pages:

- [Window Initialization](./docs/initialization.md)

- [Everything related to models and their instances](./docs/model_loading.md)

- [Simulations](./docs/simulation.md)

- [UI menu building](./docs/ui.md)

- [Misc helper functions](./docs/helpers.md)

- [Misc math functions](./docs/spatial_algebra.md)

## In progress / future plans:

- Ragdoll Physics implementation (WIP)
- Inverse kinematics (Planned)

## Libraries used:

- [glad](https://glad.dav1d.de/)

- [GLFW](https://www.glfw.org/)

- [cglm](https://github.com/recp/cglm)

- [stb](https://github.com/nothings/stb)
