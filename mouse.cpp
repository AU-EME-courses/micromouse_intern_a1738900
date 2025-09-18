#include "API.h"                        // includes the MMS simulator's API header to access its functions
#include <iostream>                    // for std::cout
#include <stdbool.h>                   // true & false values
#include <queue>                       // for using queue data structure

enum Direction { N = 0, E = 1, S = 2, W = 3 };               // assigning directions to numbers 0 to 3

enum Direction D = N;                  // variable D sets current direction of the mouse as north
int x = 0;                             // x coordinate set to start of maze
int y = 0;                             // y coordinate set to start of maze
int steps = 0;                         // variable to count number of steps taken
#define maze 16                        // maze size for mms simulator
int distance[maze][maze];              // Stores shortest distance from each cell to goal                                                                                                                                  

// defines a 2d maze array, boolean tracks which maze cells have been visited by the mouse, e.g. travelled[2][2] = true, means mouse4 has visited cell
int travelled[maze][maze] = {0};

// defines a 3d array, boolean checks which direction of the cell, walls are present, e.g. walls[2][2][2] = true, means wall south to that cell
bool walls[maze][maze][4] = {false};  

bool wallFront;                  // front wall present or not
bool wallLeft;                   // left wall present or not
bool wallRight;                  // right wall present or not
bool wallBack;                   // back wall present or not

// function to move forward in a direction
void moveForwardStep() {
    switch (D) {
        case N: y++; break;             // if the current direction is north, move 1 step on y axis
        case E: x++; break;             // if the current direction is east, move 1 step on x axis
        case S: y--; break;             // if the current direction is south, move 1 step on -y axis
        case W: x--; break;             // if the current direction is west, move 1 step on -x axis
    }
    API::moveForward();                   // MMS CHANGE: call MMS API to move forward
}

// function to record walls in the present cell
void recordWalls() {
    if (x < 0 || x >= maze || y < 0 || y >= maze)   // to exclude the coordinates outside the maze
        return;

    travelled[y][x]++;             // marks that current cell has been travelled to      
    // MMS CHANGE: Use API calls to check walls instead of variables
    walls[y][x][D] = API::wallFront();             // copies boolean value from sensor to the wall data to store if there is a wall to the north of the current cell
    walls[y][x][(D + 3) % 4] = API::wallLeft();    // copies boolean value from sensor to the wall data to store if there is a wall to the west of the current cell
    walls[y][x][(D + 1) % 4] = API::wallRight();   // copies boolean value from sensor to the wall data to store if there is a wall to the east of the current cell
    walls[y][x][(D + 2) % 4] = false;               // No API for wallBack, so left as a false value, as the mouse will never move backwards
}

void printmaze() {
    // print Top wall
    std::cerr << " ";  // space before top wall                                          
    for (int x = 0; x < maze; x++) {    // for each cell in the top row       
        if (walls[maze - 1][x][N]) std::cerr << "_";    // if wall north of cell, print _
        else std::cerr << " ";  // else print space
        std::cerr << " ";   // space between cells
    }
    std::cerr << std::endl; // new line after printing top wall
 
    for (int y = maze - 1; y >= 0; y--) {   // for each row in the maze, starting from the top
        for (int x = 0; x < maze; x++) {    // for each cell in the row
            // print Left wall
            if (x == 0) {   // always print left wall for the first cell in row, left boundry of maze
                if (walls[y][x][W]) std::cerr << "|";   // if wall west of cell, print |
                else std::cerr << " ";  // else print space
            }
 
            // print Bottom wall
            if (x == ::x && y == ::y) {   // ensures mouse's current position prints by comparing loop variables to global x and y
                std::cerr << "@";   // if the mouse is in this cell, print *
            } else if (travelled[y][x]) {
                std::cerr << travelled[y][x];   // if the cell has been travelled to, print *
            } else if (walls[y][x][S]) {
                std::cerr << "_";   // if wall south of cell, print _
            } else {
                std::cerr << " ";   // else print space
            }

            // print Right wall
            if (walls[y][x][E]) std::cerr << "|";   // if wall east of cell, print |
            else std::cerr << " ";  // else print space
        }
        std::cerr << std::endl; // new line after printing each row
    }
}

// function to move 1 step according to left-hand rule, record step info, and print every 10 steps
void move1step() {
    // check left, front and right for wall in this order
    if (API::wallLeft() == false) {              // if there is no wall on the left, turn left and move 1 step in the left  
        API::turnLeft();
        D = static_cast<Direction>((D + 3) % 4);  // turn left
        moveForwardStep();
    } else if (API::wallFront() == false) {      // if there is a wall on the left, move 1 step to the front
        moveForwardStep();
    } else if (API::wallRight() == false) {      // if there is a wall on the front as well, turn right and move 1 step in the right
        API::turnRight();
        D = static_cast<Direction>((D + 1) % 4);  // turn right
        moveForwardStep();
    } else {                            // if there is a wall on the right as well, turn around 180 and move 1 step
        API::turnRight();
        API::turnRight();
        D = static_cast<Direction>((D + 2) % 4);  // turn around
        moveForwardStep();
    }
    recordWalls();                      // records wall data for the new cell after the mouse has moved 1 step
}

bool run = true;        // variable to control the main loop

int main() {
    while (run) {                // main loop, runs until reset
        move1step();
        steps++;                // increment step count
        if (steps % 10 == 0) {  // print maze every 10 steps
            printmaze();  
        }
        if (API::wasReset()) {  // check if reset button has been pressed
            API::ackReset();    // acknowledge the reset to the API
            run = false;        // stop the loop to handle reset
        }
    }
    return 0;   
}