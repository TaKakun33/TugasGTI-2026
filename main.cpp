// include glut sesuai platform
#ifdef _WIN32
    #include <windows.h>
    #include <GL/freeglut.h>
#elif __APPLE__
    #include <GLUT/glut.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/glut.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>

// ukuran peta dan dunia 3D
#define MAP_W 17
#define MAP_H 17
#define CELL  2.0f
#define WALL_H 2.4f
#define PI 3.14159265f
#define DEG2RAD(x) ((x) * PI / 180.0f)

// peta 2D, 1 = dinding, 0 = lantai
static const int MAP[MAP_H][MAP_W] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,0,0,0,1,0,0,0,1,1,0,0,1},
    {1,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,1},
    {1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1},
    {1,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,1},
    {1,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,1},
    {1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1},
    {1,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,1},
    {1,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,1},
    {1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

// posisi dan arah kamera pemain
static float camX  = 1.5f * CELL;
static float camZ  = 1.5f * CELL;
static float camY  = WALL_H * 0.45f;
static float angle = 90.0f;
static float moveSpeed = 0.16f; 
static float turnSpeed  = 3.0f;

static int showMap = 1;
static int WIN_W = 960, WIN_H = 600;

// status dan cooldown senjata
static int gunFiring = 0;
static int gunFrame = 0;
static int gunCooldown = 0;
#define GUN_FIRE_FRAMES  8
#define GUN_COOLDOWN_MAX 15

// sistem musuh
#define MAX_ENEMIES 10  
#define ENEMY_HP_MAX 3
#define ENEMY_SPEED 0.022f
#define ENEMY_SIGHT 8.0f
#define ENEMY_ATTACK_RANGE 1.4f
#define ENEMY_ATTACK_COOLDOWN 60

enum EnemyState { ES_IDLE, ES_CHASE, ES_ATTACK, ES_DEAD };

struct Enemy {
    float x, z;
    int hp;
    EnemyState state;
    int attackTimer;
    float deathTimer;
    float flashTimer;
};

static Enemy enemies[MAX_ENEMIES];

// status hp pemain
static int playerHP = 5;
static int playerMaxHP = 5;
static float screenFlash = 0.0f;
static int gameOver = 0;

// held-keys array untuk smooth movement
static bool keys[256] = {};

// cek apakah sel (mx,mz) adalah dinding
static inline int isWall(int mx, int mz) {
    if (mx < 0 || mx >= MAP_W || mz < 0 || mz >= MAP_H) return 1;
    return MAP[mz][mx];
}

// konversi koordinat dunia ke indeks sel
static inline int worldToCell(float w) { return (int)(w / CELL); }

// cek garis pandang antara dua titik
static int hasLineOfSight(float ax, float az, float bx, float bz) {
    float dx = bx - ax, dz = bz - az;
    float dist = sqrtf(dx*dx + dz*dz);
    int steps = (int)(dist / (CELL * 0.25f)) + 1;
    for (int i = 1; i < steps; i++) {
        float t  = (float)i / steps;
        float tx = ax + dx * t;
        float tz = az + dz * t;
        if (isWall(worldToCell(tx), worldToCell(tz))) return 0;
    }
    return 1;
}

// inisialisasi posisi dan status awal musuh
static void initEnemies(void) {
    // 10 titik spawn tersebar di berbagai sudut peta
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

// update logika pergerakan dan serangan musuh
static void updateEnemies(void) {
    if (gameOver) return;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy &e = enemies[i];
        if (e.state == ES_DEAD) { e.deathTimer += 0.06f; continue; }

        if (e.flashTimer  > 0) e.flashTimer  -= 0.15f;
        if (e.attackTimer > 0) e.attackTimer--;

        float dx = camX - e.x, dz = camZ - e.z;
        float dist = sqrtf(dx*dx + dz*dz);

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
                if (!isWall(worldToCell(nx), worldToCell(nz)))
                    e.x = nx, e.z = nz;
            }
        } else {
            e.state = ES_IDLE;
        }
    }
}

