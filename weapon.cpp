#include "weapon.h"
#include "enemy.h" // Untuk shootCheck

void drawPistol(bool shooting) {
    float recoilY = 0.0f;
    if (gunFiring) {
        float t = (float)gunFrame / GUN_FIRE_FRAMES;
        recoilY = std::sinf(t * PI) * 0.08f;
    }

    glPushMatrix();
    glTranslatef(0.55f, -0.55f + recoilY, 0.0f);

    float aimDX = 0.0f - 0.55f;
    float aimDY = 0.0f - (-0.55f + recoilY);
    float aimAngle = std::atan2f(aimDY, aimDX) * 180.0f / PI - 180.0f;
    glRotatef(aimAngle, 0.0f, 0.0f, 1.0f);

    glColor3f(0.8f, 0.6f, 0.4f);
    glBegin(GL_POLYGON);
        glVertex2f(0.1f, -0.4f); glVertex2f(0.8f, -0.8f);
        glVertex2f(0.8f, -0.2f); glVertex2f(0.3f,  0.0f);
    glEnd();

    glColor3f(0.3f, 0.2f, 0.1f);
    glBegin(GL_QUADS);
        glVertex2f(0.1f, -0.4f); glVertex2f(0.3f, -0.4f);
        glVertex2f(0.3f,  0.0f); glVertex2f(0.1f,  0.0f);
    glEnd();

    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_POLYGON);
        glVertex2f(0.05f, -0.1f); glVertex2f(0.1f,  -0.1f);
        glVertex2f(0.1f,  -0.2f); glVertex2f(0.05f, -0.15f);
    glEnd();

    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_QUADS);
        glVertex2f(-0.4f, 0.0f); glVertex2f( 0.35f, 0.0f);
        glVertex2f( 0.35f, 0.15f); glVertex2f(-0.4f,  0.15f);
    glEnd();

    if (shooting) {
        glColor3f(1.0f, 0.5f, 0.0f);
        glBegin(GL_TRIANGLES);
            glVertex2f(-0.4f,  0.075f); glVertex2f(-0.55f, 0.15f); glVertex2f(-0.5f,  0.075f);
            glVertex2f(-0.4f,  0.075f); glVertex2f(-0.65f, 0.075f); glVertex2f(-0.5f,  0.0f);
            glVertex2f(-0.4f,  0.075f); glVertex2f(-0.55f,-0.05f); glVertex2f(-0.5f,  0.075f);
        glEnd();

        glColor3f(0.8f, 0.8f, 0.2f);
        glBegin(GL_QUADS);
            glVertex2f(-0.75f, 0.05f); glVertex2f(-0.9f,  0.05f);
            glVertex2f(-0.9f,  0.1f); glVertex2f(-0.75f, 0.1f);
        glEnd();
    }
    glPopMatrix();
}

void drawGunOverlay(int winW, int winH) {
    glViewport(0, 0, winW, winH);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    float asp = (float)winW / winH;
    glOrtho(-asp, asp, -1.0f, 1.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);

    bool isShooting = (gunFiring != 0) && (gunFrame < GUN_FIRE_FRAMES);
    drawPistol(isShooting);

    glEnable(GL_DEPTH_TEST);
}

void triggerShoot() {
    if (gunCooldown <= 0) {
        gunFiring = 1; gunFrame = 0; gunCooldown = GUN_COOLDOWN_MAX;
        shootCheck();
    }
}