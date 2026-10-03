// BIL 421 Assignment 1 (January 30, 2020)
// Immediate-mode OpenGL road-crossing game.

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <string>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

class Vehicle {
public:
    float lane;         // Vertical lane center
    float position;     // Horizontal position
    int direction;      // 0 moves toward +x, 1 moves toward -x
    float type;         // Width: car 0.026, truck 0.052
};

class Agent {
public:
    int roadPos;        // Step index, 0 at the bottom through 24 at the top
    float road;         // Vertical position
    float position;     // Horizontal position
    int direction;      // Up (0) or down (1)
};

class Coin {
public:
    float lane;         // Vertical lane center
    float position;     // Horizontal position
    float time;         // Time left before it disappears
    int isExist;        // 0 absent, 1 present
};

const int MOVE_LEFT = 0;
const int MOVE_RIGHT = 1;
const int MOVE_DOWN = 2;
const int MOVE_UP = 3;

GLint TIMER_DELAY = 10;
GLfloat RED_RGB[] = {1, 0, 0};
GLfloat BLUE_RGB[] = {0, 0, 1};
GLfloat WHITE_RGB[] = {1, 1, 1};
GLfloat BLACK_RGB[] = {0, 0, 0};
GLfloat YELLOW_RGB[] = {0.8, 0.8, 0};
GLfloat ORANGE_RGB[] = {1, 0.5, 0};
GLfloat GRAY_RGB[] = {0.5, 0.5, 0.5};

float lanes[18] = {};                   // Lane positions
float roads[25] = {};                   // Road positions
Vehicle vehicles[100] = {};             // Vehicle array
int vehicleControl[100] = {};           // 1 while that slot holds a vehicle
int numberOfVehicle = 0;                // Number of vehicles
int point = 0;                          // Game point
int isStopped = 0;                      // 1 if the game stops, 0 otherwise
int isFinised = 0;                      // 1 after the game has ended
int moveStack = -1;                     // Key pressed while the game is paused
Agent agent;                            // Agent
Coin coin;                              // Coin
int powerMode = 0;                      // 0 off, 1 on
int powerCounter = 0;                   // Extra points collected during a power move
int crashedVehicle = -1;                // Index of the vehicle that hit the agent
float vehicleSpeed = 0.003;             // Normal mode vehicle speed
int vehicleTime = 100;                  // Spawn when a 1..1000 roll is below this
int gameMode = 2;                       // 1 easy, 2 normal, 3 hard

void reshapeFunct(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 1.0, 0.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glutPostRedisplay();
}

// Sets lane centers, step positions, and the starting agent.
void fillArrays() {
    int j = 0;
    for (int i = 10; i < 95; i = i + 16) {
        float number = i / 100.0;
        lanes[j++] = number - 0.04;
        lanes[j++] = number;
        lanes[j++] = number + 0.04;
    }

    j = 0;
    for (int i = 2; i < 99; i += 4) {
        float number = i / 100.0;
        roads[j++] = number;
    }

    for (int i = 0; i < 100; i++) {
        vehicleControl[i] = 0;
    }

    agent.roadPos = 0;
    agent.road = roads[0];
    agent.position = 0.5;
    agent.direction = 0;

    coin.isExist = 0;
}

// Pauses the game and marks it finished.
void finish() {
    isStopped = 1;
    isFinised = 1;
}

void drawString(float x, float y, const std::string& text) {
    glRasterPos3f(x, y, 1);
    glColor3fv(YELLOW_RGB);
    for (std::string::size_type i = 0; i < text.size(); i++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);
    }
}

void drawFinishText() {
    drawString(0.38, 0.49, "Press q to quit.");
}

void drawPoint() {
    std::stringstream text;
    text << "Puan: " << point;
    drawString(0, 0.005, text.str());
}

void drawRemainingTime() {
    std::stringstream text;
    text << "Time: " << coin.time / 100;
    drawString(0, 0.967, text.str());
}

void drawGameMode() {
    std::string mode = "";
    if (gameMode == 1)
        mode = "Easy";
    else if (gameMode == 2)
        mode = "Normal";
    else if (gameMode == 3)
        mode = "Hard";

    drawString(0.85, 0.967, mode);
}

void drawNumberOfVehicle() {
    std::stringstream text;
    text << "Vehicle: " << numberOfVehicle;
    drawString(0.775, 0.005, text.str());
}

// White road bands. They turn gray after the game ends.
void drawRoads() {
    for (int i = 4; i < 85; i = i + 16) {
        double value = i / 100.0;

        if (isFinised == 1)
            glColor3fv(GRAY_RGB);
        else
            glColor3fv(WHITE_RGB);
        glRectf(0, value, 1, value + 0.12);
    }
}

