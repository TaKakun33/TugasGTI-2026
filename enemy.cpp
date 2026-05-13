#include "enemy.h"
#include "renderer.h"

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

void updateEnemies() {
    if (gameOver) return;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy &e = enemies[i];
        if (e.state == ES_DEAD) { e.deathTimer += 0.06f; continue; }
        if (e.flashTimer  > 0) e.flashTimer  -= 0.15f;
        if (e.attackTimer > 0) e.attackTimer--;

        float dx = camX - e.x, dz = camZ - e.z;
        float dist = std::sqrtf(dx*dx + dz*dz);

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
                float nx = e.x + (dx/dist) * ENEMY_SPEED;
                float nz = e.z + (dz/dist) * ENEMY_SPEED;
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
    float flash = (e.flashTimer > 0) ? e.flashTimer : 0.0f;

    if (e.state == ES_DEAD)
        glColor4f(0.45f,0.08f,0.08f, 1.0f-(e.deathTimer >1.0f?e.deathTimer-1.0f:0.0f));
    else if (e.state == ES_ATTACK)
        glColor4f(0.9f+flash*0.1f, 0.1f, 0.1f, 1.0f);
    else if (e.state == ES_CHASE)
        glColor4f(0.7f+flash*0.3f, 0.2f+flash*0.2f, 0.1f, 1.0f);
    else
        glColor4f(0.4f, 0.55f, 0.35f, 1.0f);

    glBegin(GL_QUADS);
        glVertex3f(x0,botY,z0); glVertex3f(x1,botY,z1);
        glVertex3f(x1,topY*0.55f,z1); glVertex3f(x0,topY*0.55f,z0);
    glEnd();

    if (e.state != ES_DEAD) {
        glColor4f(flash >0.5f?1.0f:0.82f, flash >0.5f?0.3f:0.62f, flash >0.5f?0.3f:0.46f, 1.0f);
        float hHW = halfW*0.45f;
        float hx0=e.x-rx*hHW, hz0=e.z-rz*hHW, hx1=e.x+rx*hHW, hz1=e.z+rz*hHW;
        glBegin(GL_QUADS);
            glVertex3f(hx0,topY*0.55f,hz0); 
            glVertex3f(hx1,topY*0.55f,hz1);
            glVertex3f(hx1,topY,hz1);       
            glVertex3f(hx0,topY,hz0);
        glEnd();

        float eyeY = topY * 0.82f;
        glColor4f(1.0f,0.1f,0.1f,1.0f);
        glBegin(GL_QUADS);
            glVertex3f(e.x-rx*0.14f,eyeY-0.04f,e.z-rz*0.14f);
            glVertex3f(e.x-rx*0.06f,eyeY-0.04f,e.z-rz*0.06f);
            glVertex3f(e.x-rx*0.06f,eyeY+0.04f,e.z-rz*0.06f);
            glVertex3f(e.x-rx*0.14f,eyeY+0.04f,e.z-rz*0.14f);
        glEnd();
        glBegin(GL_QUADS);
            glVertex3f(e.x+rx*0.06f,eyeY-0.04f,e.z+rz*0.06f);
            glVertex3f(e.x+rx*0.14f,eyeY-0.04f,e.z+rz*0.14f);
            glVertex3f(e.x+rx*0.14f,eyeY+0.04f,e.z+rz*0.14f);
            glVertex3f(e.x+rx*0.06f,eyeY+0.04f,e.z+rz*0.06f);
        glEnd();
    }

    glColor4f(0.05f,0.05f,0.05f,0.9f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
        glVertex3f(x0,botY,z0); glVertex3f(x1,botY,z1);
        glVertex3f(x1,topY*0.55f,z1); glVertex3f(x0,topY*0.55f,z0);
    glEnd();

    if (e.state != ES_DEAD) {
        float barY = topY + 0.18f;
        float barW = halfW * 0.9f;
        float hpR  = (float)e.hp / ENEMY_HP_MAX;
        glColor4f(0.4f,0.05f,0.05f,0.85f);
        glBegin(GL_QUADS);
            glVertex3f(e.x-rx*barW,barY, e.z-rz*barW);
            glVertex3f(e.x+rx*barW,barY, e.z+rz*barW);
            glVertex3f(e.x+rx*barW,barY+0.09f, e.z+rz*barW);
            glVertex3f(e.x-rx*barW,barY+0.09f, e.z-rz*barW);
        glEnd();
        float filledW = barW*2.0f*hpR - barW;
        glColor4f(1.0f-hpR,  hpR, 0.0f, 0.9f);
        glBegin(GL_QUADS);
            glVertex3f(e.x-rx*barW, barY, e.z-rz*barW);
            glVertex3f(e.x+rx*filledW,barY, e.z+rz*filledW);
            glVertex3f(e.x+rx*filledW,barY+0.09f, e.z+rz*filledW);
            glVertex3f(e.x-rx*barW, barY+0.09f, e.z-rz*barW);
        glEnd();
    }

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glLineWidth(1.0f);
}

void drawAllEnemies() {
    int order[MAX_ENEMIES];
    for (int i = 0; i < MAX_ENEMIES; i++) order[i] = i;
    for (int i = 0; i < MAX_ENEMIES-1; i++)
    for (int j = i+1; j < MAX_ENEMIES; j++) {
        float d1 = (camX-enemies[order[i]].x)*(camX-enemies[order[i]].x) + (camZ-enemies[order[i]].z)*(camZ-enemies[order[i]].z);
        float d2 = (camX-enemies[order[j]].x)*(camX-enemies[order[j]].x) + (camZ-enemies[order[j]].z)*(camZ-enemies[order[j]].z);
        if (d2 > d1) {
            int t = order[i]; order[i] = order[j]; order[j] = t;
        }
    }
    for (int i = 0; i < MAX_ENEMIES; i++) drawEnemy(enemies[order[i]]);
}