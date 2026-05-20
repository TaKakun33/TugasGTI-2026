#include "config.h"
#include "enemy.h"
#include "weapon.h"
#include "renderer.h"

// Definisi Variabel Global
const int MAP[MAP_H][MAP_W] = {
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

float camX  = 1.5f * CELL;
float camZ  = 1.5f * CELL;
float camY  = WALL_H * 0.45f;
float angle = 90.0f;
float moveSpeed = 0.16f;
float turnSpeed  = 3.0f;
int showMap = 1;
int WIN_W = 960, WIN_H = 600;
int gunFiring = 0;
int gunFrame = 0;
int gunCooldown = 0;
int playerHP    = 5;
int playerMaxHP = 5;
float screenFlash = 0.0f;
int gameOver    = 0;
bool keys[256] = {};
Enemy enemies[MAX_ENEMIES];

void resetGame() {
    camX=1.5f*CELL; camZ=1.5f*CELL; angle=90;
    playerHP=playerMaxHP; screenFlash=0; gameOver=0;
    gunFiring=0; gunFrame=0; gunCooldown=0;
    for(int i=0;i<256;i++) keys[i]=false;
    initEnemies();
}

void display() {
    glClearColor(0.62f, 0.60f, 0.48f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, WIN_W, WIN_H);

    setupLighting();
    setPerspectiveView(WIN_W, WIN_H);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FOG);
    GLfloat fogC[] = {0.62f, 0.60f, 0.48f, 1.0f};
    glFogfv(GL_FOG_COLOR, fogC); 
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 4); 
    glFogf(GL_FOG_END, 22);

    drawFloorCeiling();
    drawMaze3D();
    drawAllEnemies();
    drawGunOverlay(WIN_W, WIN_H);
    if(showMap) drawMinimap(WIN_W, WIN_H);
    drawHUD(WIN_W, WIN_H);

    glutSwapBuffers();
}

void reshape(int w, int h){ WIN_W=w; WIN_H=(h>0?h:1); glViewport(0,0,WIN_W,WIN_H); }

void keyboard(unsigned char key, int, int){
    keys[key] = true;
    if(key==27||key=='q'||key=='Q'){ exit(0); }
    if(key=='r'||key=='R'){ resetGame(); return; }
    if(key=='m'||key=='M') showMap=!showMap;
    if(key==' ' && !gameOver) triggerShoot();
}

void keyboardUp(unsigned char key, int, int){ keys[key] = false; }

void mouseClick(int button, int state, int, int){
    if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN && !gameOver) triggerShoot();
}

void timer(int){
    if(!gameOver){
        float rad=DEG2RAD(angle);
        float nx=camX, nz=camZ;
        if(keys['w']||keys['W']){ nx+=std::cosf(rad)*moveSpeed; nz+=std::sinf(rad)*moveSpeed; }
        if(keys['s']||keys['S']){ nx-=std::cosf(rad)*moveSpeed; nz-=std::sinf(rad)*moveSpeed; }
        if(keys['a']||keys['A']) angle-=turnSpeed;
        if(keys['d']||keys['D']) angle+=turnSpeed;
        if(canMove(nx,nz)){ camX=nx; camZ=nz; }
    }

    if(gunFiring){ gunFrame++; if(gunFrame>=GUN_FIRE_FRAMES) gunFiring=0; }
    if(gunCooldown>0) gunCooldown--;
    if(screenFlash>0){ screenFlash-=0.04f; if(screenFlash<0) screenFlash=0; }
    
    updateEnemies();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

int main(int argc, char**argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(960, 600);
    glutCreateWindow("Maze 3D");
    
    glEnable(GL_DEPTH_TEST); 
    glShadeModel(GL_SMOOTH); 
    glEnable(GL_NORMALIZE);
    initEnemies();
    loadAllTextures();

    glutDisplayFunc(display); 
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp); 
    glutMouseFunc(mouseClick);

    glutTimerFunc(100, timer, 0); 
    glutMainLoop();
    return 0;
}