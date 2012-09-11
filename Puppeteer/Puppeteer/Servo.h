#pragma once
#include <phidget21.h>
#include "Definitions.h"

class Servo
{
private:
	CPhidgetAdvancedServoHandle servo;
public:
	Servo(void);
	void move(double target);
	~Servo(void);
};

