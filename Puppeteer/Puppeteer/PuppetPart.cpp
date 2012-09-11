#include "StdAfx.h"
#include "PuppetPart.h"

#include "Finger.h"


PuppetPart::PuppetPart(string name)
{
	clearFinger();
	curForce = 0;
	target = 0;
	this->name = name;

	
	if (!name.compare("Head"))
		motor = new Servo();	//temp test initialization
	else motor = NULL;
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
	return target;
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

	long newForce = finger->getForce();

	if (newForce > MAX_FORCE)	//Don't pull more than maximum
		newForce = MAX_FORCE;
	
	double forceRatio = (double) newForce / MAX_FORCE;	//How much force is exerted relative to the max
	double relativeLocationFactor = (MAX_SERVO_LOCATION - MIN_SERVO_LOCATION);

	target = forceRatio * relativeLocationFactor;	//move to relative location on motor
	
	if (motor) 
		motor->move(target);	

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
	delete motor;	//clean servos
}
