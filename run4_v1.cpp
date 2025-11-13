#include "API.h"                        // includes the MMS simulator's API header to access its functions
#include <iostream>                    // for std::cout
#include <stdbool.h>                   // true & false values
#include <queue>                       // for using queue data structure
#include <vector>                      // for using vector data structure
#include <cmath>

enum Direction { N = 0, E = 1, S = 2, W = 3 };               // assigning directions to numbers 0 to 3
 
enum Direction currentDirection = N;                  // variable currentDirection sets current direction of the mouse as north
int x = 0;                             // x coordinate set to start of maze
int y = 0;                             // y coordinate set to start of maze
int goalX = 7;                        // goal x coordinate set to center of maze
int goalY = 7;                        // goal y coordinate set to center of maze
int steps = 0;                         // variable to count number of steps taken
int stepsrun1 = 0;                     // variable to count number of steps taken in first run
int stepsrun2 = 0;                     // variable to count number of steps taken in second run
int stepsrun3 = 0;                     // variable to count number of steps taken in third run
int stepsrun4 = 0;
#define maze 16                        // maze size for mms simulator
int distance[maze][maze];              // add direction dimension to store distance values for each direction in each cell
 
// defines a 2d maze array, integer value tracks how many times cells have been visited by the mouse, e.g. visitCount[2][2] = 1, means mouse has visited cell once
int visitCount[maze][maze] = {0};
 
// defines a 3d array, boolean checks which direction of the cell, walls are present, e.g. walls[2][2][2] = true, means wall south to that cell
bool walls[maze][maze][4] = {false};

bool running = true;        // variable to control the main
bool headingtoCentre = false;   // variable to track if the mouse is heading to the center or returning to start
bool run2 = false; // flag to indicate second run (returning to start)
bool run3 = false; // flag to indicate third run
bool run4 = false;
 
bool wallFront;                  // front wall present or not
bool wallLeft;                   // left wall present or not
bool wallRight;                  // right wall present or not
bool wallBack;                   // back wall present or not

std::vector<std::pair<int, int>> shortestpath; // vector to store the shortest path coordinates
 
// function to move forward in a direction
void moveForwardStep() {
    switch (currentDirection) {
        case N: y++; break;             // if the current direction is north, move 1 step on y axis
        case E: x++; break;             // if the current direction is east, move 1 step on x axis
        case S: y--; break;             // if the current direction is south, move 1 step on -y axis
        case W: x--; break;             // if the current direction is west, move 1 step on -x axis
    }
    API::moveForward();                   // call MMS API to move forward
}

// function to simulate movement to a diagonally located cell using two cardinal steps
void moveDiagonal(int targetX, int targetY) {
    int dx = targetX - x;       // difference in x coordinates
    int dy = targetY - y;       // difference in y coordinates
    if (dx > 0) {                                                               // if the difference in x coordinates is positive, move east
        while (currentDirection != E) {
            API::turnRight();
            currentDirection = static_cast<Direction>((currentDirection + 1) % 4);
        }
        API::moveForward(); x++;
    } else if (dx < 0) {                        // if the difference in x coordinates is negative, move west                                                                                 
        while (currentDirection != W) {
            API::turnRight();
            currentDirection = static_cast<Direction>((currentDirection + 1) % 4);
        }
        API::moveForward(); x--;        
    }
    if (dy > 0) {                                           // if the difference in y coordinates is positive, move north
        while (currentDirection != N) {
            API::turnRight();
            currentDirection = static_cast<Direction>((currentDirection + 1) % 4);
        }
        API::moveForward(); y++;
    } else if (dy < 0) {                                            // if the difference in y coordinates is negative, move south
        while (currentDirection != S) {                                     
            API::turnRight();                               
            currentDirection = static_cast<Direction>((currentDirection + 1) % 4);                                  
        }
        API::moveForward(); y--;
    }       
}
 
