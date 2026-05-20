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

// Konstanta Peta & Dunia
#define MAP_W 17
#define MAP_H 17
#define CELL  2.0f
#define WALL_H 2.4f
#define PI 3.14159265f
#define DEG2RAD(x) ((x) * PI / 180.0f)

// Konstanta Senjata
#define GUN_FIRE_FRAMES  8
#define GUN_COOLDOWN_MAX 15

// Konstanta Musuh
#define MAX_ENEMIES 10
#define ENEMY_HP_MAX 3
#define ENEMY_SPEED 0.022f
#define ENEMY_SIGHT 8.0f
#define ENEMY_ATTACK_RANGE 1.4f
#define ENEMY_ATTACK_COOLDOWN 60

// Deklarasi Peta
extern const int MAP[MAP_H][MAP_W];

// Variabel Global
extern float camX, camZ, camY, angle;
extern float moveSpeed, turnSpeed;
extern int showMap, WIN_W, WIN_H;
extern int gunFiring, gunFrame, gunCooldown;
extern int playerHP, playerMaxHP, gameOver;
extern float screenFlash;
extern bool keys[256];

// texture ids for textring i think. using stb_image from github which
#define TEX_WALL    0
#define TEX_FLOOR   1
#define TEX_CEILING 2
#define TEX_ENEMY   3
#define TEX_COUNT   4

extern GLuint textures[TEX_COUNT];