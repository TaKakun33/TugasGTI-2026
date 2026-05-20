#include "lantern.h"
#include "renderer.h"
#include <cmath>
#include <cstdlib>
#include <vector>

extern bool lanternOn;

// === SISTEM PARTIKEL API ===
struct FireParticle {
    float x, y, z; // Posisi lokal relatif ke ujung obor
    float vx, vy, vz; // Velocity
    float r, g, b, a; // Warna + alpha
    float size; // Ukuran
    float life; // Umur 1.0 -> 0.0
};

std::vector<FireParticle> particles;
float particleTimer = 0.0f;

void updateParticles(float deltaTime) {
    // 1. Spawn partikel baru di ujung obor
    particleTimer += deltaTime;
    if (particleTimer > 0.02f) { // Spawn tiap 0.02 detik = 50 partikel/detik
        particleTimer = 0.0f;
        for (int i = 0; i < 2; i++) { // 2 partikel per spawn
            FireParticle p;
            // Spawn random di area kecil ujung obor
            p.x = (rand() % 100 - 50) / 2000.0f;
            p.y = 0.0f;
            p.z = (rand() % 100 - 50) / 2000.0f;
            
            // Velocity: dominan ke atas + random goyang
            p.vx = (rand() % 100 - 50) / 500.0f;
            p.vy = 0.15f + (rand() % 50) / 1000.0f;
            p.vz = (rand() % 100 - 50) / 500.0f;
            
            // Warna: gradasi kuning -> oranye -> merah
            float colorVar = (rand() % 100) / 100.0f;
            p.r = 1.0f;
            p.g = 0.4f + colorVar * 0.5f; // 0.4 - 0.9
            p.b = 0.0f + colorVar * 0.2f; // 0.0 - 0.2
            p.a = 1.0f;
            
            p.size = 0.008f + (rand() % 8) / 2000.0f;
            p.life = 1.0f;
            particles.push_back(p);
        }
    }

    // 2. Update semua partikel
    for (int i = particles.size() - 1; i >= 0; i--) {
        FireParticle& p = particles[i];
        p.x += p.vx * deltaTime;
        p.y += p.vy * deltaTime;
        p.z += p.vz * deltaTime;
        
        p.life -= 1.8f * deltaTime; // Mati dalam ~0.55 detik
        p.a = p.life * p.life; // Fade out kuadratik biar smooth
        p.size *= 0.985f; // Makin mengecil
        
        // Efek angin/turbulensi kecil
        p.vx += (rand() % 100 - 50) / 8000.0f;
        p.vz += (rand() % 100 - 50) / 8000.0f;
        p.vy -= 0.02f * deltaTime; // Gravitasi dikit biar melengkung
        
        // Warna makin merah saat mau mati
        p.g *= 0.992f;
        
        if (p.life <= 0.0f) {
            particles.erase(particles.begin() + i);
        }
    }
}

void drawParticles() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive biar glow
    glDepthMask(GL_FALSE); // Biar transparan ga ganggu depth buffer

    glBegin(GL_QUADS);
    for (const auto& p : particles) {
        glColor4f(p.r * p.a, p.g * p.a, p.b * p.a, p.a); // Pre-multiply alpha
        
        float s = p.size;
        glVertex3f(p.x - s, p.y - s, p.z);
        glVertex3f(p.x + s, p.y - s, p.z);
        glVertex3f(p.x + s, p.y + s, p.z);
        glVertex3f(p.x - s, p.y + s, p.z);
    }
    glEnd();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

// === FUNGSI UTAMA OBOR ===
void drawLantern() {
    if (!lanternOn) return;

    glPushMatrix();
    
    // === POSISI DI TANGAN KIRI (VIEW SPACE) ===
    glLoadIdentity(); 
    glTranslatef(-0.20f, -0.58f, -0.25f);
    
    // Rotasi natural saat dipegang
    glRotatef(-15.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(20.0f, 0.0f, 1.0f, 0.0f);

    // --- 1. BATANG OBOR (WOODEN HANDLE) ---
    glDisable(GL_COLOR_MATERIAL);
    
    GLfloat wood_amb[] = {0.10f, 0.05f, 0.02f, 1.0f};
    GLfloat wood_diff[] = {0.45f, 0.25f, 0.10f, 1.0f};
    GLfloat wood_spec[] = {0.10f, 0.10f, 0.10f, 1.0f};
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, wood_amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, wood_diff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, wood_spec);

    glBegin(GL_QUAD_STRIP);
    int slices = 8;
    for(int i=0; i<=slices; i++) {
        float theta = 2.0f * 3.14159265f * i / slices;
        float x = 0.025f * std::cosf(theta);
        float z = 0.025f * std::sinf(theta);
        glNormal3f(x, 0, z);
        glVertex3f(x, 0, z); // Bawah
        glVertex3f(x, 0.5f, z); // Atas
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
    glTranslatef(0.0f, 0.45f, 0.0f);
    glScalef(1.2f, 0.1f, 1.2f);
    glutSolidSphere(0.025f, 8, 8);
    glPopMatrix();

    // --- 3. API + PARTICLE TRAIL ---
    static float flickerTime = 0.0f;
    flickerTime += 0.15f;
    float flickerVal = 0.9f + 0.1f * std::sinf(flickerTime * 10.0f) * std::cosf(flickerTime * 23.0f);

    glPushMatrix();
    glTranslatef(0.0f, 0.52f, 0.0f); // Posisi ujung obor
    
    // Getaran Api Acak
    glRotatef((std::rand() % 10) - 5, 1.0f, 0.0f, 0.0f);
    glRotatef((std::rand() % 10) - 5, 0.0f, 0.0f, 1.0f);

    // Update & gambar partikel trail dulu
    updateParticles(0.016f); // Anggap 60 FPS
    drawParticles();

    // Inti api solid biar ga bolong
    GLfloat fire_emissive[] = {1.0f * flickerVal, 0.6f * flickerVal, 0.1f * flickerVal, 1.0f};
    glMaterialfv(GL_FRONT, GL_EMISSION, fire_emissive);
    
    glDisable(GL_LIGHTING); 
    glColor3f(1.0f, 0.7f, 0.2f); 
    glutSolidCone(0.025f * flickerVal, 0.08f * flickerVal, 6, 4); 

    // Inti Api Putih/Kuning Terang
    glColor3f(1.0f, 0.9f, 0.5f);
    glScalef(0.5f, 0.5f, 0.5f);
    glutSolidCone(0.03f, 0.1f, 4, 2);

    // Reset state
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    
    GLfloat zero[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_EMISSION, zero);
    
    glPopMatrix();
    glPopMatrix();
}
