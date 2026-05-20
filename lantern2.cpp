#include "lantern.h"
#include "renderer.h"
#include <cmath>
#include <cstdlib> // Untuk rand() jika ingin efek kedip acak

extern bool lanternOn;
// extern float camX, camY, camZ, angle; // Tidak perlu lagi karena kita pakai View Space langsung

void drawLantern() {
    if (!lanternOn) return;

    glPushMatrix();
    
    // === POSISI DI TANGAN KIRI (VIEW SPACE) ===
    // Reset matrix agar menempel di layar, bukan di dunia 3D
    glLoadIdentity(); 
    
    // Offset: 
    // X: -0.20 (Kiri)
    // Y: -0.18 (Bawah)
    // Z: -0.25 (Maju sedikit ke dalam layar, jangan terlalu jauh biar nggak kelewat besar/tinggi)
    glTranslatef(-0.20f, -0.58f, -0.25f);
    
    // Rotasi natural saat dipegang
    glRotatef(-15.0f, 1.0f, 0.0f, 0.0f); // Miring ke belakang sedikit
    glRotatef(20.0f, 0.0f, 1.0f, 0.0f);  // Menghadap sedikit ke arah tengah layar

    // --- 1. BATANG OBOR (WOODEN HANDLE) ---
    // Matikan Color Material agar warna tidak terpengaruh lighting global/fog sebelumnya
    glDisable(GL_COLOR_MATERIAL);
    
    GLfloat wood_amb[] = {0.10f, 0.05f, 0.02f, 1.0f};
    GLfloat wood_diff[] = {0.45f, 0.25f, 0.10f, 1.0f}; // Coklat Kayu Stabil
    GLfloat wood_spec[] = {0.10f, 0.10f, 0.10f, 1.0f};
    
    glMaterialfv(GL_FRONT, GL_AMBIENT,  wood_amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  wood_diff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, wood_spec);

    // Gambar Batang Silinder Sederhana
    // Radius 0.025, Tinggi 0.5
    glBegin(GL_QUAD_STRIP);
    int slices = 8;
    for(int i=0; i<=slices; i++) {
        float theta = 2.0f * 3.14159265f * i / slices;
        float x = 0.025f * std::cosf(theta);
        float z = 0.025f * std::sinf(theta);
        glNormal3f(x, 0, z);
        glVertex3f(x, 0, z);       // Bawah
        glVertex3f(x, 0.5f, z);    // Atas
    }
    glEnd();
    
    // Tutup Atas Batang
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0,1,0); glVertex3f(0,0.5f,0);
    for(int i=0; i<=slices; i++) {
        float theta = 2.0f * 3.14159265f * i / slices;
        glVertex3f(0.025f*std::cosf(theta), 0.5f, 0.025f*std::sinf(theta));
    }
    glEnd();

    // --- 2. PEMBALUT BESI (IRON BAND) ---
    GLfloat iron_diff[] = {0.20f, 0.20f, 0.20f, 1.0f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, iron_diff);
    
    glPushMatrix();
    glTranslatef(0.0f, 0.45f, 0.0f); // Di ujung atas batang
    glScalef(1.2f, 0.1f, 1.2f);      // Lebih lebar dari batang
    glutSolidSphere(0.025f, 8, 8);   // Pakai sphere yang di-scale jadi cincin
    glPopMatrix();

    // --- 3. API (THE FLAME) ---
    // Efek Kedipan Sederhana
    static float flickerTime = 0.0f;
    flickerTime += 0.15f;
    // Nilai kedip antara 0.8 sampai 1.2
    float flickerVal = 0.9f + 0.1f * std::sinf(flickerTime * 10.0f) * std::cosf(flickerTime * 23.0f);

    glPushMatrix();
    glTranslatef(0.0f, 0.52f, 0.0f); // Di atas pembalut besi
    
    // Getaran Api Acak
    glRotatef((std::rand() % 10) - 5, 1.0f, 0.0f, 0.0f);
    glRotatef((std::rand() % 10) - 5, 0.0f, 0.0f, 1.0f);

    // Material Api: Emissive (Menyala Sendiri)
    GLfloat fire_emissive[] = {1.0f * flickerVal, 0.6f * flickerVal, 0.1f * flickerVal, 1.0f};
    glMaterialfv(GL_FRONT, GL_EMISSION, fire_emissive);
    
    // Matikan Lighting agar warna api murni sesuai emissive (tidak dipengaruhi shadow)
    glDisable(GL_LIGHTING); 

    // Warna Dasar Oranye
    glColor3f(1.0f, 0.7f, 0.2f); 
    // Bentuk Api: Cone
    glutSolidCone(0.035f * flickerVal, 0.12f * flickerVal, 6, 4); 

    // Inti Api (Putih/Kuning Terang)
    glColor3f(1.0f, 0.9f, 0.5f);
    glScalef(0.5f, 0.5f, 0.5f);
    glutSolidCone(0.03f, 0.1f, 4, 2);

    // Kembalikan State
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL); // Hidupkan kembali untuk objek lain
    
    // Reset Emission
    GLfloat zero[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_EMISSION, zero);
    
    glPopMatrix();
    glPopMatrix();
}