// Dashed lines between lanes.
void drawLines() {
    for (int i = 8; i < 95; i = i + 16) {
        double number = i / 100.0;
        for (int j = 0; j < 10; j = j + 1) {
            double value = j / 10.0 + 0.01;
            glColor3fv(BLACK_RGB);
            glRectf(value, number - 0.002, value + 0.08, number + 0.002);

            glColor3fv(BLACK_RGB);
            glRectf(value, number + 0.04 - 0.002, value + 0.08, number + 0.04 + 0.002);
        }
    }
}

void drawAgent() {
    glBegin(GL_TRIANGLES);
    glColor3fv(RED_RGB);
    if (agent.direction == 0) {
        glVertex2f(agent.position, agent.road + 0.013);
        glVertex2f(agent.position - 0.010, agent.road - 0.013);
        glVertex2f(agent.position + 0.010, agent.road - 0.013);
    } else {
        glVertex2f(agent.position, agent.road - 0.013);
        glVertex2f(agent.position - 0.010, agent.road + 0.013);
        glVertex2f(agent.position + 0.010, agent.road + 0.013);
    }
    glEnd();
}

// Draws vehicles without moving them. Used while the game is paused.
void drawVehicles() {
    for (int i = 0; i < 100; i++) {
        if (vehicleControl[i] == 1) {
            if (i == crashedVehicle)
                glColor3fv(ORANGE_RGB);
            else
                glColor3fv(BLUE_RGB);

            glRectf(vehicles[i].position, vehicles[i].lane - 0.013,
                    vehicles[i].position + vehicles[i].type, vehicles[i].lane + 0.013);
        }
    }
}

void drawCoin() {
    if (coin.isExist == 1) {
        drawRemainingTime();

        float x1 = coin.position;
        float y1 = coin.lane;
        double radius = 0.02;
        glColor3fv(YELLOW_RGB);

        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x1, y1);
        for (float angle = 1.0f; angle < 361.0f; angle += 0.2) {
            float x2 = x1 + std::sin(angle) * radius;
            float y2 = y1 + std::cos(angle) * radius;
            glVertex2f(x2, y2);
        }
        glEnd();

        if (isStopped == 0)
            coin.time -= 1;

        if (coin.time < 0)
            coin.isExist = 0;
    }
}

// Draws vehicles, moves them, and checks for a hit.
void moveVehicles() {
    for (int i = 0; i < 100; i++) {
        if (vehicleControl[i] == 1) {
            glColor3fv(BLUE_RGB);
            glRectf(vehicles[i].position, vehicles[i].lane - 0.013,
                    vehicles[i].position + vehicles[i].type, vehicles[i].lane + 0.013);

            if (vehicles[i].direction == 0)
                vehicles[i].position += vehicleSpeed;
            else
                vehicles[i].position -= vehicleSpeed;

            if (vehicles[i].position < 0 || vehicles[i].position > 1) {
                Vehicle v{};
                vehicles[i] = v;
                vehicleControl[i] = 0;
                numberOfVehicle -= 1;
            }

            if (agent.road > vehicles[i].lane - 0.013 && agent.road < vehicles[i].lane + 0.013) {
                if (agent.position > vehicles[i].position &&
                    agent.position < vehicles[i].position + vehicles[i].type) {
                    finish();
                    crashedVehicle = i;
                }
            }
        }
    }
}

// move: 0 left, 1 right, 2 down, 3 up. Any other value only checks the coin.
void moveAgent(int move) {
    if (move == MOVE_UP) {
        if (agent.roadPos < 24) {
            agent.roadPos += 1;
            agent.road = roads[agent.roadPos];
            if (agent.direction == 0) {
                point += 1;
                if (gameMode == 3)
                    point += 1;
            } else if (agent.direction == 1) {
                finish();
            }
            if (agent.roadPos == 24)
                agent.direction = 1;
        }
    } else if (move == MOVE_DOWN) {
        if (agent.roadPos > 0) {
            agent.roadPos -= 1;
            agent.road = roads[agent.roadPos];
            if (agent.direction == 1) {
                point += 1;
                if (gameMode == 3)
                    point += 1;
            } else if (agent.direction == 0) {
                finish();
            }
            if (agent.roadPos == 0)
                agent.direction = 0;
        }
    } else if (move == MOVE_LEFT) {
        if (agent.position > 0.025)
            agent.position -= 0.025;
    } else if (move == MOVE_RIGHT) {
        if (agent.position < 0.950)
            agent.position += 0.025;
    }

    if (agent.road > coin.lane - 0.013 && agent.road < coin.lane + 0.013) {
        if (agent.position > coin.position - 0.05 && agent.position < coin.position + 0.05) {
            point += 5;
            if (gameMode == 3)
                point += 5;
            coin.isExist = 0;
        }
    }
}