// cek apakah tembakan mengenai musuh
static void shootCheck(void) {
    float rad  = DEG2RAD(angle);
    float dirX = cosf(rad), dirZ = sinf(rad);

    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy &e = enemies[i];
        if (e.state == ES_DEAD) continue;

        float dx = e.x - camX, dz = e.z - camZ;
        float dist = sqrtf(dx*dx + dz*dz);
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

// gambar musuh 2d yang selalu menghadap kamera 
static void drawEnemy(const Enemy &e) {
    if (e.state == ES_DEAD && e.deathTimer > 1.5f) return;

    float dx = camX - e.x, dz = camZ - e.z;
    float dist = sqrtf(dx*dx + dz*dz);
    if (dist < 0.01f) return;

    float rx = -dz / dist;
    float rz =  dx / dist;

    float topY = WALL_H * 0.95f;
    float botY = 0.02f;
    float halfW = 0.45f;

    if (e.state == ES_DEAD) {
        float t = e.deathTimer;
        topY = WALL_H * 0.95f * (1.0f - fminf(t, 1.0f));
        botY = 0.02f - fminf(t, 1.0f) * 0.5f;
        halfW = 0.45f + t * 0.3f;
    }

    float x0 = e.x - rx*halfW, z0 = e.z - rz*halfW;
    float x1 = e.x + rx*halfW, z1 = e.z + rz*halfW;

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float flash = (e.flashTimer > 0) ? e.flashTimer : 0.0f;

    if (e.state == ES_DEAD)
        glColor4f(0.45f,0.08f,0.08f, 1.0f-(e.deathTimer>1.0f?e.deathTimer-1.0f:0.0f));
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
        glColor4f(flash>0.5f?1.0f:0.82f, flash>0.5f?0.3f:0.62f, flash>0.5f?0.3f:0.46f, 1.0f);
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
        glColor4f(1.0f-hpR, hpR, 0.0f, 0.9f);
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

// urutkan musuh dari yang terjauh ke terdekat lalu gambar
static void drawAllEnemies(void) {
    int order[MAX_ENEMIES];
    for (int i = 0; i < MAX_ENEMIES; i++) order[i] = i;
    for (int i = 0; i < MAX_ENEMIES-1; i++)
        for (int j = i+1; j < MAX_ENEMIES; j++) {
            float d1 = (camX-enemies[order[i]].x)*(camX-enemies[order[i]].x) + (camZ-enemies[order[i]].z)*(camZ-enemies[order[i]].z);
            float d2 = (camX-enemies[order[j]].x)*(camX-enemies[order[j]].x) + (camZ-enemies[order[j]].z)*(camZ-enemies[order[j]].z);
            if (d2 > d1) { 
                int t = order[i]; 
                order[i] = order[j]; 
                order[j] = t; }
        }
    for (int i = 0; i < MAX_ENEMIES; i++) drawEnemy(enemies[order[i]]);
}

// gambar pistol pemain
static void drawPistol(bool shooting) {

    // efek recoil saat menembak
    float recoilY = 0.0f;
    if (gunFiring) {
        float t = (float)gunFrame / GUN_FIRE_FRAMES;
        recoilY = sinf(t * PI) * 0.08f;
    }

    glPushMatrix();
    
    // geser ke kanan-bawah layar
    glTranslatef(0.55f, -0.55f + recoilY, 0.0f);

    // hitung sudut dari posisi pistol ke crosshair di tengah layar (0,0)
    // laras pistol mengarah ke kiri (-X), jadi offset sudut 180 derajat
    float aimDX = 0.0f - 0.55f;
    float aimDY = 0.0f - (-0.55f + recoilY);
    float aimAngle = atan2f(aimDY, aimDX) * 180.0f / PI - 180.0f;
    glRotatef(aimAngle, 0.0f, 0.0f, 1.0f);

    // gambar tangan
    glColor3f(0.8f, 0.6f, 0.4f);
    glBegin(GL_POLYGON);
        glVertex2f(0.1f, -0.4f);
        glVertex2f(0.8f, -0.8f);
        glVertex2f(0.8f, -0.2f);
        glVertex2f(0.3f,  0.0f);
    glEnd();

    // gambar pegangan pistol
    glColor3f(0.3f, 0.2f, 0.1f);
    glBegin(GL_QUADS);
        glVertex2f(0.1f, -0.4f);
        glVertex2f(0.3f, -0.4f);
        glVertex2f(0.3f,  0.0f);
        glVertex2f(0.1f,  0.0f);
    glEnd();

    // gambar pelatuk
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_POLYGON);
        glVertex2f(0.05f, -0.1f);
        glVertex2f(0.1f,  -0.1f);
        glVertex2f(0.1f,  -0.2f);
        glVertex2f(0.05f, -0.15f);
    glEnd();

    // gambar laras pistol
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_QUADS);
        glVertex2f(-0.4f, 0.0f);
        glVertex2f( 0.35f, 0.0f);
        glVertex2f( 0.35f, 0.15f);
        glVertex2f(-0.4f,  0.15f);
    glEnd();

    // efek saat menembak
    if (shooting) {
        // gambar percikan api
        glColor3f(1.0f, 0.5f, 0.0f);
        glBegin(GL_TRIANGLES);
            glVertex2f(-0.4f,  0.075f);
            glVertex2f(-0.55f, 0.15f);
            glVertex2f(-0.5f,  0.075f);
            
            glVertex2f(-0.4f,  0.075f);
            glVertex2f(-0.65f, 0.075f);
            glVertex2f(-0.5f,  0.0f);
            
            glVertex2f(-0.4f,  0.075f);
            glVertex2f(-0.55f,-0.05f);
            glVertex2f(-0.5f,  0.075f);
        glEnd();

        // gambar peluru
        glColor3f(0.8f, 0.8f, 0.2f);
        glBegin(GL_QUADS);
            glVertex2f(-0.75f, 0.05f);
            glVertex2f(-0.9f,  0.05f);
            glVertex2f(-0.9f,  0.1f);
            glVertex2f(-0.75f, 0.1f);
        glEnd();
    }

    glPopMatrix();
}

