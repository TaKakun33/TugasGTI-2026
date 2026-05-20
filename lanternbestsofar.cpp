#include "lantern.h"
#include "renderer.h"
#include <cmath>

extern bool lanternOn;
extern float camX, camY, camZ; // dari renderer

void drawLantern() {
    if (!lanternOn) {
        glDisable(GL_LIGHT0);
        return;
    }

    // === LIGHT ENTITY - ngikutin player instant ===
    static float t = 0; t += 0.12f;
    float pulse = 1.0f + 0.12f * sinf(t);          // untuk diamond
    float flick = 0.85f + 0.15f * sinf(t*2.3f);    // untuk cahaya

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat pos[] = { camX, camY + 0.35f, camZ, 1.0f };
    GLfloat diff[] = { 1.0f*flick, 0.93f*flick, 0.68f*flick, 1.0f };
    GLfloat amb[]  = { 0.18f*flick, 0.14f*flick, 0.09f*flick, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.4f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.09f);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.018f);

    // === GAMBAR LENTERA DI TANGAN ===
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(-0.20f, -0.18f, -0.35f);
    glRotatef(-10.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(12.0f, 0.0f, 0.0f, 1.0f);

    glDisable(GL_LIGHTING);

    float h = 0.24f;
    float r = 0.065f;

    // 1. RING ATAS BAWAH + PEGANGAN
    glColor3f(0.08f, 0.08f, 0.08f);
    glBegin(GL_TRIANGLE_STRIP); // bawah
    for(int i=0;i<=24;i++){ float a=i*2*3.14159f/24;
        glVertex3f(cosf(a)*(r*0.95f),0,sinf(a)*(r*0.95f));
        glVertex3f(cosf(a)*r,0.015f,sinf(a)*r); }
    glEnd();
    glBegin(GL_TRIANGLE_STRIP); // atas
    for(int i=0;i<=24;i++){ float a=i*2*3.14159f/24;
        glVertex3f(cosf(a)*r,h-0.015f,sinf(a)*r);
        glVertex3f(cosf(a)*(r*0.95f),h,sinf(a)*(r*0.95f)); }
    glEnd();
    glLineWidth(3.0f);
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<=12;i++){ float a=3.14159f*i/12.0f;
        glVertex3f(cosf(a)*r*0.7f, h + sinf(a)*r*0.7f, 0); }
    glEnd();

    // 2. DIAMOND (digambar duluan biar gak ketutup)
    glPushMatrix();
    glTranslatef(0, h*0.5f, 0);
    float s = 0.038f * pulse;
    glDisable(GL_DEPTH_TEST);
    glColor3f(1.0f, 0.97f, 0.65f);
    glBegin(GL_TRIANGLES);
        glVertex3f(0,s,0); glVertex3f(-s,0,0); glVertex3f(0,-s,0);
        glVertex3f(0,s,0); glVertex3f(0,-s,0); glVertex3f(s,0,0);
    glEnd();
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();

    // 3. KACA - LEBIH TRANSPARAN + IKUT KEDIP
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    float glassAlpha = 0.04f + 0.03f * flick; // dulu 0.18, sekarang 0.04-0.07
    glColor4f(0.65f + 0.15f*flick, 0.82f + 0.1f*flick, 1.0f, glassAlpha);

    glBegin(GL_QUAD_STRIP);
    for(int i=0;i<=24;i++){ float a=i*2*3.14159f/24;
        float x = cosf(a)*r*0.95f, z = sinf(a)*r*0.95f;
        glVertex3f(x,0.015f,z); glVertex3f(x,h-0.015f,z); }
    glEnd();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}
