#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>
#include <ctime>
#include <cmath>
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib") // sound library


const int rows = 6, cols = 6;
int windowWidth = 800, windowHeight = 600;

// Game state
int timeRemaining = 30;
bool lost = false, won = false;
int score = 0;

// Animation timers
float winTextAnimTime = 0.0f;
float pelletRotation = 0.0f;
float goalFloatTime = 0.0f;



// Player position and movement
int cubeX = 5, cubeY = 0;
bool moving = false;
int dirX = 0, dirY = 0;

// Pellets
bool pellets[rows][cols];

// Game map
int map[6][6] = {
    {1, 0, 0, 0, 0, 1},
    {0, 0, 1, 0, 1, 0},
    {0, 1, 0, 0, 0, 0},
    {0, 0, 0, 1, 0, 0},
    {1, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 2, 1}
};

// Lighting positions
float lightPos0[] = { 3.0f, -5.0f, 10.0f, 1.0f };
float lightPos1[] = { 3.0f, 10.0f, 5.0f, 1.0f };

// Material properties for walls, floor, player and goal
float wallAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
float wallDiffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
float wallSpecular[] = { 0.3f, 0.3f, 0.3f, 1.0f };
float wallShininess = 30.0f;

float floorAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
float floorDiffuse[] = { 0.7f, 0.7f, 0.7f, 1.0f };
float floorSpecular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
float floorShininess = 20.0f;

float playerAmbient[] = { 0.0f, 0.0f, 1.0f, 1.0f };
float playerDiffuse[] = { 0.0f, 0.0f, 1.0f, 1.0f };
float playerSpecular[] = { 0.0f, 0.0f, 1.0f, 1.0f };
float playerShininess = 60.0f;

float goalAmbient[] = { 0.2f, 0.0f, 0.0f, 1.0f };
float goalDiffuse[] = { 0.9f, 0.1f, 0.1f, 1.0f };
float goalSpecular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
float goalShininess = 30.0f;

// Drawn Manually (gems)
void drawOctahedron(float x, float y, float z, float size) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(size, size, size);

    float ambient[]  = {0.0f, 0.0f, 1.0f, 1.0f};
    float diffuse[]  = {0.0f, 0.0f, 1.0f, 1.0f};
    float specular[] = {0.0f, 0.0f, 1.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, 10.0f);

    float top[] = {0.0f, 0.0f, 1.0f};
    float bottom[] = {0.0f, 0.0f, -1.0f};
    float base[4][3] = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},
        {0.0f, -1.0f, 0.0f}
    };

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 4; i++) {
        int next = (i + 1) % 4;
        // Top faces
        glNormal3f(0, 0, 1);
        glVertex3fv(top);
        glVertex3fv(base[i]);
        glVertex3fv(base[next]);
        // Bottom faces
        glNormal3f(0, 0, -1);
        glVertex3fv(bottom);
        glVertex3fv(base[next]);
        glVertex3fv(base[i]);
    }
    glEnd();

    glPopMatrix();
}


// Drawn Manually (used on top of walls)
void drawPyramid(float x, float y, float z, float baseSize, float height) {
    glPushMatrix();
    glTranslatef(x, y, z);

    glBegin(GL_TRIANGLES);
    // Front
    glNormal3f(0, 0.5, 0.5);
    glVertex3f(0, 0, height);
    glVertex3f(-baseSize / 2, -baseSize / 2, 0);
    glVertex3f(baseSize / 2, -baseSize / 2, 0);

    // Right
    glNormal3f(0.5, 0.5, 0);
    glVertex3f(0, 0, height);
    glVertex3f(baseSize / 2, -baseSize / 2, 0);
    glVertex3f(baseSize / 2, baseSize / 2, 0);

    // Back
    glNormal3f(0, 0.5, -0.5);
    glVertex3f(0, 0, height);
    glVertex3f(baseSize / 2, baseSize / 2, 0);
    glVertex3f(-baseSize / 2, baseSize / 2, 0);

    // Left
    glNormal3f(-0.5, 0.5, 0);
    glVertex3f(0, 0, height);
    glVertex3f(-baseSize / 2, baseSize / 2, 0);
    glVertex3f(-baseSize / 2, -baseSize / 2, 0);
    glEnd();

    // Base
    glBegin(GL_QUADS);
    glNormal3f(0, 0, -1);
    glVertex3f(-baseSize / 2, -baseSize / 2, 0);
    glVertex3f(baseSize / 2, -baseSize / 2, 0);
    glVertex3f(baseSize / 2, baseSize / 2, 0);
    glVertex3f(-baseSize / 2, baseSize / 2, 0);
    glEnd();

    glPopMatrix();
}