// setup layar untuk menggambar pistol 2d di atas 3d
static void drawGunOverlay(int winW, int winH) {
    glViewport(0, 0, winW, winH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    float asp = (float)winW / winH;
    glOrtho(-asp, asp, -1.0f, 1.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    bool isShooting = (gunFiring != 0) && (gunFrame < GUN_FIRE_FRAMES);
    drawPistol(isShooting);

    glEnable(GL_DEPTH_TEST);
}

// gambar lantai dan langit-langit sebagai satu quad besar
static void drawFloorCeiling(void) {
    glBegin(GL_QUADS);
        glColor3f(0.76f,0.72f,0.58f); glNormal3f(0,1,0);
        glVertex3f(0,0,0); glVertex3f(MAP_W*CELL,0,0);
        glVertex3f(MAP_W*CELL,0,MAP_H*CELL); glVertex3f(0,0,MAP_H*CELL);
    glEnd();
    glBegin(GL_QUADS);
        glColor3f(0.92f,0.90f,0.82f); glNormal3f(0,-1,0);
        glVertex3f(0,WALL_H,0); glVertex3f(MAP_W*CELL,WALL_H,0);
        glVertex3f(MAP_W*CELL,WALL_H,MAP_H*CELL); glVertex3f(0,WALL_H,MAP_H*CELL);
    glEnd();
}

// gambar satu kubus dinding di sel (mx, mz)
static void drawWallCube(int mx, int mz) {
    float x0=mx*CELL, x1=x0+CELL;
    float z0=mz*CELL, z1=z0+CELL;
    float y0=0.0f, y1=WALL_H;
    glColor3f(0.82f,0.82f,0.68f);
    if (!isWall(mx,mz-1)){glBegin(GL_QUADS);glNormal3f(0,0,-1);glVertex3f(x0,y0,z0);glVertex3f(x1,y0,z0);glVertex3f(x1,y1,z0);glVertex3f(x0,y1,z0);glEnd();}
    if (!isWall(mx,mz+1)){glBegin(GL_QUADS);glNormal3f(0,0,1); glVertex3f(x1,y0,z1);glVertex3f(x0,y0,z1);glVertex3f(x0,y1,z1);glVertex3f(x1,y1,z1);glEnd();}
    if (!isWall(mx-1,mz)){glBegin(GL_QUADS);glNormal3f(-1,0,0);glVertex3f(x0,y0,z1);glVertex3f(x0,y0,z0);glVertex3f(x0,y1,z0);glVertex3f(x0,y1,z1);glEnd();}
    if (!isWall(mx+1,mz)){glBegin(GL_QUADS);glNormal3f(1,0,0); glVertex3f(x1,y0,z0);glVertex3f(x1,y0,z1);glVertex3f(x1,y1,z1);glVertex3f(x1,y1,z0);glEnd();}
    glColor3f(0.75f,0.75f,0.62f);
    glBegin(GL_QUADS);glNormal3f(0,1,0);glVertex3f(x0,y1,z0);glVertex3f(x1,y1,z0);glVertex3f(x1,y1,z1);glVertex3f(x0,y1,z1);glEnd();
}

// iterasi semua sel peta, gambar dinding yang ada
static void drawMaze3D(void) {
    for (int z=0;z<MAP_H;z++)
        for (int x=0;x<MAP_W;x++)
            if (MAP[z][x]) drawWallCube(x,z);
}

// setup pencahayaan
static void setupLighting(void) {
    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0); glEnable(GL_LIGHT1);
    GLfloat amb[]  ={0.35f,0.33f,0.22f,1};
    GLfloat d0[]   ={0.80f,0.76f,0.55f,1};
    GLfloat s0[]   ={0.20f,0.20f,0.15f,1};
    GLfloat pos0[] ={camX, WALL_H*0.9f, camZ, 1};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,amb);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,d0); glLightfv(GL_LIGHT0,GL_SPECULAR,s0);
    glLightfv(GL_LIGHT0,GL_POSITION,pos0);
    glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION,0.08f);
    GLfloat d1[]  ={0.25f,0.25f,0.20f,1};
    GLfloat pos1[]={0,1,0,0};
    glLightfv(GL_LIGHT1,GL_DIFFUSE,d1); glLightfv(GL_LIGHT1,GL_POSITION,pos1);
    GLfloat ms[]={0.1f,0.1f,0.1f,1};
    glMaterialfv(GL_FRONT,GL_SPECULAR,ms); glMateriali(GL_FRONT,GL_SHININESS,8);
    glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
}

