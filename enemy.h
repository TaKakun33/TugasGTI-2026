#pragma once
#include "config.h"

enum EnemyState { ES_IDLE, ES_CHASE, ES_ATTACK, ES_DEAD };

struct Enemy {
    float x, z;
    int hp;
    EnemyState state;
    int attackTimer;
    float deathTimer;
    float flashTimer;
};

extern Enemy enemies[MAX_ENEMIES];

void initEnemies();
void updateEnemies();
void shootCheck();
void drawEnemy(const Enemy &e);
void drawAllEnemies();

// sda