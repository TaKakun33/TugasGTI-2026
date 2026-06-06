#include "config.h"
#include "enemy.h"
#include "weapon.h"
#include "renderer.h"
#include "lantern.h"

#include <cstdlib>
#include <ctime>



int MAP[MAP_H][MAP_W];

float camX  = 1.5f * CELL;
float camZ  = 1.5f * CELL;
float camY  = WALL_H * 0.45f;
float angle = 90.0f;
float moveSpeed = 0.16f;
float turnSpeed  = 3.0f;
int WIN_W = 960, WIN_H = 600;
int gunFiring = 0;
int gunFrame = 0;
int gunCooldown = 0;
int playerHP    = 5;
int playerMaxHP = 5;
float screenFlash = 0.0f;
int gameOver    = 0;
bool lanternOn = true;
bool keys[256] = {};
Enemy enemies[MAX_ENEMIES];

struct Room {
    int x, z, w, h;  
    int cx() const { return x + w/2; }
    int cz() const { return z + h/2; }
};

struct BSPNode {
    int x, z, w, h;    
    BSPNode *left, *right;
    Room room;
    bool hasRoom;

    BSPNode(int x, int z, int w, int h)
        : x(x), z(z), w(w), h(h),
          left(nullptr), right(nullptr), hasRoom(false) {}

    ~BSPNode() { delete left; delete right; }
};

static const int MIN_REGION = 5;  
static const int MIN_ROOM   = 3;  

static void splitNode(BSPNode *node, int depth) {
    if (depth <= 0) return;

    bool canSplitH = node->h >= MIN_REGION * 2;
    bool canSplitV = node->w >= MIN_REGION * 2;
    if (!canSplitH && !canSplitV) return;

    bool splitHoriz;
    if (canSplitH && canSplitV)
        splitHoriz = (rand() % 2 == 0);
    else
        splitHoriz = canSplitH;

    if (splitHoriz) {
        // Split along Z axis
        int splitZ = MIN_REGION + rand() % (node->h - MIN_REGION * 2 + 1);
        node->left  = new BSPNode(node->x, node->z,          node->w, splitZ);
        node->right = new BSPNode(node->x, node->z + splitZ, node->w, node->h - splitZ);
    } else {
        // Split along X axis
        int splitX = MIN_REGION + rand() % (node->w - MIN_REGION * 2 + 1);
        node->left  = new BSPNode(node->x,          node->z, splitX,          node->h);
        node->right = new BSPNode(node->x + splitX, node->z, node->w - splitX, node->h);
    }

    splitNode(node->left,  depth - 1);
    splitNode(node->right, depth - 1);
}

static void carveRoom(BSPNode *node) {
    if (node->left || node->right) {
        if (node->left)  carveRoom(node->left);
        if (node->right) carveRoom(node->right);
        return;
    }

    int maxW = node->w - 2;
    int maxH = node->h - 2;
    if (maxW < MIN_ROOM || maxH < MIN_ROOM) return;

    int rw = MIN_ROOM + rand() % (maxW - MIN_ROOM + 1);
    int rh = MIN_ROOM + rand() % (maxH - MIN_ROOM + 1);
    int rx = node->x + 1 + rand() % (node->w - rw - 1);
    int rz = node->z + 1 + rand() % (node->h - rh - 1);

    rx = std::max(1, std::min(rx, MAP_W - rw - 1));
    rz = std::max(1, std::min(rz, MAP_H - rh - 1));
    rw = std::min(rw, MAP_W - rx - 1);
    rh = std::min(rh, MAP_H - rz - 1);

    for (int z = rz; z < rz + rh; z++)
        for (int x = rx; x < rx + rw; x++)
            MAP[z][x] = 0;

    node->room = {rx, rz, rw, rh};
    node->hasRoom = true;
}

static Room getRoom(BSPNode *node) {
    if (node->hasRoom) return node->room;
    if (node->left && node->right) {
        Room l = getRoom(node->left);
        Room r = getRoom(node->right);
        return (rand() % 2 == 0) ? l : r;
    }
    if (node->left)  return getRoom(node->left);
    return getRoom(node->right);
}

