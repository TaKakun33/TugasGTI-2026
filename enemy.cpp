#include "enemy.h"
#include "renderer.h"
#include "lantern.h"

extern bool lanternOn;

static int hasLineOfSight(float ax, float az, float bx, float bz) {
    float dx = bx - ax, dz = bz - az;
    float dist = std::sqrtf(dx*dx + dz*dz);
    int steps = (int)(dist / (CELL * 0.25f)) + 1;
    for (int i = 1; i < steps; i++) {
        float t  = (float)i / steps;
        float tx = ax + dx * t;
        float tz = az + dz * t;
        if (isWall((int)(tx/CELL), (int)(tz/CELL))) return 0;
    }
    return 1;
}

void initEnemies() {
    const float spawnX[] = {2.5f, 7.5f,13.5f, 2.5f,14.5f, 5.5f,11.5f, 2.5f, 8.5f,14.5f};
    const float spawnZ[] = {5.0f, 2.5f, 2.5f, 8.5f, 8.5f,13.5f,13.5f,14.5f,14.5f,14.5f};
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].x = spawnX[i] * CELL;
        enemies[i].z = spawnZ[i] * CELL;
        enemies[i].hp = ENEMY_HP_MAX;
        enemies[i].state = ES_IDLE;
        enemies[i].attackTimer = 0;
        enemies[i].deathTimer = 0.0f;
        enemies[i].flashTimer = 0.0f;
    }
}

#include "lantern.h"
extern bool lanternOn; // taro di atas file

void updateEnemies() {
    if (gameOver) return;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy &e = enemies[i];
        if (e.state == ES_DEAD) { e.deathTimer += 0.06f; continue; }
        if (e.flashTimer > 0) e.flashTimer -= 0.15f;
        if (e.attackTimer > 0) e.attackTimer--;

        float dx = camX - e.x, dz = camZ - e.z;
        float dist = std::sqrtf(dx*dx + dz*dz);

        // === SPEED DINAMIS ===
        float currentSpeed = ENEMY_SPEED;

        if (dist < ENEMY_SIGHT && hasLineOfSight(e.x, e.z, camX, camZ)) {
            if (dist < ENEMY_ATTACK_RANGE) {
                e.state = ES_ATTACK;
                if (e.attackTimer <= 0) {
                    playerHP--;
                    screenFlash = 1.0f;
                    e.attackTimer = ENEMY_ATTACK_COOLDOWN;
                    if (playerHP <= 0) { playerHP = 0; gameOver = 1; }
                }
            } else {
                e.state = ES_CHASE;
                float nx = e.x + (dx/dist) * currentSpeed; // pake currentSpeed
                float nz = e.z + (dz/dist) * currentSpeed; // pake currentSpeed
                if (!isWall((int)(nx/CELL), (int)(nz/CELL)))
                    e.x = nx, e.z = nz;
            }
        } else {
            e.state = ES_IDLE;
        }
    }
}

void shootCheck() {
    float rad  = DEG2RAD(angle);
    float dirX = std::cosf(rad), dirZ = std::sinf(rad);
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy &e = enemies[i];
        if (e.state == ES_DEAD) continue;
        float dx = e.x - camX, dz = e.z - camZ;
        float dist = std::sqrtf(dx*dx + dz*dz);
        if (dist > ENEMY_SIGHT) continue;
        float dot = (dx/dist)*dirX + (dz/dist)*dirZ;
        if (dot < 0.94f) continue;
        if (!hasLineOfSight(camX, camZ, e.x, e.z)) continue;
        e.hp--;
        e.flashTimer = 1.0f;
        if (e.hp <= 0) { e.state = ES_DEAD; e.deathTimer = 0.0f; }
        break;
    }
}

