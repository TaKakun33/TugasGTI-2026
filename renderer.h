#pragma once
#include "config.h"

inline int isWall(int mx, int mz) {
    if (mx < 0 || mx >= MAP_W || mz < 0 || mz >= MAP_H) return 1;
    return MAP[mz][mx];
}
inline int worldToCell(float w) { return (int)(w / CELL); }

void drawFloorCeiling();
void drawWallCube(int mx, int mz);
void drawMaze3D();
void setupLighting();
void setPerspectiveView(int w, int h);
void drawMinimap(int winW, int winH);
void drawHUD(int winW, int winH);
int canMove(float nx, float nz);