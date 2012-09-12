#pragma once
#include <phidget21.h>
#include "Definitions.h"

//Servo Controller class

class ServoC
{
private:
	CPhidgetAdvancedServoHandle servo;

public:
	
	ServoC(void);
	void setMovement(servoMovement movement, int motorId);
	~ServoC(void);
};