static void carveCorridor(int x1, int z1, int x2, int z2) {
    if (rand() % 2 == 0) {
        int minX = std::min(x1, x2), maxX = std::max(x1, x2);
        for (int x = minX; x <= maxX; x++) {
            MAP[z1][x] = 0;
            if (z1 + 1 < MAP_H - 1) MAP[z1+1][x] = 0;
        }
        int minZ = std::min(z1, z2), maxZ = std::max(z1, z2);
        for (int z = minZ; z <= maxZ; z++) {
            MAP[z][x2] = 0;
            if (x2 + 1 < MAP_W - 1) MAP[z][x2+1] = 0;
        }
    } else {
        int minZ = std::min(z1, z2), maxZ = std::max(z1, z2);
        for (int z = minZ; z <= maxZ; z++) {
            MAP[z][x1] = 0;
            if (x1 + 1 < MAP_W - 1) MAP[z][x1+1] = 0;
        }
        int minX = std::min(x1, x2), maxX = std::max(x1, x2);
        for (int x = minX; x <= maxX; x++) {
            MAP[z2][x] = 0;
            if (z2 + 1 < MAP_H - 1) MAP[z2+1][x] = 0;
        }
    }
}

static void connectRooms(BSPNode *node) {
    if (!node->left || !node->right) return;

    connectRooms(node->left);
    connectRooms(node->right);

    Room l = getRoom(node->left);
    Room r = getRoom(node->right);
    carveCorridor(l.cx(), l.cz(), r.cx(), r.cz());
}

static void floodFill(int sx, int sz, bool visited[MAP_H][MAP_W]) {
    int qx[MAP_W * MAP_H], qz[MAP_W * MAP_H];
    int head = 0, tail = 0;
    qx[tail] = sx; qz[tail] = sz; tail++;
    visited[sz][sx] = true;
    const int dx[] = {1,-1,0,0};
    const int dz[] = {0,0,1,-1};
    while (head < tail) {
        int cx = qx[head], cz = qz[head]; head++;
        for (int d = 0; d < 4; d++) {
            int nx = cx + dx[d], nz = cz + dz[d];
            if (nx < 0 || nx >= MAP_W || nz < 0 || nz >= MAP_H) continue;
            if (MAP[nz][nx] != 0) continue;
            if (visited[nz][nx]) continue;
            visited[nz][nx] = true;
            qx[tail] = nx; qz[tail] = nz; tail++;
        }
    }
}

static void ensureConnectivity() {
    MAP[1][1] = 0; MAP[1][2] = 0;
    MAP[2][1] = 0; MAP[2][2] = 0;

    for (int pass = 0; pass < MAP_W * MAP_H; pass++) {
        bool visited[MAP_H][MAP_W] = {};
        floodFill(1, 1, visited);

        int ux = -1, uz = -1;
        for (int z = 1; z < MAP_H - 1 && ux < 0; z++)
            for (int x = 1; x < MAP_W - 1 && ux < 0; x++)
                if (MAP[z][x] == 0 && !visited[z][x])
                    { ux = x; uz = z; }

        if (ux < 0) break;

        int bestX = 1, bestZ = 1, bestDist = 999999;
        for (int z = 1; z < MAP_H - 1; z++)
            for (int x = 1; x < MAP_W - 1; x++) {
                if (!visited[z][x]) continue;
                int dist = (x - ux)*(x - ux) + (z - uz)*(z - uz);
                if (dist < bestDist) { bestDist = dist; bestX = x; bestZ = z; }
            }

        int x1 = std::min(ux, bestX), x2 = std::max(ux, bestX);
        for (int x = x1; x <= x2; x++) MAP[uz][x] = 0;
        int z1 = std::min(uz, bestZ), z2 = std::max(uz, bestZ);
        for (int z = z1; z <= z2; z++) MAP[z][bestX] = 0;
    }
}

static void generateMap() {
    for (int z = 0; z < MAP_H; z++)
        for (int x = 0; x < MAP_W; x++)
            MAP[z][x] = 1;

    BSPNode *root = new BSPNode(1, 1, MAP_W - 2, MAP_H - 2);
    splitNode(root, 4);   
    carveRoom(root);
    connectRooms(root);
    delete root;

    for (int x = 0; x < MAP_W; x++) MAP[0][x] = MAP[MAP_H-1][x] = 1;
    for (int z = 0; z < MAP_H; z++) MAP[z][0] = MAP[z][MAP_W-1] = 1;

    ensureConnectivity();

    for (int x = 0; x < MAP_W; x++) MAP[0][x] = MAP[MAP_H-1][x] = 1;
    for (int z = 0; z < MAP_H; z++) MAP[z][0] = MAP[z][MAP_W-1] = 1;
}

