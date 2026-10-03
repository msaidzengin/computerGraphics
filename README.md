# computerGraphics

This is BIL 421 Assignment 1, completed on January 30, 2020.

A 2D road-crossing game written with immediate-mode OpenGL and GLUT. The player moves a triangle across 18 lanes of cars and trucks and can collect coins. A collision, or a step against the agent's current direction, ends the game.

The original handout is `assignment1.pdf`.

## Build

Install a C++ compiler, OpenGL, GLU, and GLUT. On Debian or Ubuntu:

```bash
sudo apt install g++ freeglut3-dev libglu1-mesa-dev
```

```bash
g++ -std=c++11 game.cpp -o game -lGL -lGLU -lglut
```

## Run

```bash
./game
```

## Controls

- Arrow keys move the agent. A step with the current direction scores 1 point.
- Left click pauses or resumes.
- Right click pauses, then advances one step. Arrow keys pressed while paused are applied on that step.
- Enter starts a power move to the far sidewalk. Reaching it adds 2 extra points for each step of that move.
- 1, 2, and 3 select easy, normal, and hard. Hard doubles step and coin points and makes traffic faster. The game starts in normal mode.
- Q quits.

The score (`Puan`) is at the bottom left, the vehicle count at the bottom right, the mode at the top right, and the coin timer at the top left. A coin is worth 5 points. When the game ends, the roads turn gray and the vehicle that hit the agent turns orange.

## Screenshots

![Normal mode](ss/ss1.png)
![Hard mode with a coin](ss/ss2.png)
![Easy mode](ss/ss3.png)
