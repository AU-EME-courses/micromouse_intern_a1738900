#include "API.h"                        // includes the MMS simulator's API header to access its functions
#include <iostream>                    // for std::cout
#include <stdbool.h>                   // true & false values
#include <queue>                       // for using queue data structure
 
enum Direction { N = 0, E = 1, S = 2, W = 3 };               // assigning directions to numbers 0 to 3
 
enum Direction D = N;                  // variable D sets current direction of the mouse as north
int x = 0;                             // x coordinate set to start of maze
int y = 0;                             // y coordinate set to start of maze
int steps = 0;                         // variable to count number of steps taken
int stepsrun1 = 0;                     // variable to count number of steps taken in first run
int stepsrun2 = 0;                     // variable to count number of steps taken in second run
#define maze 16                        // maze size for mms simulator
int distance[maze][maze];              // 2d array to store distance
 
// defines a 2d maze array, boolean tracks which maze cells have been visited by the mouse, e.g. travelled[2][2] = true, means mouse has visited cell
int travelled[maze][maze] = {0};
 
// defines a 3d array, boolean checks which direction of the cell, walls are present, e.g. walls[2][2][2] = true, means wall south to that cell
bool walls[maze][maze][4] = {false};

bool run = true;        // variable to control the main
bool centre = false;   // variable to track if the mouse is heading to the center or returning to start
bool run2 = false; // flag to indicate second run (returning to start)
 
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
    
    API::setColor(x, y, 'G');  // Marks cell green (visited)    
    // MMS CHANGE: Use API calls to check walls instead of variables
    walls[y][x][D] = API::wallFront();             // copies boolean value from sensor to the wall data to store if there is a wall to the north of the current cell
    walls[y][x][(D + 3) % 4] = API::wallLeft();    // copies boolean value from sensor to the wall data to store if there is a wall to the west of the current cell
    walls[y][x][(D + 1) % 4] = API::wallRight();   // copies boolean value from sensor to the wall data to store if there is a wall to the east of the current cell
    walls[y][x][(D + 2) % 4] = false;               // No API for wallBack, so left as a false value, as the mouse will never move backwards

    // Highlight detected walls in red
    for (int dir = 0; dir < 4; dir++) { 
        if (walls[y][x][dir]) {     // if there is a wall in this direction
            API::setWall(x, y, "nesw"[dir]);   // Visually draw the wall
        }
    }
 
    // update the wall information for the adjacent cells
    for (int dir = 0; dir < 4; dir++) { // temporary direction variable 'dir'
        int ny = y, nx = x; // store current coordinates into new variables to calculate neighbor cell coordinates
        switch (dir) {  // update neighbor coordinates based on direction
            case N: ny = y + 1; break;  // neighbor cell north
            case S: ny = y - 1; break;  // neighbor cell south
            case E: nx = x + 1; break;  // neighbor cell east
            case W: nx = x - 1; break;  // neighbor cell west
        }
        if (ny >= 0 && ny < maze && nx >= 0 && nx < maze) { // if neighbor cell is within maze bounds
            walls[ny][nx][(dir + 2) % 4] = walls[y][x][dir];    // update the wall information for the neighbor cell to be consistent with current cell
        }
    }
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
 
void floodfill(int GoalX, int GoalY){ // function prototype for floodfill with goal coordinates as parameters{
    std::queue<std::pair<int, int>> q;  // declares a queue where each element is in y, x format is a pair to hold cells to be processed
 
    // Initialize distances before starting floodfill
    for (int i = 0; i < maze; i++) {
        for (int j = 0; j < maze; j++) {
            distance[i][j] = 9999;  // initialize all distances to a large number
        }
    }
 
    q.push({GoalY, GoalX});   // initiates queue by storing goal cell coordinates to the back of the queue
    distance[GoalY][GoalX] = 0;   // variable to store distance from goal to each cell, thus goal to goal distance is set to 0
 
    while (!q.empty()) {    // while the queue is not empty, i.e. there are still cells to process
        auto [fy, fx] = q.front();    // get the front cell entry from the queue
        q.pop();   // remove the front cell entry from the queue
 
        // local variable dir to explore all 4 directions from the current cell
        for (int dir = 0; dir < 4; dir++) {
            int ny = fy, nx = fx; // store current coordinates into new variables to calculate neighbor cell coordinates
 
            switch (dir) {  // update neighbor coordinates based on direction
                case N: ny++; break;    // update neighbor coordinates by 1 step on y axis if direction is north
                case S: ny--; break;    // update neighbor coordinates by 1 step on -y axis if direction is south
                case E: nx++; break;    // update neighbor coordinates by 1 step on x axis  if direction is east
                case W: nx--; break;    // update neighbor coordinates by 1 step on -x axis if direction is west
            }
 
            // if neighbor cell is within maze bounds and there is no wall between current cell and neighbor cell
            if (ny >= 0 && ny < maze && nx >= 0 && nx < maze && !walls[fy][fx][dir]) {
                // If the neighbor cell doesn't yet have a better path ((distance[y][x]) + 1 as going to neighbor adds 1 step)
                if (distance[ny][nx] > distance[fy][fx] + 1) {
                    distance[ny][nx] = distance[fy][fx] + 1;  // update the neighbor cell distance to the shortest path found
                    q.push({ny, nx});   // add neighbor cell coordinates to the back of the queue
                }
            }
        }
    }
}
 
