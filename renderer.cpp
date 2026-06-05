#define STB_IMAGE_IMPLEMENTATION
#define GL_GLEXT_PROTOTYPES 1
#include "stb_image.h"
#include "renderer.h"
#include "enemy.h"
#include <cmath>
#include <cstring>
#include <cstdio>

extern bool lanternOn; // Mengakses variabel dari game.cpp

GLuint textures[TEX_COUNT];

static bool loadTexture(GLuint &texID, const char *path) {
    int w, h, channels;
    stbi_set_flip_vertically_on_load(1);
    unsigned char *data = stbi_load(path, &w, &h, &channels, 4);
    if (!data) {
        fprintf(stderr, "Failed to load: %s\n", path);
        return false;
    }
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Fixed: Using GLU to generate mipmaps
    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    stbi_image_free(data);
    return true;
}

void loadAllTextures() {
    loadTexture(textures[TEX_WALL],    "Wall_sprite.png");
    loadTexture(textures[TEX_FLOOR],   "Floor_sprite.png");
    loadTexture(textures[TEX_CEILING], "Ceiling_sprite.png");
    loadTexture(textures[TEX_ENEMY],   "Enemy_sprite.jpeg");
}

void drawFloorCeiling() {
    glEnable(GL_TEXTURE_2D);

    // flooring
    glBindTexture(GL_TEXTURE_2D, textures[TEX_FLOOR]);
    glColor3f(1,1,1);
    glNormal3f(0,1,0);
    glBegin(GL_QUADS);
        glTexCoord2f(0,0);          
        glVertex3f(0, 0, 0);
        glTexCoord2f(MAP_W,0);      
        glVertex3f(MAP_W*CELL, 0, 0);
        glTexCoord2f(MAP_W,MAP_H);  
        glVertex3f(MAP_W*CELL, 0, MAP_H*CELL);
        glTexCoord2f(0,MAP_H);      
        glVertex3f(0, 0, MAP_H*CELL);
    glEnd();

    // ceiling
    glBindTexture(GL_TEXTURE_2D, textures[TEX_CEILING]);
    glColor3f(1,1,1);
    glNormal3f(0,-1,0);
    glBegin(GL_QUADS);
        glTexCoord2f(0,0);          
        glVertex3f(0, WALL_H, 0);
        glTexCoord2f(MAP_W,0);      
        glVertex3f(MAP_W*CELL, WALL_H, 0);
        glTexCoord2f(MAP_W,MAP_H);  
        glVertex3f(MAP_W*CELL, WALL_H, MAP_H*CELL);
        glTexCoord2f(0,MAP_H);      
        glVertex3f(0, WALL_H, MAP_H*CELL);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void drawWallCube(int mx, int mz) {
    float x0 = mx*CELL, x1=x0+CELL;
    float z0 = mz*CELL, z1=z0+CELL;
    float y0 = 0.0f, y1=WALL_H;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[TEX_WALL]);
    glColor3f(1,1,1);

    if (!isWall(mx,mz-1)){
        glBegin(GL_QUADS);
            glNormal3f(0,0,-1);
            glTexCoord2f(0,0); 
            glVertex3f(x0,y0,z0);
            glTexCoord2f(1,0); 
            glVertex3f(x1,y0,z0);
            glTexCoord2f(1,1); 
            glVertex3f(x1,y1,z0);
            glTexCoord2f(0,1); 
            glVertex3f(x0,y1,z0);
        glEnd();
    }
    if (!isWall(mx,mz+1)){
        glBegin(GL_QUADS);
            glNormal3f(0,0,1);
            glTexCoord2f(0,0); 
            glVertex3f(x1,y0,z1);
            glTexCoord2f(1,0); 
            glVertex3f(x0,y0,z1);
            glTexCoord2f(1,1); 
            glVertex3f(x0,y1,z1);
            glTexCoord2f(0,1); 
            glVertex3f(x1,y1,z1);
        glEnd();
    }
    if (!isWall(mx-1,mz)){
        glBegin(GL_QUADS);
            glNormal3f(-1,0,0);
            glTexCoord2f(0,0); 
            glVertex3f(x0,y0,z1);
            glTexCoord2f(1,0); 
            glVertex3f(x0,y0,z0);
            glTexCoord2f(1,1); 
            glVertex3f(x0,y1,z0);
            glTexCoord2f(0,1); 
            glVertex3f(x0,y1,z1);
        glEnd();
    }
    if (!isWall(mx+1,mz)){
        glBegin(GL_QUADS);
            glNormal3f(1,0,0);
            glTexCoord2f(0,0); 
            glVertex3f(x1,y0,z0);
            glTexCoord2f(1,0); 
            glVertex3f(x1,y0,z1);
            glTexCoord2f(1,1); 
            glVertex3f(x1,y1,z1);
            glTexCoord2f(0,1); 
            glVertex3f(x1,y1,z0);
        glEnd();
    }

    // Wall top face (slightly darker tint)
    glColor3f(0.85f,0.85f,0.85f);
    glBegin(GL_QUADS);
        glNormal3f(0,1,0);
        glTexCoord2f(0,0); 
        glVertex3f(x0,y1,z0);
        glTexCoord2f(1,0); 
        glVertex3f(x1,y1,z0);
        glTexCoord2f(1,1); 
        glVertex3f(x1,y1,z1);
        glTexCoord2f(0,1); 
        glVertex3f(x0,y1,z1);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void drawMaze3D() {
    for (int z=0;z<MAP_H;z++)
    for (int x=0;x<MAP_W;x++)
    if (MAP[z][x]) drawWallCube(x,z);
}

void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Efek Kedipan Global (Sinkron dengan api di lantern.cpp)
    static float flickerTime = 0.0f;
    flickerTime += 0.1f;
    // Nilai antara 0.8 sampai 1.2
    float flicker = 0.9f + 0.1f * std::sinf(flickerTime * 10.0f) * std::cosf(flickerTime * 23.0f);

    GLfloat amb[] = {0.02f, 0.02f, 0.03f, 1.0f}; 
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);

    // Posisi Cahaya: Menempel di depan kamera
    float rad = DEG2RAD(angle);
    float forward = 0.8f; 
    GLfloat pos0[] = {
        camX + std::cosf(rad) * forward, 
        camY, 
        camZ + std::sinf(rad) * forward, 
        1.0f
    };
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);

    if (lanternOn) {
        // === OBOR NYALA (Oranye Berkedip) ===
        // Intensitas dikalikan dengan 'flicker'
        GLfloat diffOn[] = {1.2f * flicker, 0.8f * flicker, 0.3f * flicker, 1.0f}; 
        GLfloat specOn[] = {0.5f * flicker, 0.3f * flicker, 0.1f * flicker, 1.0f};
        
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffOn);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specOn);
        
        // Attenuation: Jarak pendek, cepat memudar (khas obor)
        glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.0f);
        glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.8f);
        glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 1.5f);
    } else {
        // === OBOR MATI (Gelap Total/Biru Bulan) ===
        GLfloat diffOff[] = {0.05f, 0.06f, 0.10f, 1.0f}; 
        GLfloat specOff[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffOff);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specOff);

        glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.0f);
        glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.2f);
        glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.1f);
    }

    glDisable(GL_LIGHT1);

    // Material Setup
    GLfloat ms[] = {0.1f, 0.1f, 0.1f, 1.0f};
    glMaterialfv(GL_FRONT, GL_SPECULAR, ms);
    glMateriali(GL_FRONT, GL_SHININESS, 4);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
}