// Draw wall cubes
void drawCube(float x, float y, float z, float r, float g, float b, bool isPlayer = false) {
    glPushMatrix();
    glTranslatef(x, y, z);

    if (isPlayer) {
        glMaterialfv(GL_FRONT, GL_AMBIENT, playerAmbient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, playerDiffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, playerSpecular);
        glMaterialf(GL_FRONT, GL_SHININESS, playerShininess);
        glScalef(0.9f, 0.9f, 0.9f);
    } else {
        float matAmbient[] = { r * 0.3f, g * 0.3f, b * 0.3f, 1.0f };
        float matDiffuse[] = { r * 0.8f, g * 0.8f, b * 0.8f, 1.0f };
        float matSpecular[] = { 0.1f, 0.1f, 0.1f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
        glMaterialf(GL_FRONT, GL_SHININESS, 10.0f);
    }

    glutSolidCube(1.0);
    glPopMatrix();
}

// Draw floor tile or goal
void drawFloorTile(float x, float y, bool isGoal = false) {
    glPushMatrix();
    glTranslatef(x, y, -0.5f);

    if (isGoal) {
        float pulse = 0.5f + 0.5f * sin(goalFloatTime * 2.0f);
        float goalDiffuse[] = { 0.9f * pulse, 0.1f * pulse, 0.1f * pulse, 1.0f };
        glMaterialfv(GL_FRONT, GL_DIFFUSE, goalDiffuse);

    } else {
        glMaterialfv(GL_FRONT, GL_AMBIENT, floorAmbient);
        glMaterialfv(GL_FRONT, GL_SPECULAR, floorSpecular);
        glMaterialf(GL_FRONT, GL_SHININESS, floorShininess);

        float white[]     = { 1.0f, 1.0f, 1.0f, 1.0f };
        float lightGrey[] = { 0.5f, 0.5f, 0.5f, 1.0f };
        glMaterialfv(GL_FRONT, GL_DIFFUSE, ((int)(x + y)) % 2 == 0 ? white : lightGrey);
    }

    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glVertex3f(-0.5f, -0.5f, 0.0f);
    glVertex3f(0.5f, -0.5f, 0.0f);
    glVertex3f(0.5f, 0.5f, 0.0f);
    glVertex3f(-0.5f, 0.5f, 0.0f);
    glEnd();

    glPopMatrix();
}

// Drawn Manually (player)
void drawSphere(float x, float y, float z, float radius, int slices, int stacks, bool isPlayer = false) {
    glPushMatrix();
    glTranslatef(x, y, z);

    if (isPlayer) {
        glMaterialfv(GL_FRONT, GL_AMBIENT, playerAmbient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, playerDiffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, playerSpecular);
        glMaterialf(GL_FRONT, GL_SHININESS, playerShininess);
    } else {
        float ambient[]  = { 0.3f, 0.3f, 0.3f, 1.0f };
        float diffuse[]  = { 0.6f, 0.6f, 0.6f, 1.0f };
        float specular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
        glMaterialf(GL_FRONT, GL_SHININESS, 20.0f);
    }

    for (int i = 0; i < stacks; ++i) {
        float lat0 = M_PI * (-0.5 + (float)i / stacks);
        float lat1 = M_PI * (-0.5 + (float)(i + 1) / stacks);
        float z0 = radius * sin(lat0);
        float zr0 = radius * cos(lat0);
        float z1 = radius * sin(lat1);
        float zr1 = radius * cos(lat1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; ++j) {
            float lng = 2 * M_PI * (float)j / slices;
            float x1 = cos(lng);
            float y1 = sin(lng);

            glNormal3f(x1 * zr0, y1 * zr0, z0);
            glVertex3f(x1 * zr0, y1 * zr0, z0);
            glNormal3f(x1 * zr1, y1 * zr1, z1);
            glVertex3f(x1 * zr1, y1 * zr1, z1);
        }
        glEnd();
    }

    glPopMatrix();
}

void drawText(const char* text) {
    void* font = GLUT_BITMAP_TIMES_ROMAN_24;
    float bounce = sin(winTextAnimTime) * 10.0f;
    float pulse = 0.5f + 0.5f * sin(winTextAnimTime * 2);
    int len = (int)strlen(text);
    int textWidth = 0;
    for (int i = 0; i < len; i++)
        textWidth += glutBitmapWidth(font, text[i]);
    float x = (windowWidth - textWidth) / 2.0f;
    float y = windowHeight / 2.0f + bounce;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glColor3f(1.0f, pulse, 0.2f);

    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            glRasterPos2f(x + dx, y + dy);
            for (int i = 0; i < len; i++)
                glutBitmapCharacter(font, text[i]);
        }
    }

    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}