// function to record walls in the present cell
void recordWalls() {
    if (x < 0 || x >= maze || y < 0 || y >= maze)   // to exclude the coordinates outside the maze
        return;
        
    // Use API calls to check walls instead of variables
    walls[y][x][currentDirection] = API::wallFront();             // copies boolean value from sensor to the wall data to store if there is a wall to the north of the current cell
    walls[y][x][(currentDirection + 3) % 4] = API::wallLeft();    // copies boolean value from sensor to the wall data to store if there is a wall to the west of the current cell
    walls[y][x][(currentDirection + 1) % 4] = API::wallRight();   // copies boolean value from sensor to the wall data to store if there is a wall to the east of the current cell
    walls[y][x][(currentDirection + 2) % 4] = false;               // No API for wallBack, so left as a false value, as the mouse will never move backwards
    if (x == 0 && y == 0) walls[0][0][S] = true; // starting cell has south wall

    // Highlight detected walls in red
    for (int direction = 0; direction < 4; direction++) { 
        if (walls[y][x][direction]) {     // if there is a wall in this direction
            API::setWall(x, y, "nesw"[direction]);   // Visually draw the wall
        }
    }
 
    // update the wall information for the adjacent cells
    for (int direction = 0; direction < 4; direction++) { // temporary direction variable 'direction'
        int nextY = y, nextX = x; // store current coordinates into new variables to calculate neighbor cell coordinates
        switch (direction) {  // update neighbor coordinates based on direction
            case N: nextY = y + 1; break;  // neighbor cell north
            case S: nextY = y - 1; break;  // neighbor cell south
            case E: nextX = x + 1; break;  // neighbor cell east
            case W: nextX = x - 1; break;  // neighbor cell west
        }
        if (nextY >= 0 && nextY < maze && nextX >= 0 && nextX < maze) { // if neighbor cell is within maze bounds
            walls[nextY][nextX][(direction + 2) % 4] = walls[y][x][direction];    // update the wall information for the neighbor cell to be consistent with current cell
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
            } else if (visitCount[y][x]) {
                std::cerr << visitCount[y][x];   // if the cell has been visitCount to, print *
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
 
void floodfill(int goalX, int goalY){ // function prototype for floodfill with goal coordinates as parameters
    std::queue<std::pair<int, int>> q;  // declares a queue where each element is in y, x format is a pair to hold cells to be processed
 
    // Initialize distances before starting floodfill
    for (int i = 0; i < maze; i++) {
        for (int j = 0; j < maze; j++) {
            distance[i][j] = 9999;  // initialize all distances to a large number
        }
    }
 
    q.push({goalY, goalX});   // initiates queue by storing goal cell coordinates to the back of the queue
    distance[goalY][goalX] = 0;   // variable to store distance from goal to each cell, thus goal to goal distance is set to 0
 
    while (!q.empty()) {    // while the queue is not empty, i.e. there are still cells to process
        auto [cellY, cellX] = q.front();    // get the front cell entry from the queue
        q.pop();   // remove the front cell entry from the queue

        // local variable direction to explore all 4 directions from the current cell
        for (int direction = 0; direction < 4; direction++) {
            int nextY = cellY, nextX = cellX; // store current coordinates into new variables to calculate neighbor cell coordinates

            switch (direction) {  // update neighbor coordinates based on direction
                case N: nextY++; break;    // update neighbor coordinates by 1 step on y axis if direction is north
                case S: nextY--; break;    // update neighbor coordinates by 1 step on -y axis if direction is south
                case E: nextX++; break;    // update neighbor coordinates by 1 step on x axis  if direction is east
                case W: nextX--; break;    // update neighbor coordinates by 1 step on -x axis if direction is west
            }
 
            // if neighbor cell is within maze bounds and there is no wall between current cell and neighbor cell
            if (nextY >= 0 && nextY < maze && nextX >= 0 && nextX < maze && !walls[cellY][cellX][direction]) {
                // If the neighbor cell doesn't yet have a better path ((distance[y][x]) + 1 as going to neighbor adds 1 step)
                if (distance[nextY][nextX] > distance[cellY][cellX] + 1) {
                    distance[nextY][nextX] = distance[cellY][cellX] + 1;  // update the neighbor cell distance to the shortest path found
                    q.push({nextY, nextX});   // add neighbor cell coordinates to the back of the queue
                }
            }
        }
    }
}
    
// function to move one step towards the goal using floodfill logic
void move1step() {

    if (!run3 && !run4) {
        if (visitCount[y][x]==0) {   // if this cell has not been visited before
            recordWalls();
        }
    }
    API::setColor(x, y, !run2 ? 'G' : (!run3 ? 'B' : (!run4 ? 'Y' : 'C'))); // color current cell based on run number
    visitCount[y][x]++;  // counter for number of times the cell has been visited
    if (run3 && headingtoCentre) { // only record path during third run to center
        shortestpath.push_back({x, y});
    }
    int bestdirection = -1;        // declared to sentinel value '-1' as no best direction has been found yet
    int bestdistance = 9999;     // initialize best distance as a large value

    for (int direction = 0; direction < 4; direction++) {  // temporary direction variable 'direction' to explore possible neighbor cells
        int nextX = x, nextY = y;
        switch (direction) {  // update neighbor coordinates based on direction
            case N: nextY++; break;
            case S: nextY--; break;
            case E: nextX++; break;
            case W: nextX--; break;
        }
 
        // check if the neighbor cell is within bounds and not blocked by a wall
        if (nextY >= 0 && nextY < maze && nextX >= 0 && nextX < maze && !walls[y][x][direction]) {
            // if the neighbor has a lower distance value (i.e. closer to goal), update best direction
            if (distance[nextY][nextX] < bestdistance || (distance[nextY][nextX] == bestdistance && direction == currentDirection)) {
                bestdistance = distance[nextY][nextX];
                bestdirection = direction;
            }
        }
    }
 
    // if no valid direction is found, return
    if (bestdirection == -1) {
    if (headingtoCentre) {
        // Try a fallback, maybe turn right or left even if not bestdistance, to push movement
        API::turnRight();
        currentDirection = static_cast<Direction>((currentDirection + 1) % 4);
        // maybe move forward if there's no wall
        if (!walls[y][x][currentDirection]) {
            moveForwardStep();
        }
    }
    return;
}
 
    // turn towards best direction from current direction 
    int turns = (bestdirection - currentDirection + 4) % 4;    // calculate number of right turns needed to face best direction
    if (turns == 3) {  // more efficient to turn left once than right 3 times
        API::turnLeft();            // call MMS API to turn left                                                
        currentDirection = static_cast<Direction>((currentDirection + 3) % 4);        // update current direction to new direction after turning left                
    } else {            
        for (int i = 0; i < turns; i++) {       // loop to turn right the required number of times
            API::turnRight();      // call MMS API to turn right
            currentDirection = static_cast<Direction>((currentDirection + 1) % 4);    // update current direction to new direction after turning right
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
    std::vector<std::pair<int, int>> run4path; // vector to store the path for the fourth run                                                                      
    while (running) {
        if (run4 && headingtoCentre && !shortestpath.empty()) {  // during fourth run to center, follow the recorded shortest path from third run
            if (run4path.empty()) { // if run4path is empty
                 run4path = shortestpath;   // copy shortest path to run4path if not already done
              }                                                                                                             
            else {    // if there are still cells left in the run4path to follow                                                
                auto [targetX, targetY] = run4path.front(); // get the next target cell from the front of the run4path                               
                run4path.erase(run4path.begin());   // remove the front cell from run4path after retrieving it                   
                moveDiagonal(targetX, targetY); // move diagonally to the target cell                                
                steps++; stepsrun4++;         // increment step counters               
                if (x >= 7 && x <= 8 && y >= 7 && y <= 8) {     // if one of the four center cells reached, head to start            
                    running = false;    // end the run after reaching center in fourth run
                }
                continue;   // skip the rest of the loop to continue following the path
            }
        }
        if (!headingtoCentre) {                    
            floodfill(goalX, goalY);  // go to center first
        } else {            
            floodfill(0, 0);  // return to start
        }
        move1step();        // move one step towards the goal using floodfill logic                  
        steps++;            // increment step counter
        if (!run2 && !headingtoCentre) {
            stepsrun1++;
        } else if (run2 && !run3 && !headingtoCentre) {
            stepsrun2++;
        } else if (run3 && !run4 && !headingtoCentre) {
            stepsrun3++;
        } else if (run4 && !headingtoCentre) {
            stepsrun4++;
        }
        if (!headingtoCentre && x >= 7 && x <= 8 && y >= 7 && y <= 8) {
            headingtoCentre = true;   // if one of the four center cells reached, head to start
        }    
        if (headingtoCentre && x == 0 && y == 0) {
            if (!run2) {
                headingtoCentre = false;
                run2 = true;   // Start second run
            } else if (!run3) {
                headingtoCentre = false;
                run3 = true;   // Start third run
            } else if (!run4) {
                headingtoCentre = false;
                run4 = true;
            } else {
                running = false;   // All runs complete
            }
        }
        if (steps % 10 == 0) {  // print maze every 10 steps
            printmaze();
            std::cerr << "1st: " << stepsrun1 << ", 2nd: " << stepsrun2 << ", 3rd: " << stepsrun3 << ", 4th: " << stepsrun4 << std::endl;
        }
        if (API::wasReset()) {      // if the reset button has been pressed on the simulator              
            API::ackReset();       // acknowledge the reset to the simulator
            running = false;      // stop the main loop when reset is pressed
        }
    }
    for (const auto& cell : shortestpath) {
        API::setColor(cell.first, cell.second, 'R');  // x is cell.first, y is cell.second
    }
    return 0;

}