void setPerspectiveView(int w, int h) {
    float rad = DEG2RAD(angle);

    glMatrixMode(GL_PROJECTION); 
    glLoadIdentity();
    gluPerspective(70.0,(double)w/h,0.1,100.0);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    gluLookAt(camX,camY,camZ, camX+std::cosf(rad),camY,camZ+std::sinf(rad), 0,1,0);
}

void drawMinimap(int winW, int winH) {
    int mw=160,mh=160,ox=winW-mw-8,oy=8;

    glViewport(ox,oy,mw,mh);
    glMatrixMode(GL_PROJECTION); 
    glLoadIdentity();
    glOrtho(0,MAP_W*CELL,MAP_H*CELL,0,-1,1);
    glMatrixMode(GL_MODELVIEW); 
    glLoadIdentity();
    glDisable(GL_LIGHTING); 
    glDisable(GL_DEPTH_TEST);

    glColor4f(0.1f,0.1f,0.08f,0.85f);
    glBegin(GL_QUADS);glVertex2f(0,0);
    glVertex2f(MAP_W*CELL,0);
    glVertex2f(MAP_W*CELL,MAP_H*CELL);
    glVertex2f(0,MAP_H*CELL);glEnd();

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
    float ax=camX+std::cosf(pr)*0.45f,az=camZ+std::sinf(pr)*0.45f;
    float bx=camX+std::cosf(pr+2.4f)*0.25f,bz=camZ+std::sinf(pr+2.4f)*0.25f;
    float cx2=camX+std::cosf(pr-2.4f)*0.25f,cz2=camZ+std::sinf(pr-2.4f)*0.25f;
    
    glColor3f(1,0.85f,0.1f);
    glBegin(GL_TRIANGLES);
        glVertex2f(ax,az);
        glVertex2f(bx,bz);
        glVertex2f(cx2,cz2);
    glEnd();
    
    glEnable(GL_DEPTH_TEST);
}

