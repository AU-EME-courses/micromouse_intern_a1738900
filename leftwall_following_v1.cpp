#include <stdio.h>                      // input & output 
#include <stdbool.h>                    // true & false values 
enum Direction {N,E,S,W};               // assigning directions to numbers 0,1,2 and 3
enum Direction D = N;                   // variable D sets current direction of the mouse as north
int x = 0;                              // x coordinate set to start of maze 
int y = 0;                              // y coordinate set to start of maze
#define maze 16                         // maze size for mms simulator                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               

// to define whether there's a wall in each direction
bool wallFront = false;     
bool wallLeft  = false;
bool wallRight = false;
bool wallBack  = false;

// defines a 2d maze array, boolean tracks which maze cells have been visited by the mouse, e.g. travelled[2][2] = true, means mouse4 has visited cell
bool travelled[maze][maze] = {false};

// defines a 3d array, boolean checks which direction of the cell, walls are present, e.g. walls[2][2][2] = true, means wall south to that cell
bool walls[maze][maze][4] = {false};     

// function for turning right
void turnRight() {
    D = (D+1) % 4;                      // modulus to keep value between 0 & 3
}

// function for turning left
void turnLeft() {
    D = (D+3) % 4;                      // cant use D=(D-1)%4 as result can be -, i.e. invalid direction
}

// function for turning around 180 degrees
void turnAround() {
    D = (D+2) % 4;    
}

// function to move forward in a direction
void moveForward() {
    switch (D) {
        case N: y++; break;             // if the current direction is north, move 1 step on y axis 
        case E: x++; break;             // if the current direction is east, move 1 step on x axis
        case S: y--; break;             // if the current direction is south, move 1 step on -y axis
        case W: x--; break;             // if the current direction is west, move 1 step on -x axis
    }
}

// function to record walls in the present cell 
void recordWalls() {
    if (x < 0 || x >= maze || y < 0 || y >= maze)   // to exclude the coordinates outside the maze
        return;

    travelled[y][x] = true;             // marks that current cell has been travelled to       

    walls[y][x][D] = wallFront;             // copies boolean value from sensor to the wall data to store if there is a wall to the north of the current cell
    walls[y][x][(D + 3) % 4] = wallLeft;    // copies boolean value from sensor to the wall data to store if there is a wall to the west of the current cell 
    walls[y][x][(D + 1) % 4] = wallRight;   // copies boolean value from sensor to the wall data to store if there is a wall to the east of the current cell
    walls[y][x][(D + 2) % 4] = wallBack;    // copies boolean value from sensor to the wall data to store if there is a wall to the south of the current cell
}

void move1step() {
// check left, front and right for wall in this order
    if (wallLeft==false) {              // if there is no wall on the left, turn left and move 1 step in the left  
        turnLeft();
        moveForward();
    } else if (wallFront==false) {      // if there is a wall on the left, move 1 step to the front
        moveForward();
    } else if (wallRight==false) {      // if there is a wall on the front as well, turn right and move 1 step in the right
        turnRight();
        moveForward();
    } else {                            // if there is a wall on the right as well, turn around 180 and move 1 step
        turnAround();
        moveForward(); 
    }
    recordWalls();                      // records wall data for the new cell after the mouse has moved 1 step
}
