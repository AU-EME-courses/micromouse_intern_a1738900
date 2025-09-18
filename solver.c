#include "solver.h"
#include "API.h"

Action solver() {
    return leftWallFollower();
}

// This is an example of a simple left wall following algorithm.
Action leftWallFollower() {
    if(API::wallFront()) {
        if(API::wallLeft()){
            return RIGHT;
        }
        return LEFT;
    }
    return FORWARD;
}


// Put your implementation of floodfill here!
Action floodFill() {
    return IDLE;
}

int main() {
    while(true) {
        Action nextMove = solver();
        switch(nextMove) {
            case LEFT:
                API::turnLeft();
                break;
            case RIGHT:
                API::turnRight();
                break;
            case FORWARD:
                API::moveForward(1);
                break;
            case IDLE:
                return 0;
        }
    }
    return 0;
}