void drawTimer() {
    char buffer[50]; sprintf(buffer, "Time Left: %d s", timeRemaining);
    char scoreText[50]; sprintf(scoreText, "Score: %d", score);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(20, windowHeight - 30);
    for (int i = 0; buffer[i]; i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);

    glRasterPos2f(20, windowHeight - 60);
    for (int i = 0; scoreText[i]; i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, scoreText[i]);

    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// Update player's cube position
void updateCube() {
    if (!moving || won || lost) return;

    int nextX = cubeX + dirX;
    int nextY = cubeY + dirY;

    // Check boundaries and wall collisions
    if (nextX < 0 || nextX >= cols || nextY < 0 || nextY >= rows || map[nextY][nextX] == 1) {
        moving = false;
        return;
    }

    // Move player
    cubeX = nextX;
    cubeY = nextY;


   // Collect pellet if present
if (pellets[cubeY][cubeX]) {
    pellets[cubeY][cubeX] = false;
    score++;
    PlaySound("collect.wav", NULL, SND_FILENAME | SND_ASYNC);
}

// Check if reached goal
if (map[cubeY][cubeX] == 2) {
    won = true;
    moving = false;
    std::cout << "You win!\n";
    PlaySound("win.wav", NULL, SND_FILENAME | SND_ASYNC);
}
}

void drawScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(3, -8, 8, 3, 3, 0, 0, 0, 1);
    glScalef(1.5f, 1.5f, 1.0f);

    // Draw the map tiles, walls, pellets
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            // Draw floor or goal
            drawFloorTile(x, rows - 1 - y, map[y][x] == 2);

            if (map[y][x] == 1) { // Wall blocks
                glMaterialfv(GL_FRONT, GL_AMBIENT, wallAmbient);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, wallDiffuse);
                glMaterialfv(GL_FRONT, GL_SPECULAR, wallSpecular);
                glMaterialf(GL_FRONT, GL_SHININESS, wallShininess);

                float heightVariation = 0.1f * ((x + y) % 3);
                drawCube(x, rows - 1 - y, 0.0f + heightVariation, 0.5f, 0.5f, 0.5f);

                // pyramid on top of wall blocks
                glPushMatrix();
                glTranslatef(x, rows - 1 - y, 0.5f + heightVariation);
                drawPyramid(0.0f, 0.0f, 0.0f, 1.0f, 0.7f);
                glPopMatrix();
            }

            // Draw pellets
            if (pellets[y][x]) {
                glPushMatrix();
                glTranslatef(x, rows - 1 - y, 0.5f);
                glRotatef(pelletRotation, 0.0f, 1.0f, 0.0f);
                glScalef(0.2f, 0.2f, 0.2f);
                drawOctahedron(0.0f, 0.0f, 0.0f, 1.0f);
                glPopMatrix();
            }
        }
    }

    // Draw player (sphere)
    drawSphere(cubeX, rows - 1 - cubeY, 0.5f, 0.4f, 24, 24, true);

    // Draw win/lose messages
    if (lost)
        drawText("YOU LOST :( press ESC to exit or R to try again");

    if (won)
        drawText("YOU WIN! press ESC to exit or R to try again");

    // Draw timer and score
    drawTimer();

    glutSwapBuffers();
}

void timer(int) {
    if (!won && !lost) {
        updateCube();
        timeRemaining--;

        if (timeRemaining <= 0) {
            lost = true;
            std::cout << "You lose!\n";
            PlaySound("lose1.wav", NULL, SND_FILENAME);
            PlaySound("lose2.wav", NULL, SND_FILENAME | SND_ASYNC);
        }
    }

    winTextAnimTime += 0.4f;

    pelletRotation += 5.0f;
    if (pelletRotation >= 360.0f)
        pelletRotation -= 360.0f;

    goalFloatTime += 0.4f;

    glutPostRedisplay();
    glutTimerFunc(500, timer, 0);
}

void resetGame() {
    cubeX = 5; cubeY = 0;
    dirX = 0; dirY = 0;
    moving = false;
    won = false;
    lost = false;
    timeRemaining = 30;
    score = 0;

    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            pellets[y][x] = (map[y][x] == 0);
        }
    }
}

// Arrow keys for movement
void handleKey(int key, int, int) {
    if (moving || won || lost) return;

    switch (key) {
        case GLUT_KEY_UP:    dirX = 0; dirY = -1; break;
        case GLUT_KEY_DOWN:  dirX = 0; dirY = 1;  break;
        case GLUT_KEY_LEFT:  dirX = -1; dirY = 0; break;
        case GLUT_KEY_RIGHT: dirX = 1; dirY = 0;  break;
        default: return;
    }
    moving = true;
}

// Normal keys for restart or quit
void handleNormalKey(unsigned char key, int, int) {
    if (key == 27) { // ESC key
        exit(0);
    }
    if (key == 'r' || key == 'R') {
        resetGame();
    }
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glShadeModel(GL_SMOOTH);

    GLfloat ambientLight[] = {0.3f, 0.3f, 0.3f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
    float light0Diffuse[] = {0.6f, 0.6f, 0.6f, 1.0f};
    float light0Specular[] = {0.8f, 0.8f, 0.8f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0Specular);

    glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);
    float light1Diffuse[] = {0.5f, 0.5f, 0.5f, 1.0f};
    float light1Specular[] = {0.3f, 0.3f, 0.3f, 1.0f};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1Diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1Specular);

    GLfloat lightColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat lightPos[] = {0.0f, 0.0f, 10.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(45, (double)windowWidth / windowHeight, 1, 100);
    glMatrixMode(GL_MODELVIEW);

    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

    srand((unsigned int)time(NULL));
    resetGame();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutCreateWindow("3D Pac-Man Maze Game");
    init();
    glutDisplayFunc(drawScene);
    glutSpecialFunc(handleKey);
    glutKeyboardFunc(handleNormalKey);
    glutTimerFunc(100, timer, 0);
    glutMainLoop();
    return 0;
}
