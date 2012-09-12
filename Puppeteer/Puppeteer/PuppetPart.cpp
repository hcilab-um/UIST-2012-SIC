#include "StdAfx.h"
#include "PuppetPart.h"

#include "Finger.h"


PuppetPart::PuppetPart(string name, long maxTicks, ServoC* controller, int motorId, bool isPullClockwise)
{
	clearFinger();
	curForce = 0;
	curMovement = eStopped;
	curTicks = 0;
	target = 0;
	active = false;
	this->name = name;
	this->maxTicks = maxTicks;
	this->isPullClockwise = isPullClockwise;
	motor = controller;	//access to servo controller
	this->motorId = motorId;	//Id of motor controlling current part
}

string PuppetPart::getName()
{
	return name;
}

long PuppetPart::getForce()
{
	return curForce;
}

double PuppetPart::getTarget()
{
	return curTicks;
}

void PuppetPart::clearFinger()
{
	finger = NULL;
	active = false;
}


void PuppetPart::linkFinger(Finger* finger)
{
	if (!finger)
		clearFinger();
	else
	{
		this->finger = finger;
		finger->updateControl(this);
		active = true;
	}

}

void PuppetPart::move()
{
	if (!active)		//Finger has been lifted, don't move part
		return;

	servoMovement targetMovement;
	//if (finger->_lZForce == -1)
	long newForce = finger->getForce();

	if (newForce > MAX_FORCE)	//Don't pull more than maximum
		newForce = MAX_FORCE;

	if (newForce < MIN_FORCE)
		newForce = MIN_FORCE;
	
	double forceRatio = (double) newForce / MAX_FORCE;	//How much force is exerted relative to the max
	//double relativeLocationFactor = (MAX_SERVO_LOCATION - MIN_SERVO_LOCATION);

	long targetTicks = (long) (forceRatio * maxTicks);	//move to relative location on motor
	
	if (curTicks < targetTicks)		//Need to pull string
	{
		if (isPullClockwise)	//figure pull direction
			targetMovement = eClockwise;		
		else targetMovement = eC_Clockwise;

		++curTicks;
	}
	
	else if (curTicks > targetTicks)	//Need to release string
	{
		if (isPullClockwise)	//figure release direction
			targetMovement = eC_Clockwise;		
		else targetMovement = eClockwise;

		--curTicks;
	}
	else targetMovement = eStopped;		//Correct position
		
	if (curMovement != targetMovement)	//Change motor movement
	{
		if (motor) 
			motor->setMovement(targetMovement, motorId);

		curMovement = targetMovement;
	}

	curForce = newForce;
}

bool PuppetPart::isActive()
{
	return active;
}

void PuppetPart::setActive(bool activate)
{
	active = activate;
}

PuppetPart::~PuppetPart(void)
{
}