void createVehicle() {
    int randomLane = std::rand() % 18;
    int randomVehicle = std::rand() % 2;

    Vehicle v;
    v.lane = lanes[randomLane];
    if (randomVehicle == 0)
        v.type = 0.026;
    else
        v.type = 0.052;

    // Even lanes enter from the right and travel left. Odd lanes do the opposite.
    if (randomLane % 2 == 0) {
        v.direction = 1;
        v.position = 1;
    } else {
        v.direction = 0;
        v.position = 0;
    }

    int index = -1;
    for (int i = 0; i < 100; i++) {
        if (vehicleControl[i] == 0) {
            index = i;
            break;
        }
    }

    if (index != -1) {
        vehicles[index] = v;
        vehicleControl[index] = 1;
        numberOfVehicle += 1;
    }
}

void createCoin() {
    int randomLane = std::rand() % 18;
    int randomPosition = std::rand() % 39 + 1;
    int randomTime = std::rand() % 400 + 300;

    randomPosition *= 25;
    float rPos = randomPosition / 1000.0;

    coin.isExist = 1;
    coin.time = randomTime;
    coin.lane = lanes[randomLane];
    coin.position = rPos;
}

void displayFunct(void) {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    drawRoads();
    drawLines();
    drawAgent();

    if (gameMode == 1) {
        vehicleSpeed = 0.002;
        vehicleTime = 60;
    } else if (gameMode == 2) {
        vehicleSpeed = 0.003;
        vehicleTime = 100;
    } else if (gameMode == 3) {
        vehicleSpeed = 0.006;
        vehicleTime = 200;
    }

    if (isStopped == 0) {
        int random = std::rand() % 1000 + 1;
        if (random < vehicleTime)
            createVehicle();
        moveVehicles();
    } else {
        drawVehicles();
    }

    glColor3fv(YELLOW_RGB);
    drawPoint();
    drawNumberOfVehicle();
    drawGameMode();

    if (isStopped == 0 && coin.isExist == 0) {
        int random = std::rand() % 1000 + 1;
        if (random < 5)
            createCoin();
    }
    drawCoin();

    if (powerMode == 1 && isFinised == 0) {
        if (agent.direction == 0)
            moveAgent(MOVE_UP);
        else
            moveAgent(MOVE_DOWN);
        powerCounter += 2;

        if (agent.roadPos == 0 || agent.roadPos == 24) {
            powerMode = 0;
            point += powerCounter;
            powerCounter = 0;
        }
    }

    if (isFinised == 1)
        drawFinishText();

    glutSwapBuffers();
}

void timerFunct(int id) {
    glutPostRedisplay();
    glutTimerFunc(TIMER_DELAY, timerFunct, 0);
}

// Left click pauses and resumes. Right click advances a single step.
void mouseFunct(int b, int s, int x, int y) {
    if (isFinised == 0) {
        if (s == GLUT_DOWN) {
            if (b == GLUT_LEFT_BUTTON)
                isStopped = !isStopped;

            if (b == GLUT_RIGHT_BUTTON) {
                if (isStopped == 0)
                    isStopped = 1;

                if (isStopped == 1) {
                    moveVehicles();
                    moveAgent(moveStack);
                    moveStack = -1;
                    if (coin.isExist == 1)
                        coin.time -= 1;
                    int random = std::rand() % 1000 + 1;
                    if (random < vehicleTime)
                        createVehicle();
                }
            }
        }
    }
}

void keyboardFunct(unsigned char c, int x, int y) {
    switch (c) {
    case 'q':
    case 'Q':
        std::exit(0);
        break;
    case '1':
        gameMode = 1;
        break;
    case '2':
        gameMode = 2;
        break;
    case '3':
        gameMode = 3;
        break;
    case 13:
        if (isStopped == 0)
            powerMode = 1;
        break;
    default:
        break;
    }
}

void catchKeyFunct(int key, int x, int y) {
    int move = -1;
    if (key == GLUT_KEY_LEFT)
        move = MOVE_LEFT;
    else if (key == GLUT_KEY_RIGHT)
        move = MOVE_RIGHT;
    else if (key == GLUT_KEY_DOWN)
        move = MOVE_DOWN;
    else if (key == GLUT_KEY_UP)
        move = MOVE_UP;

    if (move == -1)
        return;

    if (isStopped == 0)
        moveAgent(move);
    else
        moveStack = move;
}

int main(int argc, char** argv) {
    std::srand(std::time(0));
    fillArrays();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(500, 600);
    glutInitWindowPosition(0, 0);
    glutCreateWindow(argv[0]);

    glutDisplayFunc(displayFunct);
    glutReshapeFunc(reshapeFunct);
    glutMouseFunc(mouseFunct);
    glutKeyboardFunc(keyboardFunct);
    glutSpecialFunc(catchKeyFunct);
    glutTimerFunc(TIMER_DELAY, timerFunct, 0);
    glutMainLoop();
    return 0;
}
