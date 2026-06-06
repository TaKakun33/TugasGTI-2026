#pragma once

#ifdef _WIN32
#include <windows.h>
#include <GL/freeglut.h>
#elif defined(__APPLE__)
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>

// map & world constants
#define MAP_W 50
#define MAP_H 50
#define CELL  2.5f
#define WALL_H 3.0f
#define PI 3.14159265f
#define DEG2RAD(x) ((x) * PI / 180.0f)

// weapon constants
#define GUN_FIRE_FRAMES  8
#define GUN_COOLDOWN_MAX 15

// enemy constants
#define MAX_ENEMIES 20
#define ENEMY_HP_MAX 3
#define ENEMY_SPEED 0.15f
#define ENEMY_SIGHT 10.0f
#define ENEMY_ATTACK_RANGE 0.5f
#define ENEMY_ATTACK_COOLDOWN 120

// map declaration - no longer const, gets regenerated each game
extern int MAP[MAP_H][MAP_W];

// global variables
extern float camX, camZ, camY, angle;
extern float moveSpeed, turnSpeed;
extern int showMap, WIN_W, WIN_H;
extern int gunFiring, gunFrame, gunCooldown;
extern int playerHP, playerMaxHP, gameOver;
extern float screenFlash;
extern bool keys[256];

// texture IDs
#define TEX_WALL    0
#define TEX_FLOOR   1
#define TEX_CEILING 2
#define TEX_ENEMY   3
#define TEX_COUNT   4

extern GLuint textures[TEX_COUNT];