static void placeEnemies() {
    const float minDistFromStart = 5.0f * CELL;
    int placed = 0;
    int attempts = 0;

    while (placed < MAX_ENEMIES && attempts < 20000) {
        attempts++;
        int ex = 1 + rand() % (MAP_W - 2);
        int ez = 1 + rand() % (MAP_H - 2);
        if (MAP[ez][ex] != 0) continue;

        float wx = (ex + 0.5f) * CELL;
        float wz = (ez + 0.5f) * CELL;
        float dx = wx - camX, dz = wz - camZ;
        float dist = std::sqrtf(dx*dx + dz*dz);
        if (dist < minDistFromStart) continue;

        enemies[placed].x = wx;
        enemies[placed].z = wz;
        enemies[placed].hp = ENEMY_HP_MAX;
        enemies[placed].state = ES_IDLE;
        enemies[placed].attackTimer = 0;
        enemies[placed].deathTimer = 0.0f;
        enemies[placed].flashTimer = 0.0f;
        placed++;
    }

    for (int i = placed; i < MAX_ENEMIES; i++) {
        enemies[i].x = -999.0f;
        enemies[i].z = -999.0f;
        enemies[i].state = ES_DEAD;
        enemies[i].deathTimer = 999.0f;
    }
}

void resetGame() {
    camX = 1.5f * CELL;
    camZ = 1.5f * CELL;
    angle = 90;
    playerHP = playerMaxHP;
    screenFlash = 0;
    gameOver = 0;
    gunFiring = 0; gunFrame = 0; gunCooldown = 0;
    for (int i = 0; i < 256; i++) keys[i] = false;
    generateMap();
    placeEnemies();
}

void display() {
    if (lanternOn) {
        glClearColor(0.05f, 0.04f, 0.03f, 1.0f); 
    } else {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);   \
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, WIN_W, WIN_H);

setPerspectiveView(WIN_W, WIN_H);

glEnable(GL_LIGHTING);
glEnable(GL_NORMALIZE);
if (!lanternOn) {
    glDisable(GL_LIGHT0);
    GLfloat black[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);
} else {
    GLfloat dark[] = {0.02f, 0.02f, 0.02f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, dark);
}
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FOG);
    
    GLfloat fogC[4];
    if (lanternOn) {
        fogC[0] = 0.0f; fogC[1] = 0.0f; fogC[2] = 0.0f; fogC[3] = 1.0f;
        glFogf(GL_FOG_START, 3.0f);
        glFogf(GL_FOG_END, 12.0f); 
    } else {
        fogC[0] = 0.0f; fogC[1] = 0.0f; fogC[2] = 0.0f; fogC[3] = 1.0f;
        glFogf(GL_FOG_START, 0.5f);
        glFogf(GL_FOG_END, 2.0f);  
    }
    
    glFogfv(GL_FOG_COLOR, fogC);
    glFogi(GL_FOG_MODE, GL_LINEAR);

    drawFloorCeiling();
    drawMaze3D();
    drawAllEnemies();

	glDisable(GL_FOG);
    drawLantern();
    drawGunOverlay(WIN_W, WIN_H);
    drawHUD(WIN_W, WIN_H);
    glutSwapBuffers();
}

void reshape(int w, int h) {
    WIN_W = w; WIN_H = (h > 0 ? h : 1);
    glViewport(0, 0, WIN_W, WIN_H);
}

void keyboard(unsigned char key, int, int) {
    keys[key] = true;
    if (key == 27 || key == 'q' || key == 'Q') exit(0);
    if (key == 'r' || key == 'R') { resetGame(); return; }
    if (key == ' ' && !gameOver) triggerShoot();
}

void keyboardUp(unsigned char key, int, int) { keys[key] = false; }

void mouseClick(int button, int state, int, int) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && !gameOver)
        triggerShoot();
}

void timer(int) {
    if (!gameOver) {
        float rad = DEG2RAD(angle);
        float nx = camX, nz = camZ;
        if (keys['w'] || keys['W']) { nx += std::cosf(rad)*moveSpeed; nz += std::sinf(rad)*moveSpeed; }
        if (keys['s'] || keys['S']) { nx -= std::cosf(rad)*moveSpeed; nz -= std::sinf(rad)*moveSpeed; }
        if (keys['a'] || keys['A']) angle -= turnSpeed;
        if (keys['d'] || keys['D']) angle += turnSpeed;
        if (canMove(nx, nz)) { camX = nx; camZ = nz; }
    }

    if (gunFiring) { gunFrame++; if (gunFrame >= GUN_FIRE_FRAMES) gunFiring = 0; }
    if (gunCooldown > 0) gunCooldown--;
    if (screenFlash > 0) { screenFlash -= 0.04f; if (screenFlash < 0) screenFlash = 0; }

    updateEnemies();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

int main(int argc, char **argv) {
    srand((unsigned int)time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(960, 600);
    glutCreateWindow("Imagine Getting Noclipped");

    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    generateMap();
    placeEnemies();
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