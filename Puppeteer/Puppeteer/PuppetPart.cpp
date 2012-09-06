#include "StdAfx.h"
#include "PuppetPart.h"

#include "Finger.h"


PuppetPart::PuppetPart(string name)
{
	clearFinger();
	curForce = 0;
	this->name = name;
}

string PuppetPart::getName()
{
	return name;
}

long PuppetPart::getForce()
{
	return curForce;
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
	long newForce = finger->getForce();
	if (newForce > curForce)	//Need to pull part up
	{
		//servo->move
	}
	else if (newForce < curForce) 	//Need to release part
	{
		//servo->move
	}

	curForce = newForce;
}

bool PuppetPart::isActive()
{
	return active;
}

PuppetPart::~PuppetPart(void)
{
}