void drawEnemy(const Enemy &e) {
    if (e.state == ES_DEAD && e.deathTimer > 1.5f) return;
    float dx = camX - e.x, dz = camZ - e.z;
    float dist = std::sqrtf(dx*dx + dz*dz);
    if (dist < 0.01f) return;

    float rx = -dz / dist;
    float rz =  dx / dist;
    float topY = WALL_H * 0.95f;
    float botY = 0.02f;
    float halfW = 0.45f;

    if (e.state == ES_DEAD) {
        float t = e.deathTimer;
        topY = WALL_H * 0.95f * (1.0f - std::fminf(t, 1.0f));
        botY = 0.02f - std::fminf(t, 1.0f) * 0.5f;
        halfW = 0.45f + t * 0.3f;
    }

    float x0 = e.x - rx*halfW, z0 = e.z - rz*halfW;
    float x1 = e.x + rx*halfW, z1 = e.z + rz*halfW;

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Calculate alpha for death fade
    float alpha = 1.0f;
    if (e.state == ES_DEAD && e.deathTimer > 1.0f)
        alpha = 1.0f - (e.deathTimer - 1.0f);

    // Tint color based on enemy state (multiplied with texture)
    float flash = (e.flashTimer > 0) ? e.flashTimer : 0.0f;
    if (e.state == ES_DEAD)
        glColor4f(0.8f, 0.3f, 0.3f, alpha);
    else if (e.state == ES_ATTACK)
        glColor4f(1.0f, 0.3f + flash*0.2f, 0.3f, 1.0f);
    else if (e.state == ES_CHASE)
        glColor4f(1.0f, 0.6f + flash*0.2f, 0.3f, 1.0f);
    else
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // idle: full texture color, no tint

    // Draw textured sprite
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[TEX_ENEMY]);
    glBegin(GL_QUADS);
        glTexCoord2f(0,0); glVertex3f(x0, botY, z0);
        glTexCoord2f(1,0); glVertex3f(x1, botY, z1);
        glTexCoord2f(1,1); glVertex3f(x1, topY, z1);
        glTexCoord2f(0,1); glVertex3f(x0, topY, z0);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // HP bar (only when alive)
    if (e.state != ES_DEAD) {
        float barY = topY + 0.18f;
        float barW = halfW * 0.9f;
        float hpR  = (float)e.hp / ENEMY_HP_MAX;

        // Background
        glColor4f(0.4f, 0.05f, 0.05f, 0.85f);
        glBegin(GL_QUADS);
            glVertex3f(e.x-rx*barW,    barY,         e.z-rz*barW);
            glVertex3f(e.x+rx*barW,    barY,         e.z+rz*barW);
            glVertex3f(e.x+rx*barW,    barY+0.09f,   e.z+rz*barW);
            glVertex3f(e.x-rx*barW,    barY+0.09f,   e.z-rz*barW);
        glEnd();

        // Filled portion
        float filledW = barW*2.0f*hpR - barW;
        glColor4f(1.0f-hpR, hpR, 0.0f, 0.9f);
        glBegin(GL_QUADS);
            glVertex3f(e.x-rx*barW,    barY,         e.z-rz*barW);
            glVertex3f(e.x+rx*filledW, barY,         e.z+rz*filledW);
            glVertex3f(e.x+rx*filledW, barY+0.09f,   e.z+rz*filledW);
            glVertex3f(e.x-rx*barW,    barY+0.09f,   e.z-rz*barW);
        glEnd();
    }

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glLineWidth(1.0f);
}

void drawAllEnemies() {
    int order[MAX_ENEMIES];
    for (int i = 0; i < MAX_ENEMIES; i++) order[i] = i;
    // Sort back-to-front for correct transparency
    for (int i = 0; i < MAX_ENEMIES-1; i++)
    for (int j = i+1; j < MAX_ENEMIES; j++) {
        float d1 = (camX-enemies[order[i]].x)*(camX-enemies[order[i]].x)
                 + (camZ-enemies[order[i]].z)*(camZ-enemies[order[i]].z);
        float d2 = (camX-enemies[order[j]].x)*(camX-enemies[order[j]].x)
                 + (camZ-enemies[order[j]].z)*(camZ-enemies[order[j]].z);
        if (d2 > d1) {
            int t = order[i]; order[i] = order[j]; order[j] = t;
        }
    }
    for (int i = 0; i < MAX_ENEMIES; i++) drawEnemy(enemies[order[i]]);
}