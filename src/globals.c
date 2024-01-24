#include <const.h>
#include <cglm/cglm.h>

/*
                                    GLOBALS.c
  Responsible for initializing the global variables that are used across
  multiple modules of the engine.
*/

float RES_X = BASE_RES_X;
float RES_Y = BASE_RES_Y;
vec2 MOUSE_POS = GLM_VEC2_ZERO_INIT;
int CURSOR_ENABLED = 1;
float DELTA_TIME = 0.0;
float LAST_FRAME = 0.0;
