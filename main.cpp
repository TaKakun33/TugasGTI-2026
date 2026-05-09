// include glut sesuai platform
#ifdef _WIN32
#include <windows.h>
#include <GL/freeglut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>
#include <cstdio>
#include <cstring>

// ukuran peta dan dunia 3D
#define MAP_W 17
#define MAP_H 17
#define CELL 2.0f         // 1 sel = 2 unit dunia
#define WALL_H 2.4f       // tinggi dinding
#define PI 3.14159265f
#define DEG2RAD(x) ((x) * PI / 180.0f)

// peta 2D, 1 = dinding, 0 = lantai
static const int MAP[MAP_H][MAP_W] = {
    {1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1},
    {1,1,1,0,1,0,1,1,0,1,1,0,1,1,1,0,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
    {1,0,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1},
    {1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1},
    {1,1,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1},
    {1,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1},
    {1,0,1,1,1,0,1,1,1,0,1,0,1,0,0,0,1},
    {1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1},
    {1,0,0,0,1,0,0,0,1,0,1,1,1,0,1,0,1},
    {1,1,1,0,1,0,1,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
};

// posisi dan arah kamera pemain
static float camX  = 1.5f * CELL;
static float camZ  = 1.5f * CELL;
static float camY  = WALL_H * 0.45f; // ketinggian mata
static float angle = 90.0f;
static float moveSpeed = 0.08f;
static float turnSpeed  = 3.0f;

static int showMap = 1;
static int WIN_W   = 960, WIN_H = 600;

// cek apakah sel (mx,mz) adalah dinding
static inline int isWall(int mx, int mz) {
    if (mx < 0 || mx >= MAP_W || mz < 0 || mz >= MAP_H) return 1;
    return MAP[mz][mx];
}

// konversi koordinat dunia ke indeks sel
static inline int worldToCell(float w) {
    return (int)(w / CELL);
}

// gambar lantai dan langit-langit sebagai satu quad besar
static void drawFloorCeiling(void) {
    glBegin(GL_QUADS);
        glColor3f(0.76f, 0.72f, 0.58f); // kuning pucat
        glNormal3f(0, 1, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(MAP_W*CELL, 0, 0);
        glVertex3f(MAP_W*CELL, 0, MAP_H*CELL);
        glVertex3f(0, 0, MAP_H*CELL);
    glEnd();

    glBegin(GL_QUADS);
        glColor3f(0.92f, 0.90f, 0.82f); // putih hangat
        glNormal3f(0, -1, 0);
        glVertex3f(0, WALL_H, 0);
        glVertex3f(MAP_W*CELL, WALL_H, 0);
        glVertex3f(MAP_W*CELL, WALL_H, MAP_H*CELL);
        glVertex3f(0, WALL_H, MAP_H*CELL);
    glEnd();
}

// gambar satu kubus dinding di sel (mx, mz)
// hanya sisi yang menghadap ruang kosong yang digambar
static void drawWallCube(int mx, int mz) {
    float x0 = mx * CELL, x1 = x0 + CELL;
    float z0 = mz * CELL, z1 = z0 + CELL;
    float y0 = 0.0f, y1 = WALL_H;

    glColor3f(0.82f, 0.82f, 0.68f); // warna dinding krem

    if (!isWall(mx, mz - 1)) { // sisi utara
        glBegin(GL_QUADS);
            glNormal3f(0, 0, -1);
            glVertex3f(x0,y0,z0); glVertex3f(x1,y0,z0);
            glVertex3f(x1,y1,z0); glVertex3f(x0,y1,z0);
        glEnd();
    }
    if (!isWall(mx, mz + 1)) { // sisi selatan
        glBegin(GL_QUADS);
            glNormal3f(0, 0, 1);
            glVertex3f(x1,y0,z1); glVertex3f(x0,y0,z1);
            glVertex3f(x0,y1,z1); glVertex3f(x1,y1,z1);
        glEnd();
    }
    if (!isWall(mx - 1, mz)) { // sisi barat
        glBegin(GL_QUADS);
            glNormal3f(-1, 0, 0);
            glVertex3f(x0,y0,z1); glVertex3f(x0,y0,z0);
            glVertex3f(x0,y1,z0); glVertex3f(x0,y1,z1);
        glEnd();
    }
    if (!isWall(mx + 1, mz)) { // sisi timur
        glBegin(GL_QUADS);
            glNormal3f(1, 0, 0);
            glVertex3f(x1,y0,z0); glVertex3f(x1,y0,z1);
            glVertex3f(x1,y1,z1); glVertex3f(x1,y1,z0);
        glEnd();
    }

    glColor3f(0.75f, 0.75f, 0.62f); // atap sedikit lebih gelap
    glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glVertex3f(x0,y1,z0); glVertex3f(x1,y1,z0);
        glVertex3f(x1,y1,z1); glVertex3f(x0,y1,z1);
    glEnd();
}

// iterasi semua sel peta, gambar dinding yang ada
static void drawMaze3D(void) {
    for (int z = 0; z < MAP_H; z++)
        for (int x = 0; x < MAP_W; x++)
            if (MAP[z][x]) drawWallCube(x, z);
}

// setup dua lampu: point light di atas kepala + directional dari langit-langit
static void setupLighting(void) {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    GLfloat amb[] = {0.35f, 0.33f, 0.22f, 1.0f};
    GLfloat diff0[] = {0.80f, 0.76f, 0.55f, 1.0f};
    GLfloat spec0[] = {0.20f, 0.20f, 0.15f, 1.0f};
    GLfloat pos0[] = {camX, WALL_H * 0.9f, camZ, 1.0f}; // ikut posisi pemain

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  diff0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, spec0);
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);
    glLightf (GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.08f);

    GLfloat diff1[] = {0.25f, 0.25f, 0.20f, 1.0f};
    GLfloat pos1[]  = {0.0f, 1.0f, 0.0f, 0.0f}; // directional dari atas
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  diff1);
    glLightfv(GL_LIGHT1, GL_POSITION, pos1);

    GLfloat mat_spec[] = {0.1f, 0.1f, 0.1f, 1.0f};
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_spec);
    glMateriali (GL_FRONT, GL_SHININESS, 8);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
}

