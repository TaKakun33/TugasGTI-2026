#include "lantern.h"
#include "renderer.h"
#include <cmath>

extern bool lanternOn;

void drawLantern() {
    if (!lanternOn) return;

    glPushMatrix();
    glLoadIdentity();
    // posisi yang kamu pakai
    glTranslatef(-0.20f, -0.18f, -0.35f);
    glRotatef(-10.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(12.0f, 0.0f, 0.0f, 1.0f);

    glDisable(GL_LIGHTING);

    float h = 0.24f;
    float r = 0.065f;

    // === 1. RING ATAS BAWAH + PEGANGAN (solid) ===
    glColor3f(0.08f, 0.08f, 0.08f);
    // ring bawah
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0;i<=24;i++){
        float a = i*2*3.14159f/24;
        glVertex3f(cosf(a)*(r*0.95f), 0, sinf(a)*(r*0.95f));
        glVertex3f(cosf(a)*r, 0.015f, sinf(a)*r);
    }
    glEnd();
    // ring atas
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0;i<=24;i++){
        float a = i*2*3.14159f/24;
        glVertex3f(cosf(a)*r, h-0.015f, sinf(a)*r);
        glVertex3f(cosf(a)*(r*0.95f), h, sinf(a)*(r*0.95f));
    }
    glEnd();
    // pegangan
    glLineWidth(3.0f);
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<=12;i++){
        float a = 3.14159f * i / 12.0f;
        glVertex3f(cosf(a)*r*0.7f, h + sinf(a)*r*0.7f, 0);
    }
    glEnd();

    // === 2. DIAMOND DULUAN (biar gak ketutup kaca) ===
    glPushMatrix();
    glTranslatef(0, h*0.5f, 0);
    
    static float t=0; t+=0.12f;
    float pulse = 1.0f + 0.12f * sinf(t);
    float s = 0.038f * pulse; // aku gedein dikit

    glDisable(GL_DEPTH_TEST); // PENTING: biar selalu di depan
    glColor3f(1.0f, 0.97f, 0.65f); // putih-kuning solid

    // diamond 2D solid
    glBegin(GL_TRIANGLES);
    glVertex3f(0, s, 0); glVertex3f(-s, 0, 0); glVertex3f(0, -s, 0);
    glVertex3f(0, s, 0); glVertex3f(0, -s, 0); glVertex3f(s, 0, 0);
    glEnd();

    glEnable(GL_DEPTH_TEST);
    glPopMatrix();

    // === 3. KACA TRANSPARAN (gambar terakhir) ===
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); // kaca tidak nulis depth

    glColor4f(0.6f, 0.8f, 1.0f, 0.18f);
    glBegin(GL_QUAD_STRIP);
    for(int i=0;i<=24;i++){
        float a = i*2*3.14159f/24;
        float x = cosf(a)*r*0.95f;
        float z = sinf(a)*r*0.95f;
        glVertex3f(x, 0.015f, z);
        glVertex3f(x, h-0.015f, z);
    }
    glEnd();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}
