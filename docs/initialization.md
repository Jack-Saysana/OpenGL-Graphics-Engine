# Window Initialization

At the beginning of an application, a window must be initialized for the rendering of OpenGL elements. The engine provides a wrapper around GLFW's basic window initialization.

The main features include:

## Providing additional window state:

- When utilizing the engine's wrapper, the created window will automatically register screen size and mouse position callbacks.
  - Upon changing screen size, the global variables `RES_X` and `RES_Y` are automatically updated to represent the pixel size of the window width and height, respectively

- Furthermore, the wrapper also automatically registers a screen position callback, which updates the global `MOUSE_POS` vector

- The global variable `CURSOR_ENABLED` tracks whether or not the cursor is currently enabled on the window

## Functions:

```GLFWwindow *init_gl(char *name)```

Wrapper window initializer, which initializes the new GLFW window while also allocating the neccesarry resources for providing the wrapper services.

**Arguments**

- `char *name`: Name of the window

**Returns**

- The newly initialized `GLFWwindow` pointer for rendering

```void cleanup_gl()```

Terminates GLFW and cleans up any wrapper resources

```int register_fb_size_callback(void (*cb)(GLFWwindow *, int, int))```

**Arguments**

- `void (*cb)(GLFWwindow *, int, int)`: Framebuffer size callback function to be registered to the current window

**Returns**

0 if successful, -1 if an error occured

```int register_mouse_movement_callback(void (*cb)(GLFWwindow *, double, double))```

**Arguments**

- `void (*cb)(GLFWwindow *, double, double)`: Mouse movement callback function to be registered to the current window

**Returns**

0 if successful, -1 if an error occured

```int register_scroll_callback(void (*cb)(GLFWwindow *, double, double))```

**Arguments**

- `void (*cb)(GLFWwindow *, double, double)`: Mouse scroll callback function to be registered to the current window

**Returns**

0 if successful, -1 if an error occured

```int register_mouse_button_callback(void (*cb)(GLFWwindow *, int, int, int))```

**Arguments**

- `void (*cb)(GLFWwindow *, int, int, int)`: Mouse button callback function to be registered to the current window

**Returns**

0 if successful, -1 if an error occured