// set proyeksi perspektif dan posisi kamera first-person
static void setPerspectiveView(int w, int h) {
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(70.0,(double)w/h,0.1,100.0);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    float rad=DEG2RAD(angle);
    gluLookAt(camX,camY,camZ, camX+cosf(rad),camY,camZ+sinf(rad), 0,1,0);
}

// gambar minimap 2d di sudut kanan atas
static void drawMinimap(int winW, int winH) {
    int mw=160,mh=160,ox=winW-mw-8,oy=8;
    glViewport(ox,oy,mw,mh);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glOrtho(0,MAP_W*CELL,MAP_H*CELL,0,-1,1);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);

    glColor4f(0.1f,0.1f,0.08f,0.85f);
    glBegin(GL_QUADS);glVertex2f(0,0);glVertex2f(MAP_W*CELL,0);glVertex2f(MAP_W*CELL,MAP_H*CELL);glVertex2f(0,MAP_H*CELL);glEnd();

    for(int z=0;z<MAP_H;z++) for(int x=0;x<MAP_W;x++){
        float fx=x*CELL,fz=z*CELL;
        glColor3f(MAP[z][x]?0.55f:0.22f, MAP[z][x]?0.55f:0.22f, MAP[z][x]?0.42f:0.18f);
        glBegin(GL_QUADS);glVertex2f(fx,fz);glVertex2f(fx+CELL,fz);glVertex2f(fx+CELL,fz+CELL);glVertex2f(fx,fz+CELL);glEnd();
    }

    for(int i=0;i<MAX_ENEMIES;i++){
        if(enemies[i].state==ES_DEAD) continue;
        float ex=enemies[i].x,ez=enemies[i].z,sz=0.25f;
        glColor3f(enemies[i].state==ES_CHASE||enemies[i].state==ES_ATTACK?1.0f:0.3f,
                  enemies[i].state==ES_CHASE||enemies[i].state==ES_ATTACK?0.2f:0.9f,0.2f);
        glBegin(GL_QUADS);glVertex2f(ex-sz,ez-sz);glVertex2f(ex+sz,ez-sz);glVertex2f(ex+sz,ez+sz);glVertex2f(ex-sz,ez+sz);glEnd();
    }

    float pr=DEG2RAD(angle);
    float ax=camX+cosf(pr)*0.45f,az=camZ+sinf(pr)*0.45f;
    float bx=camX+cosf(pr+2.4f)*0.25f,bz=camZ+sinf(pr+2.4f)*0.25f;
    float cx2=camX+cosf(pr-2.4f)*0.25f,cz2=camZ+sinf(pr-2.4f)*0.25f;
    glColor3f(1,0.85f,0.1f);
    glBegin(GL_TRIANGLES);glVertex2f(ax,az);glVertex2f(bx,bz);glVertex2f(cx2,cz2);glEnd();
    glEnable(GL_DEPTH_TEST);
}