// function to move one step towards the goal using floodfill logic
void move1step() {
    travelled[y][x]++;  // counter for number of times the cell has been visited
    if (!run2) {  // if this is the first run
        if (travelled[y][x] == 1) { // if the current cell has not been travelled to yet, record walls
            recordWalls();
        }
    }
    int bestD = -1;        // declared to sentinel value '-1' as no best direction has been found yet
    int bestdistance = 9999;     // initialize best distance as a large value
 
    for (int dir = 0; dir < 4; dir++) {  // temporary direction variable 'dir' to explore possible neighbor cells
        int nx = x, ny = y;  
        switch (dir) {  // update neighbor coordinates based on direction
            case N: ny++; break;        
            case S: ny--; break;
            case E: nx++; break;
            case W: nx--; break;
        }
 
        // check if the neighbor cell is within bounds and not blocked by a wall
        if (ny >= 0 && ny < maze && nx >= 0 && nx < maze && !walls[y][x][dir]) {
            // if the neighbor has a lower distance value (i.e. closer to goal), update best direction
            if (distance[ny][nx] < bestdistance) {                                
                bestdistance = distance[ny][nx];    // update best distance to the new lowest distance found                                                                    
                bestD = dir;    // update best direction to the current direction being checked 'dir'
            }
        }
    }
 
    // if no valid direction is found, return
    if (bestD == -1) {
        return;
    }
 
    // turn towards best direction from current direction D
    int turns = (bestD - D + 4) % 4;    // calculate number of right turns needed to face best direction
    if (turns == 3) {  // more efficient to turn left once than right 3 times
        API::turnLeft();            // MMS CHANGE: call MMS API to turn left                                                
        D = static_cast<Direction>((D + 3) % 4);        // update current direction D to new direction after turning left                
    } else {            
        for (int i = 0; i < turns; i++) {       // loop to turn right the required number of times
            API::turnRight();      // MMS CHANGE: call MMS API to turn right      
            D = static_cast<Direction>((D + 1) % 4);    // update current direction D to new direction after turning right            
        }
    }
    moveForwardStep();  // move forward one step in the new direction
}
 
int main() {                    
    for (int i = 0; i < maze; i++) {                            
        for (int j = 0; j < maze; j++) {                            
            distance[i][j] = 9999;  // initialize all distances from cells to goal as a large number
        }
    }
    while (run) {          
        if (!centre) {                    
        floodfill(8, 8);  // go to center first
        } else {            
        floodfill(0, 0);  // return to start
        }
        move1step();        // move one step towards the goal using floodfill logic            
        steps++;            // increment step counter
        if (!run2 && !centre) {     // if this is the first run                                       
            stepsrun1++;    // increment first run step counter                                                                    
        } else if (run2 && !centre) {   // if this is the second run              
            stepsrun2++;    // increment second run step counter                                   
        }
        if (!centre && x == 8 && y == 8) {  // if the mouse has reached the center
            centre = true;    // goal reached, now return to start
        }
        if (centre && x == 0 && y == 0) {   // if the mouse has returned to the start
            if (!run2) {          // if this is the first run
                centre = false;  // reset to head to center again
                run2 = true;      // Second run begins
            } else {
                run = false;      // Second run finished, stop program
            }
        }
        if (steps % 10 == 0) {  // print maze every 10 steps
            printmaze();
            std::cerr << "1st run: " << stepsrun1 << ", 2nd run: " << stepsrun2 << std::endl;
        }
        if (API::wasReset()) {      // if the reset button has been pressed on the simulator              
            API::ackReset();       // acknowledge the reset to the simulator
            run = false;      // stop the main loop when reset is pressed
        }
    }
    return 0;
}