// set proyeksi perspektif dan posisi kamera first-person
static void setPerspectiveView(int w, int h) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70.0, (double)w / h, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float rad = DEG2RAD(angle);
    float lx  = camX + cosf(rad); // titik yang dilihat
    float lz  = camZ + sinf(rad);
    gluLookAt(camX, camY, camZ, lx, camY, lz, 0, 1, 0);
}

// gambar minimap 2D di sudut kanan atas
static void drawMinimap(int winW, int winH) {
    int mw = 160, mh = 160;
    int ox = winW - mw - 8, oy = 8;

    // viewport khusus minimap
    glViewport(ox, oy, mw, mh);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float worldW = MAP_W * CELL, worldH = MAP_H * CELL;
    glOrtho(0, worldW, worldH, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // background gelap
    glColor4f(0.1f, 0.1f, 0.08f, 0.85f);
    glBegin(GL_QUADS);
        glVertex2f(0,0); glVertex2f(worldW,0);
        glVertex2f(worldW,worldH); glVertex2f(0,worldH);
    glEnd();

    // gambar tiap sel peta
    for (int z = 0; z < MAP_H; z++) {
        for (int x = 0; x < MAP_W; x++) {
            float fx = x * CELL, fz = z * CELL;
            glColor3f(MAP[z][x] ? 0.55f : 0.22f, MAP[z][x] ? 0.55f : 0.22f, MAP[z][x] ? 0.42f : 0.18f);
            glBegin(GL_QUADS);
                glVertex2f(fx, fz);
                glVertex2f(fx+CELL, fz);
                glVertex2f(fx+CELL, fz+CELL);
                glVertex2f(fx, fz+CELL);
            glEnd();
        }
    }

    // segitiga kuning menunjukkan posisi dan arah pemain
    float pr = DEG2RAD(angle);
    float ax = camX + cosf(pr) * 0.45f, az = camZ + sinf(pr) * 0.45f;
    float bx = camX + cosf(pr + 2.4f) * 0.25f, bz = camZ + sinf(pr + 2.4f) * 0.25f;
    float cx = camX + cosf(pr - 2.4f) * 0.25f, cz = camZ + sinf(pr - 2.4f) * 0.25f;

    glColor3f(1.0f, 0.85f, 0.1f);
    glBegin(GL_TRIANGLES);
        glVertex2f(ax,az); glVertex2f(bx,bz); glVertex2f(cx,cz);
    glEnd();

    glEnable(GL_DEPTH_TEST);
}

// gambar teks hint dan crosshair di tengah layar
static void drawHUD(int winW, int winH) {
    glViewport(0, 0, winW, winH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, winW, 0, winH, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    const char *hint = "[W/S] Maju/Mundur  [A/D] Putar  [M] Minimap";
    glColor4f(0.9f, 0.9f, 0.7f, 1.0f);
    glRasterPos2i(8, 10);
    for (const char *c = hint; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);

    // crosshair
    int cx = winW / 2, cy = winH / 2;
    glColor3f(1, 1, 0.6f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
        glVertex2i(cx-8, cy); glVertex2i(cx+8, cy);
        glVertex2i(cx, cy-8); glVertex2i(cx, cy+8);
    glEnd();

    glEnable(GL_DEPTH_TEST);
}

// callback render utama
static void display(void) {
    glClearColor(0.62f, 0.60f, 0.48f, 1.0f); // warna fog
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, WIN_W, WIN_H);
    setupLighting();
    setPerspectiveView(WIN_W, WIN_H);

    // fog linear biar lorong terasa panjang
    glEnable(GL_FOG);
    GLfloat fogColor[] = {0.62f, 0.60f, 0.48f, 1.0f};
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 4.0f);
    glFogf(GL_FOG_END,  22.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    drawFloorCeiling();
    drawMaze3D();

    if (showMap) drawMinimap(WIN_W, WIN_H);
    glViewport(0, 0, WIN_W, WIN_H);
    drawHUD(WIN_W, WIN_H);

    glutSwapBuffers();
}

static void reshape(int w, int h) {
    WIN_W = w;
    WIN_H = (h > 0 ? h : 1);
    glViewport(0, 0, WIN_W, WIN_H);
}

// cek 4 titik sudut AABB pemain agar tidak nembus dinding
static int canMove(float nx, float nz) {
    float margin = 0.25f;
    int cx0 = worldToCell(nx - margin), cz0 = worldToCell(nz - margin);
    int cx1 = worldToCell(nx + margin), cz1 = worldToCell(nz + margin);
    return !isWall(cx0,cz0) && !isWall(cx1,cz0) &&
           !isWall(cx0,cz1) && !isWall(cx1,cz1);
}

static void keyboard(unsigned char key, int /*x*/, int /*y*/) {
    float rad = DEG2RAD(angle);
    float nx  = camX, nz = camZ;

    switch (key) {
        case 'w': case 'W':
            nx += cosf(rad) * moveSpeed;
            nz += sinf(rad) * moveSpeed;
            break;
        case 's': case 'S':
            nx -= cosf(rad) * moveSpeed;
            nz -= sinf(rad) * moveSpeed;
            break;
        case 'a': case 'A': angle -= turnSpeed; break;
        case 'd': case 'D': angle += turnSpeed; break;
        case 'm': case 'M': showMap = !showMap;  break;
    }

    if (canMove(nx, nz)) { camX = nx; camZ = nz; }
    glutPostRedisplay();
}

// timer ~60fps biar gerakan tidak tergantung event keyboard
static void timer(int /*v*/) {
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("Maze 3D");

    // posisi awal di sel entry (1,1)
    camX  = 1.5f * CELL;
    camZ  = 1.5f * CELL;
    angle = 90.0f;

    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}