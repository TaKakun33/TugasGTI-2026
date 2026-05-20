#include "lantern.h"
#include "renderer.h"
#include "enemy.h" // butuh data musuh
#include <cmath>
#include <cstdlib>

extern bool lanternOn;
extern float camX, camY, camZ;

// ===== SETTING GAMPANG DI SINI =====
static float BRIGHTNESS = 1.0f; // 0.5 = redup, 1.5 = terang banget
static float RANGE = 9.0f; // jangkauan cahaya (blok)
static float RED_CHANCE = 0.004f; // 0.4% per frame saat musuh dekat
static float RED_DIST = 3.0f; // jarak trigger (dalam CELL)
bool redMode = false;

bool isLanternRedMode() { return redMode; }

void drawLantern() {
    //static bool redMode = false;
    static int redEnemy = -1;
    static float t = 0; t += 0.02f;
    float pulse = 1.0f + 0.22f * sinf(t*2.0f);
    float flick = 0.8f + 0.2f * sinf(t*1.0f);
// === EFEK MODE MERAH ===
float currentRange = RANGE;
float currentBright = BRIGHTNESS;

if (redMode) {
    // 1. Jarak cahaya berkurang 60%
    currentRange = RANGE * 0.4f;
    
}
    if (!lanternOn) {
        glDisable(GL_LIGHT0);
        redMode = false;
        return;
    }

    // === CEK MUSUH DEKAT ===
    if (!redMode) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].state == ES_DEAD) continue;
            float dx = enemies[i].x - camX;
            float dz = enemies[i].z - camZ;
            float dist = sqrtf(dx*dx + dz*dz);
            if (dist < RED_DIST * CELL) {
                if ((rand() / (float)RAND_MAX) < RED_CHANCE) {
                    redMode = true;
                    redEnemy = i;
                    break;
                }
            }
        }
    } else {
        // tetap merah sampai musuh pemicu mati
        if (redEnemy >= 0 && enemies[redEnemy].state == ES_DEAD) {
            redMode = false;
            redEnemy = -1;
        }
    }

    // === LIGHT ENTITY ===
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat pos[] = { camX, camY + 0.3f, camZ, 1.0f };

    // warna normal vs merah
    GLfloat diff[4], amb[4];
    if (redMode) {
        diff[0] = 0.5f * flick * BRIGHTNESS; 
	diff[1] = 0.0f * flick * BRIGHTNESS; 
	diff[2] = 0.0f * flick * BRIGHTNESS; 
	diff[3] = 1;
        amb[0] = 0.15f; amb[1] = 0.02f; amb[2] = 0.02f; amb[3] = 1;
    } else {
        diff[0] = 1.0f * flick * BRIGHTNESS; diff[1] = 0.93f * flick * BRIGHTNESS; diff[2] = 0.68f * flick * BRIGHTNESS; diff[3] = 1;
        amb[0] = 0.02f; amb[1] = 0.02f; amb[2] = 0.02f; amb[3] = 1;
    }

    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);

    // atenuasi = cara atur jangkauan
glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.5f);
glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, (0.8f / currentRange));
glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, (0.5f / (currentRange*currentRange)));
    // === GAMBAR LENTERA ===
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(-0.20f, -0.18f, -0.35f);
    glRotatef(-10,1,0,0); glRotatef(12,0,0,1);
    glDisable(GL_LIGHTING);

    float h=0.24f, r=0.065f;
    glColor3f(0.08f,0.08f,0.08f);
    // ring
    glBegin(GL_TRIANGLE_STRIP); for(int i=0;i<=24;i++){float a=i*6.283f/24;
        glVertex3f(cosf(a)*r*0.95f,0,sinf(a)*r*0.95f); glVertex3f(cosf(a)*r,0.015f,sinf(a)*r);} glEnd();
    glBegin(GL_TRIANGLE_STRIP); for(int i=0;i<=24;i++){float a=i*6.283f/24;
        glVertex3f(cosf(a)*r,h-0.015f,sinf(a)*r); glVertex3f(cosf(a)*r*0.95f,h,sinf(a)*r*0.95f);} glEnd();
    glBegin(GL_LINE_STRIP); for(int i=0;i<=12;i++){float a=3.14159f*i/12;
        glVertex3f(cosf(a)*r*0.7f, h+sinf(a)*r*0.7f,0);} glEnd();

    // DIAMOND
    glPushMatrix(); glTranslatef(0,h*0.5f,0);
    float s = 0.04f * pulse;
    glDisable(GL_DEPTH_TEST);
    if (redMode) glColor3f(0.5f, 0.0f, 0.0f);
    else glColor3f(1.0f, 0.97f, 0.65f);
    glBegin(GL_TRIANGLES);
        glVertex3f(0,s,0); glVertex3f(-s,0,0); glVertex3f(0,-s,0);
        glVertex3f(0,s,0); glVertex3f(0,-s,0); glVertex3f(s,0,0);
    glEnd();
    glEnable(GL_DEPTH_TEST); glPopMatrix();

    // KACA
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    float glassAlpha = 0.02f + 0.02f * flick;
    if (redMode) glColor4f(0.5f, 0.0f, 0.0f, glassAlpha);
    else glColor4f(0.65f + 0.15f*flick, 0.82f, 1.0f, glassAlpha);
    glBegin(GL_QUAD_STRIP); for(int i=0;i<=24;i++){float a=i*6.283f/24; float x=cosf(a)*r*0.95f,z=sinf(a)*r*0.95f;
        glVertex3f(x,0.015f,z); glVertex3f(x,h-0.015f,z);} glEnd();
    glDepthMask(GL_TRUE); glDisable(GL_BLEND);

    glEnable(GL_LIGHTING);
    glPopMatrix();
}
