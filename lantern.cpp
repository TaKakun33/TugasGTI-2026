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
void drawLantern() {
    static float t = 0; t += 0.02f;
    float pulse = 1.0f + 0.22f * sinf(t*2.0f);
    float flick = 0.8f + 0.2f * sinf(t*1.0f);
// === EFEK MODE MERAH ===
float currentRange = RANGE;
float currentBright = BRIGHTNESS;
    if (!lanternOn) {
        glDisable(GL_LIGHT0);
        return;
    }


    // === LIGHT ENTITY ===
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat pos[] = { camX, camY + 0.3f, camZ, 1.0f };

    // warna normal vs merah
    GLfloat diff[4], amb[4];
    diff[0] = 1.0f * flick * BRIGHTNESS; diff[1] = 0.93f * flick * BRIGHTNESS; diff[2] = 0.68f * flick * BRIGHTNESS; diff[3] = 1;
    amb[0] = 0.02f; amb[1] = 0.02f; amb[2] = 0.02f; amb[3] = 1;

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
    glColor3f(1.0f, 0.97f, 0.65f);
    glBegin(GL_TRIANGLES);
        glVertex3f(0,s,0); glVertex3f(-s,0,0); glVertex3f(0,-s,0);
        glVertex3f(0,s,0); glVertex3f(0,-s,0); glVertex3f(s,0,0);
    glEnd();
    glEnable(GL_DEPTH_TEST); glPopMatrix();

    // KACA
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    float glassAlpha = 0.02f + 0.02f * flick;
    glColor4f(0.65f + 0.15f*flick, 0.82f, 1.0f, glassAlpha);
    glBegin(GL_QUAD_STRIP); for(int i=0;i<=24;i++){float a=i*6.283f/24; float x=cosf(a)*r*0.95f,z=sinf(a)*r*0.95f;
        glVertex3f(x,0.015f,z); glVertex3f(x,h-0.015f,z);} glEnd();
    glDepthMask(GL_TRUE); glDisable(GL_BLEND);

    glEnable(GL_LIGHTING);
    glPopMatrix();
}