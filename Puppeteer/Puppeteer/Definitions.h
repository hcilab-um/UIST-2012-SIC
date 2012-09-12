#pragma once

#define MAX_FINGERS 5
#define MAX_FORCE 500
#define MIN_FORCE 0
#define MOVE_C_CLOCKWISE_LOCATION 0
#define STOP_LOCATION 50
#define MOVE_CLOCKWISE_LOCATION 100
#define LEG_MAX_TICKS 100
#define HAND_MAX_TICKS 100
#define HEAD_MAX_TICKS 100

enum servoMovement{
	eC_Clockwise = -1,
	eStopped = 0,
	eClockwise = 1
};
