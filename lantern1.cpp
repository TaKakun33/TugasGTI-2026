#include "lantern.h"
#include "renderer.h"
#include <cmath>

extern bool lanternOn;

static void drawSolidCylinder(float radius, float height, int slices, int stacks) {
    float step = height / stacks;
    for (int i = 0; i < stacks; i++) {
        float y0 = i * step, y1 = (i + 1) * step;
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; j++) {
            float ang = 2.0f * 3.14159265f * j / slices;
            float x = radius * std::cosf(ang), z = radius * std::sinf(ang);
            glNormal3f(x, 0.0f, z); glVertex3f(x, y0, z); glVertex3f(x, y1, z);
        }
        glEnd();
    }
    glBegin(GL_TRIANGLE_FAN); glNormal3f(0,1,0); glVertex3f(0,height,0);
    for (int j = 0; j <= slices; j++) { float ang = 2.0f * 3.14159265f * j / slices; glVertex3f(radius*std::cosf(ang), height, radius*std::sinf(ang)); }
    glEnd();
    glBegin(GL_TRIANGLE_FAN); glNormal3f(0,-1,0); glVertex3f(0,0,0);
    for (int j = slices; j >= 0; j--) { float ang = 2.0f * 3.14159265f * j / slices; glVertex3f(radius*std::cosf(ang), 0, radius*std::sinf(ang)); }
    glEnd();
}

void drawLantern() {
    if (!lanternOn) return;

    glPushMatrix();
    glLoadIdentity(); // Reset ke View Space agar menempel di layar
    
    // Posisi: Kiri (-X), Bawah (-Y), Maju ke dalam layar (-Z)
    glTranslatef(-0.22f, -0.14f, -0.5f);
    glRotatef(-10.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(15.0f, 0.0f, 1.0f, 0.0f);

    GLfloat metal_amb[]  = {0.15f, 0.12f, 0.10f, 1.0f};
    GLfloat metal_diff[] = {0.45f, 0.40f, 0.35f, 1.0f};
    GLfloat metal_spec[] = {0.35f, 0.35f, 0.35f, 1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT,  metal_amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  metal_diff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, metal_spec);

    drawSolidCylinder(0.045f, 0.05f, 12, 2); // Alas

    glPushMatrix(); glTranslatef(0,0.05,0);
    glDisable(GL_LIGHTING); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.2f, 0.25f, 0.3f, 0.35f);
    drawSolidCylinder(0.038f, 0.11f, 12, 1);
    glDisable(GL_BLEND); glEnable(GL_LIGHTING); glPopMatrix();

    glPushMatrix(); glTranslatef(0,0.09,0);
    GLfloat emissive[] = {1.0f, 0.7f, 0.2f, 1.0f};
    glMaterialfv(GL_FRONT, GL_EMISSION, emissive);
    glutSolidSphere(0.022f, 10, 10);
    GLfloat zero[] = {0,0,0,1}; glMaterialfv(GL_FRONT, GL_EMISSION, zero);
    glPopMatrix();

    glPushMatrix(); glTranslatef(0,0.16,0);
    drawSolidCylinder(0.055f, 0.035f, 12, 2);
    glTranslatef(0,0.03,0); glScalef(1,0.6,1); glutSolidSphere(0.055f, 12, 8);
    glPopMatrix();

    glPushMatrix(); glTranslatef(0,0.23,0); glRotatef(90,1,0,0);
    glutSolidTorus(0.006f, 0.035f, 6, 12);
    glPopMatrix();

    glPopMatrix();
}
