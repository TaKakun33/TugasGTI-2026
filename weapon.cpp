#include "weapon.h"
#include "enemy.h"

void drawPistol(bool shooting) {
    float recoilZ = 0.0f;
    if (gunFiring) {
        float t = (float)gunFrame / GUN_FIRE_FRAMES;
        recoilZ = std::sinf(t * PI) * 0.04f;
    }

    glPushMatrix();

    // position the gun in bottom-right of view, pointing forward (into screen = -Z)
    glTranslatef(0.18f, -0.13f, -0.25f + recoilZ);

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // barrel of the gun
    glPushMatrix();
        glTranslatef(0.0f, 0.01f, -0.09f); // shift forward
        glColor3f(0.25f, 0.25f, 0.25f);
        glScalef(0.03f, 0.03f, 0.18f);     // thin, long in Z
        glutSolidCube(1.0f);
    glPopMatrix();

    // body of the gun
    glPushMatrix();
        glTranslatef(0.0f, 0.01f, 0.02f);
        glColor3f(0.18f, 0.18f, 0.18f);
        glScalef(0.055f, 0.055f, 0.12f);
        glutSolidCube(1.0f);
    glPopMatrix();

    // grip?
    glPushMatrix();
        glTranslatef(0.0f, -0.055f, 0.06f);
        glColor3f(0.22f, 0.15f, 0.10f);    // dark brown grip
        glScalef(0.045f, 0.09f, 0.05f);
        glutSolidCube(1.0f);
    glPopMatrix();

    // --- TRIGGER GUARD (small flat box) ---
    glPushMatrix();
        glTranslatef(0.0f, -0.022f, 0.025f);
        glColor3f(0.20f, 0.20f, 0.20f);
        glScalef(0.012f, 0.025f, 0.04f);
        glutSolidCube(1.0f);
    glPopMatrix();

    // muzzle
    if (shooting && gunFrame < 3) {
        glPushMatrix();
        glTranslatef(0.0f, 0.01f, -0.20f);   // at the tip of barrel
        glColor4f(1.0f, 0.75f, 0.1f, 0.9f);
        glScalef(0.06f, 0.06f, 0.06f);
        glutSolidSphere(1.0f, 6, 6);
        glPopMatrix();

        // Outer flash glow
        glPushMatrix();
        glTranslatef(0.0f, 0.01f, -0.21f);
        glColor4f(1.0f, 0.4f, 0.05f, 0.45f);
        glScalef(0.10f, 0.10f, 0.04f);
        glutSolidSphere(1.0f, 6, 6);
        glPopMatrix();
    }

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void drawGunOverlay(int winW, int winH) {
    glViewport(0, 0, winW, winH);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70.0, (double)winW / winH, 0.01, 10.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST); // draw on top of everything

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