#include "config.h"
#include "enemy.h"
#include "weapon.h"
#include "renderer.h"
#include <cstdlib>
#include <ctime>

int MAP[MAP_H][MAP_W];

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

// -------------------------------------------------------
// BSP Room Generator
// Each BSP node holds a region of the map.
// We recursively split regions, carve a room inside each
// leaf, then connect sibling rooms with an L-shaped corridor.
// -------------------------------------------------------

struct Room {
    int x, z, w, h;   // top-left corner + size (in map cells)
    int cx() const { return x + w/2; }
    int cz() const { return z + h/2; }
};

struct BSPNode {
    int x, z, w, h;        // region this node covers
    BSPNode *left, *right;
    Room room;
    bool hasRoom;

    BSPNode(int x, int z, int w, int h)
        : x(x), z(z), w(w), h(h),
          left(nullptr), right(nullptr), hasRoom(false) {}

    ~BSPNode() { delete left; delete right; }
};

static const int MIN_REGION = 5;   // minimum region size before we stop splitting
static const int MIN_ROOM   = 3;   // minimum room size (in cells)

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
        // Internal node — recurse
        if (node->left)  carveRoom(node->left);
        if (node->right) carveRoom(node->right);
        return;
    }

    // Leaf node — carve a room with random size inside the region
    // Leave at least 1 cell border so rooms don't touch each other
    int maxW = node->w - 2;
    int maxH = node->h - 2;
    if (maxW < MIN_ROOM || maxH < MIN_ROOM) return;

    int rw = MIN_ROOM + rand() % (maxW - MIN_ROOM + 1);
    int rh = MIN_ROOM + rand() % (maxH - MIN_ROOM + 1);
    int rx = node->x + 1 + rand() % (node->w - rw - 1);
    int rz = node->z + 1 + rand() % (node->h - rh - 1);

    // Clamp to map bounds
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

// Get the "center room" of a subtree (used for corridor connection)
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

// Carve an L-shaped corridor between two points
static void carveCorridor(int x1, int z1, int x2, int z2) {
    // Horizontal then vertical (or vice versa randomly)
    if (rand() % 2 == 0) {
        // Go horizontal first
        int minX = std::min(x1, x2), maxX = std::max(x1, x2);
        for (int x = minX; x <= maxX; x++) {
            MAP[z1][x] = 0;
            // Make corridors 2 wide so they feel less cramped
            if (z1 + 1 < MAP_H - 1) MAP[z1+1][x] = 0;
        }
        int minZ = std::min(z1, z2), maxZ = std::max(z1, z2);
        for (int z = minZ; z <= maxZ; z++) {
            MAP[z][x2] = 0;
            if (x2 + 1 < MAP_W - 1) MAP[z][x2+1] = 0;
        }
    } else {
        // Go vertical first
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

static void generateMap() {
    // Fill everything with walls
    for (int z = 0; z < MAP_H; z++)
        for (int x = 0; x < MAP_W; x++)
            MAP[z][x] = 1;

    // Build BSP tree over the interior (leave border walls)
    BSPNode *root = new BSPNode(1, 1, MAP_W - 2, MAP_H - 2);
    splitNode(root, 4);   // depth 4 = up to 16 rooms on a 17x17 map
    carveRoom(root);
    connectRooms(root);
    delete root;

    // Always enforce solid border
    for (int x = 0; x < MAP_W; x++) MAP[0][x] = MAP[MAP_H-1][x] = 1;
    for (int z = 0; z < MAP_H; z++) MAP[z][0] = MAP[z][MAP_W-1] = 1;

    // Guarantee player start is open
    MAP[1][1] = 0;
    MAP[1][2] = 0;
    MAP[2][1] = 0;
    MAP[2][2] = 0;
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
    if (showMap) drawMinimap(WIN_W, WIN_H);
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
    if (key == 'm' || key == 'M') showMap = !showMap;
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
    glutCreateWindow("Maze 3D");

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