void drawHUD(int winW, int winH) {
    glViewport(0,0,winW,winH);
    glMatrixMode(GL_PROJECTION); 
    glLoadIdentity();
    glOrtho(0,winW,0,winH,-1,1);
    glMatrixMode(GL_MODELVIEW); 
    glLoadIdentity();
    glDisable(GL_LIGHTING); 
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    
    if(screenFlash >0){
        glColor4f(0.9f,0.05f,0.05f,screenFlash*0.45f);
        glBegin(GL_QUADS);
            glVertex2f(0,0);
            glVertex2f(winW,0);
            glVertex2f(winW,winH);
            glVertex2f(0,winH);
        glEnd();
    }

    float hpR=(float)playerHP/playerMaxHP;
    glColor4f(0.95f,0.85f,0.7f,1); glRasterPos2i(10,42);
    for(const char*c= "HP: ";*c;c++) glutBitmapCharacter(GLUT_BITMAP_8_BY_13,*c);
    glColor4f(0.25f,0.06f,0.06f,0.85f);
    glBegin(GL_QUADS);glVertex2f(40,36);glVertex2f(220,36);glVertex2f(220,56);glVertex2f(40,56);glEnd();
    glColor4f(1-hpR,hpR,0.05f,1);
    glBegin(GL_QUADS);glVertex2f(40,36);glVertex2f(40+180*hpR,36);glVertex2f(40+180*hpR,56);glVertex2f(40,56);glEnd();
    glColor4f(0.7f,0.7f,0.6f,1); glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);glVertex2f(40,36);glVertex2f(220,36);glVertex2f(220,56);glVertex2f(40,56);glEnd();

    int alive=0;
    for(int i=0; i <MAX_ENEMIES;i++) if(enemies[i].state!=ES_DEAD) alive++;
    char buf[64]; snprintf(buf,64, "Musuh: %d/%d ",alive,MAX_ENEMIES);
    glColor4f(1,0.7f,0.3f,1); glRasterPos2i(8,64);
    for(const char*c=buf;*c;c++) glutBitmapCharacter(GLUT_BITMAP_8_BY_13,*c);

    glColor4f(0.9f,0.9f,0.7f,1); glRasterPos2i(8,10);
    const char*hint= "[W/S] Maju/Mundur  [A/D] Putar [SPACE/LMB] Tembak  [R] Restart  [ESC/Q] Keluar ";
    for(const char*c=hint;*c;c++) glutBitmapCharacter(GLUT_BITMAP_8_BY_13,*c);

    int cx=winW/2,cy=winH/2;
    glColor3f(gunFiring && gunFrame <4?1.0f:1.0f, gunFiring && gunFrame <4?0.3f:1.0f, gunFiring && gunFrame <4?0.1f:0.6f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
        glVertex2i(cx-8,cy);glVertex2i(cx+8,cy);
        glVertex2i(cx,cy-8);glVertex2i(cx,cy+8);
    glEnd();

    if(gameOver){
        glColor4f(0,0,0,0.72f);
        glBegin(GL_QUADS);glVertex2f(0,0);glVertex2f(winW,0);glVertex2f(winW,winH);glVertex2f(0,winH);glEnd();
        glColor4f(1,0.15f,0.15f,1);
        const char*go= "== KAMU MATI ==  Tekan R untuk restart ";
        glRasterPos2i(winW/2-(int)std::strlen(go)*5,winH/2);
        for(const char*c=go;*c;c++) glutBitmapCharacter(GLUT_BITMAP_9_BY_15,*c);
    }
    if(alive==0 && !gameOver){
        glColor4f(0,0,0,0.62f);
        glBegin(GL_QUADS);glVertex2f(0,0);glVertex2f(winW,0);glVertex2f(winW,winH);glVertex2f(0,winH);glEnd();
        glColor4f(0.3f,1,0.4f,1);
        const char*win= "== SEMUA MUSUH DIKALAHKAN! ==  Tekan R untuk main lagi ";
        glRasterPos2i(winW/2-(int)std::strlen(win)*5,winH/2);
        for(const char*c=win;*c;c++) glutBitmapCharacter(GLUT_BITMAP_9_BY_15,*c);
    }

    glDisable(GL_BLEND); glLineWidth(1); glEnable(GL_DEPTH_TEST);
}

