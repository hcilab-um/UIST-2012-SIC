#pragma once

#define MAX_INPUT_WAIT 10
#define MAX_FINGERS 5
#define MAX_FORCE 450
#define MIN_FORCE 0

#define MOVE_C_CLOCKWISE_LOCATION 0
#define STOP_LOCATION 50
#define MOVE_CLOCKWISE_LOCATION 100

#define FIVE_FINGERS_LEFT 2300//2700//2900
#define FOUR_FINGERS_LEFT 2900//3300
#define FIVE_FINGERS_RIGHT 4400//3900
#define FOUR_FINGERS_RIGHT 3500

#define CYCLE_TIME	20//in ms
#define MAX_SPEED_LEVEL 1//3
#define MIN_SPEED_LEVEL -1//-3
#define LEG_MAX_TICKS 40//100//300
#define LEG_TICKS_MID_LEVEL 20
#define HAND_MAX_TICKS 35//100//350
#define HAND_TICKS_MID_LEVEL 20
#define HEAD_MAX_TICKS 40//150
#define THRESHOLD_TICKS 10//20	//less than x ticks should not move servo from current position

#define STOPPING_TICKS 1

#define MAX_LEFT_WHEEL_TICKS 75//125
#define MAX_RIGHT_WHEEL_TICKS -75//-125

enum servoMovement{
	eC_Clockwise = -1,
	eStopped = 0,
	eClockwise = 1
};

enum handLocation{
	eLeft = -1,
	eCenter = 0,
	eRight = 1
};

enum recordPlayMode
{
	operate = 'o',
	record = 'r',
	stop = 's',
	play = 'p'
};