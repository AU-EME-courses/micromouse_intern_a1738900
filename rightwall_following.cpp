#include <iostream>
#include <string>
using namespace std;
enum Direction {N,E,S,W};               // assigning directions to numbers 0,1,2 and 3
enum Direction D = N;                   // variable D sets current direction of the mouse as north
int x = 0;                              // x coordinate set to start of maze 
int y = 0;                              // y coordinate set to start of maze
#define maze 16                         // maze size for mms simulator

// declares a fucntion of string type that takes a constant, cmd (command) of string type as its input parameter 
string sendCommand(const string& cmd) {     
    cout << cmd << end1;    // sends command string to mms simulator via standard output
    string res;             // res is a string variable to store the mms simulator's response
    cin >> res;             // reads the simulator's response from standard input 
    return res;             // returns the recieved response
}

// send wall commands to the simulator, read the response, and convert that response into true/false
bool getWallFront() {return sendCommand("wallFront")=="true";}  // Returns true if there is a wall directly in front of the mouse, otherwise false
bool getWallLeft() {return sendCommand("wallLeft")=="true";}    // Returns true if there is a wall directly on the left of the mouse, otherwise false
bool getWallRight() {return sendCommand("wallRight")=="true";}  // Returns true if there is a wall directly on the right of the mouse, otherwise false
bool getWallBack() {return sendCommand("wallBack")=="true";}    // Returns true if there is a wall directly at the back of the mouse, otherwise false

// defines a 2d maze array, boolean tracks which maze cells have been visited by the mouse, e.g. travelled[2][2] = true, means mouse4 has visited cell
bool travelled[maze][maze] = {false};

// defines a 3d array, boolean checks which direction of the cell, walls are present, e.g. walls[2][2][2] = true, means wall south to that cell
bool walls[maze][maze][4] = {false};     

// function for turning right
void turnRight() {
    D = Direction((D+1)%4);     // modulus to keep value between 0 & 3
    sendCommand("turnRight");   // to send command to the mouse to turn 90 degrees to the right in the mms simulator
}

// function for turning left
void turnLeft() {
    D = Direction((D+3)%4);     // cant use D=(D-1)%4 as result can be -, i.e. invalid direction
    sendCommand("turnLeft");    // to send command to the mouse to turn 90 degrees to the left in the mms simulator                
}

// function for turning around 180 degrees
void turnAround() {
    turnRight();
    turnRight();    // turning right two times equals turning around 
}

// function to move forward in a direction
void moveForward() {
    string res = sendCommand("moveForward");    // send move forward command and store simulator's response
    if (res == "crash") {               // mouse hits walls if response is crash 
        cerr << "Crashed!" << endl;     // print error message
        exit(1);
    }                        
    switch (D) {
        case N: y++; break;             // if the current direction is north, move 1 step on y axis 
        case E: x++; break;             // if the current direction is east, move 1 step on x axis
        case S: y--; break;             // if the current direction is south, move 1 step on -y axis
        case W: x--; break;             // if the current direction is west, move 1 step on -x axis
    }
}

// function to record walls in the present cell 
void recordWalls(bool front, bool left, bool right, bool back) {
    if (x < 0 || x >= maze || y < 0 || y >= maze)   // to exclude the coordinates outside the maze
        return;

    travelled[y][x] = true;             // marks that current cell has been travelled to       

    walls[y][x][D] = front;             // copies boolean value from sensor to the wall data to store if there is a wall to the north of the current cell
    walls[y][x][(D + 3) % 4] = left;    // copies boolean value from sensor to the wall data to store if there is a wall to the west of the current cell 
    walls[y][x][(D + 1) % 4] = right;   // copies boolean value from sensor to the wall data to store if there is a wall to the east of the current cell
    walls[y][x][(D + 2) % 4] = back;    // copies boolean value from sensor to the wall data to store if there is a wall to the south of the current cell
}

void RightWallFollow() {

    //Check for walls on all four sides and store the results
    bool front = getWallFront();        // true if wall is in front
    bool right = getWallRight();        // true if wall is on the right
    bool left = getWallLeft();          // true if wall is on the left 
    bool back = getWallBack();          // true if wall is at the back

    recordWalls(front, left, right, back);  // record walls before moving

    if (right==false) {              // if there is no wall on the right, turn left and move 1 step in the left  
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

int main() {
    while (true) {
        rightWallFollowStep();
    }
    return 0;
}