// gambar ui seperti hp, sisa musuh, dan crosshair
static void drawHUD(int winW, int winH) {
    glViewport(0,0,winW,winH);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glOrtho(0,winW,0,winH,-1,1);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    // efek layar merah saat kena hit
    if(screenFlash>0){
        glColor4f(0.9f,0.05f,0.05f,screenFlash*0.45f);
        glBegin(GL_QUADS);glVertex2f(0,0);glVertex2f(winW,0);glVertex2f(winW,winH);glVertex2f(0,winH);glEnd();
    }

    // bar hp pemain
    float hpR=(float)playerHP/playerMaxHP;
    glColor4f(0.95f,0.85f,0.7f,1); 
    glRasterPos2i(10,42);
    for(const char*c="HP:";*c;c++) glutBitmapCharacter(GLUT_BITMAP_8_BY_13,*c);

    glColor4f(0.25f,0.06f,0.06f,0.85f);
        glBegin(GL_QUADS);
        glVertex2f(40,36);
        glVertex2f(220,36);
        glVertex2f(220,56);
    glVertex2f(40,56);
    glEnd();

    glColor4f(1-hpR,hpR,0.05f,1);
    glBegin(GL_QUADS);
        glVertex2f(40,36);
        glVertex2f(40+180*hpR,36);
        glVertex2f(40+180*hpR,56);
        glVertex2f(40,56);
    glEnd();

    glColor4f(0.7f,0.7f,0.6f,1); 
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(40,36);
        glVertex2f(220,36);
        glVertex2f(220,56);
        glVertex2f(40,56);
    glEnd();

    // jumlah musuh tersisa
    int alive=0;
    for(int i=0;i<MAX_ENEMIES;i++) if(enemies[i].state!=ES_DEAD) alive++;
    char buf[64]; snprintf(buf,64,"Musuh: %d/%d",alive,MAX_ENEMIES);
    glColor4f(1,0.7f,0.3f,1); glRasterPos2i(8,64);
    for(const char*c=buf;*c;c++) glutBitmapCharacter(GLUT_BITMAP_8_BY_13,*c);

    // teks bantuan kontrol
    glColor4f(0.9f,0.9f,0.7f,1); glRasterPos2i(8,10);
    const char*hint="[W/S] Maju/Mundur  [A/D] Putar  [M] Minimap  [SPACE/LMB] Tembak  [R] Restart  [ESC/Q] Keluar";
    for(const char*c=hint;*c;c++) glutBitmapCharacter(GLUT_BITMAP_8_BY_13,*c);

    // crosshair di tengah layar
    int cx=winW/2,cy=winH/2;
    glColor3f(gunFiring&&gunFrame<4?1.0f:1.0f, gunFiring&&gunFrame<4?0.3f:1.0f, gunFiring&&gunFrame<4?0.1f:0.6f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
        glVertex2i(cx-8,cy);glVertex2i(cx+8,cy);
        glVertex2i(cx,cy-8);glVertex2i(cx,cy+8);
    glEnd();

    // layar game over
    if(gameOver){
        glColor4f(0,0,0,0.72f);
        glBegin(GL_QUADS);glVertex2f(0,0);glVertex2f(winW,0);glVertex2f(winW,winH);glVertex2f(0,winH);glEnd();
        glColor4f(1,0.15f,0.15f,1);
        const char*go="== KAMU MATI ==  Tekan R untuk restart";
        glRasterPos2i(winW/2-(int)strlen(go)*5,winH/2);
        for(const char*c=go;*c;c++) glutBitmapCharacter(GLUT_BITMAP_9_BY_15,*c);
    }
    // layar menang
    if(alive==0&&!gameOver){
        glColor4f(0,0,0,0.62f);
        glBegin(GL_QUADS);glVertex2f(0,0);glVertex2f(winW,0);glVertex2f(winW,winH);glVertex2f(0,winH);glEnd();
        glColor4f(0.3f,1,0.4f,1);
        const char*win="== SEMUA MUSUH DIKALAHKAN! ==  Tekan R untuk main lagi";
        glRasterPos2i(winW/2-(int)strlen(win)*5,winH/2);
        for(const char*c=win;*c;c++) glutBitmapCharacter(GLUT_BITMAP_9_BY_15,*c);
    }

    glDisable(GL_BLEND); glLineWidth(1); glEnable(GL_DEPTH_TEST);
}

// reset game ke kondisi awal
static void resetGame(void) {
    camX=1.5f*CELL; camZ=1.5f*CELL; angle=90;
    playerHP=playerMaxHP; screenFlash=0; gameOver=0;
    gunFiring=0; gunFrame=0; gunCooldown=0;
    // clear semua keys saat restart
    for(int i=0;i<256;i++) keys[i]=false;
    initEnemies();
}

// callback render utama
static void display(void) {
    glClearColor(0.62f,0.60f,0.48f,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glViewport(0,0,WIN_W,WIN_H);
    setupLighting();
    setPerspectiveView(WIN_W,WIN_H);

    glEnable(GL_FOG);
    GLfloat fogC[]={0.62f,0.60f,0.48f,1};
    glFogfv(GL_FOG_COLOR,fogC); glFogi(GL_FOG_MODE,GL_LINEAR);
    glFogf(GL_FOG_START,4); glFogf(GL_FOG_END,22);

    glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL);

    drawFloorCeiling();
    drawMaze3D();
    drawAllEnemies();

    drawGunOverlay(WIN_W, WIN_H);

    if(showMap) drawMinimap(WIN_W,WIN_H);
    glViewport(0,0,WIN_W,WIN_H);
    drawHUD(WIN_W,WIN_H);

    glutSwapBuffers();
}

// update ukuran layar
static void reshape(int w,int h){WIN_W=w;WIN_H=(h>0?h:1);glViewport(0,0,WIN_W,WIN_H);}

// cek 4 titik sudut aabb pemain agar tidak nembus dinding
static int canMove(float nx,float nz){
    float m=0.25f;
    return !isWall(worldToCell(nx-m),worldToCell(nz-m))&&
           !isWall(worldToCell(nx+m),worldToCell(nz-m))&&
           !isWall(worldToCell(nx-m),worldToCell(nz+m))&&
           !isWall(worldToCell(nx+m),worldToCell(nz+m));
}

// fungsi untuk memicu tembakan
static void triggerShoot(void){
    if(gunCooldown<=0){
        gunFiring=1; gunFrame=0; gunCooldown=GUN_COOLDOWN_MAX;
        shootCheck();
    }
}

// input keyboard  hanya catat tombol ditekan, tidak langsung gerak
static void keyboard(unsigned char key,int,int){
    keys[key] = true;
    // keluar game dengan tombol ESC atau Q
    if(key==27||key=='q'||key=='Q'){exit(0);}
    if(key=='r'||key=='R'){resetGame();return;}
    if(key=='m'||key=='M') showMap=!showMap;
    if(key==' '&&!gameOver) triggerShoot();
}

// catat tombol dilepas untuk smooth movement
static void keyboardUp(unsigned char key,int,int){
    keys[key] = false;
}

// klik kiri untuk menembak
static void mouseClick(int button,int state,int,int){
    if(button==GLUT_LEFT_BUTTON&&state==GLUT_DOWN&&!gameOver) triggerShoot();
}

// timer untuk update frame, animasi, dan proses movement dari held keys
static void timer(int){

    if(!gameOver){
        float rad=DEG2RAD(angle);
        float nx=camX, nz=camZ;
        if(keys['w']||keys['W']){ nx+=cosf(rad)*moveSpeed; nz+=sinf(rad)*moveSpeed; }
        if(keys['s']||keys['S']){ nx-=cosf(rad)*moveSpeed; nz-=sinf(rad)*moveSpeed; }
        if(keys['a']||keys['A']) angle-=turnSpeed;
        if(keys['d']||keys['D']) angle+=turnSpeed;
        if(canMove(nx,nz)){ camX=nx; camZ=nz; }
    }

    if(gunFiring){gunFrame++;if(gunFrame>=GUN_FIRE_FRAMES)gunFiring=0;}
    if(gunCooldown>0) gunCooldown--;
    if(screenFlash>0){screenFlash-=0.04f;if(screenFlash<0)screenFlash=0;}
    updateEnemies();
    glutPostRedisplay();
    glutTimerFunc(16,timer,0);
}

// inisialisasi glut dan mulai loop utama
int main(int argc,char**argv){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
    glutInitWindowSize(WIN_W,WIN_H);
    glutCreateWindow("GTIMazeShooter");
    camX=1.5f*CELL; camZ=1.5f*CELL; angle=90;
    glEnable(GL_DEPTH_TEST); glShadeModel(GL_SMOOTH); glEnable(GL_NORMALIZE);
    initEnemies();
    glutDisplayFunc(display); glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp); 
    glutMouseFunc(mouseClick);
    glutTimerFunc(16,timer,0);
    glutMainLoop();
    return 0;
}