int canMove(float nx, float nz){
    float m=0.25f;
    return !isWall(worldToCell(nx-m),worldToCell(nz-m)) &&
           !isWall(worldToCell(nx+m),worldToCell(nz-m)) &&
           !isWall(worldToCell(nx-m),worldToCell(nz+m)) &&
           !isWall(worldToCell(nx+m),worldToCell(nz+m));
}

// ---------------------------------------------------------------------------
// Planar Shadow
// Projects the enemy billboard silhouette onto the floor (Y=0) using a fixed
// overhead light position.
// ---------------------------------------------------------------------------
void drawPlanarShadow(float ex, float ez, float halfW, float spriteTopY) {
    const float lx = camX;
    const float ly = WALL_H * 0.9f;
    const float lz = camZ;
    const float floorY = 0.01f;   

    float dx = camX - ex, dz = camZ - ez;
    float dist = std::sqrtf(dx*dx + dz*dz);
    if (dist < 0.01f) return;
    float rx = -dz / dist;
    float rz =  dx / dist;

    float cx[4] = { ex - rx*halfW, ex + rx*halfW, ex + rx*halfW, ex - rx*halfW };
    float cy[4] = { 0.02f,        0.02f,         spriteTopY,    spriteTopY    };
    float cz2[4] = { ez - rz*halfW, ez + rz*halfW, ez + rz*halfW, ez - rz*halfW };

    float px[4], pz[4];
    for (int i = 0; i < 4; i++) {
        float vx = cx[i] - lx;
        float vy = cy[i] - ly;
        float vz = cz2[i] - lz;
        if (std::fabsf(vy) < 0.001f) { px[i] = cx[i]; pz[i] = cz2[i]; continue; }
        float t = (floorY - ly) / vy;
        px[i] = lx + t * vx;
        pz[i] = lz + t * vz;
    }

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.0f, 0.0f, 0.0f, 0.38f);
    glBegin(GL_QUADS);
        glVertex3f(px[0], floorY, pz[0]);
        glVertex3f(px[1], floorY, pz[1]);
        glVertex3f(px[2], floorY, pz[2]);
        glVertex3f(px[3], floorY, pz[3]);
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}