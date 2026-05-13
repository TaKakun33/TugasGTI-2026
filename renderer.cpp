#include "renderer.h"
#include "enemy.h"

void drawFloorCeiling() {
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

void drawWallCube(int mx, int mz) {
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

void drawMaze3D() {
    for (int z=0;z<MAP_H;z++)
    for (int x=0;x<MAP_W;x++)
    if (MAP[z][x]) drawWallCube(x,z);
}

void setupLighting() {
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

void setPerspectiveView(int w, int h) {
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(70.0,(double)w/h,0.1,100.0);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    float rad=DEG2RAD(angle);
    gluLookAt(camX,camY,camZ, camX+std::cosf(rad),camY,camZ+std::sinf(rad), 0,1,0);
}

void drawMinimap(int winW, int winH) {
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
    float ax=camX+std::cosf(pr)*0.45f,az=camZ+std::sinf(pr)*0.45f;
    float bx=camX+std::cosf(pr+2.4f)*0.25f,bz=camZ+std::sinf(pr+2.4f)*0.25f;
    float cx2=camX+std::cosf(pr-2.4f)*0.25f,cz2=camZ+std::sinf(pr-2.4f)*0.25f;
    glColor3f(1,0.85f,0.1f);
    glBegin(GL_TRIANGLES);glVertex2f(ax,az);glVertex2f(bx,bz);glVertex2f(cx2,cz2);glEnd();
    glEnable(GL_DEPTH_TEST);
}

void drawHUD(int winW, int winH) {
    glViewport(0,0,winW,winH);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glOrtho(0,winW,0,winH,-1,1);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    
    if(screenFlash >0){
        glColor4f(0.9f,0.05f,0.05f,screenFlash*0.45f);
        glBegin(GL_QUADS);glVertex2f(0,0);glVertex2f(winW,0);glVertex2f(winW,winH);glVertex2f(0,winH);glEnd();
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
    const char*hint= "[W/S] Maju/Mundur  [A/D] Putar  [M] Minimap  [SPACE/LMB] Tembak  [R] Restart  [ESC/Q] Keluar ";
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
