#include "StdAfx.h"
#include "Puppet.h"
#include "ServoC.h"


Puppet::Puppet(void)
{
	controller = new ServoC();	//init servos controller

	body["lLeg"] = new PuppetPart("Left Leg", LEG_MAX_TICKS, controller, 3, false);	//MotorId - 0-3 are servo1, 4-6 are servo2
	body["lHand"] = new PuppetPart("Left Hand", HAND_MAX_TICKS, controller, 2, true);
	body["head"] = new PuppetPart("Head", HEAD_MAX_TICKS, controller, 7, true);	//disabled
	body["rHand"] = new PuppetPart("Right Hand", HAND_MAX_TICKS, controller, 1, false);
	body["rLeg"] = new PuppetPart("Right Leg", LEG_MAX_TICKS, controller, 0, false);

	wheelTicks = 0;
	curWheelMovement = eCenter;

	myfile.open ("example.txt");
}

void Puppet::move(handLocation curHandLocation, recordPlayMode mode, FILE* recordFile)
{
	map<string, PuppetPart*>::iterator iter;
	for (iter = body.begin(); iter != body.end(); ++iter)
	{
		iter->second->move(myfile);	//decide if part needs moving
	}

	moveWheels(curHandLocation);

	if(mode == record)
	{
		for (iter = body.begin(); iter != body.end(); ++iter)
		{
			fprintf(recordFile, "%ld,",iter->second->getForce());	//decide if part needs moving
		}
		fprintf(recordFile,"%d\n",curHandLocation);
	}
}

void Puppet::moveWheels(handLocation curHandLocation)
{
	//Back wheel is motorId 6 (counter-clockwise left), front is motorId 5 (clockwise left)
	if (curHandLocation == eLeft)
	{
		if (wheelTicks < MAX_LEFT_WHEEL_TICKS)
		{
			if (curWheelMovement != eLeft)
			{
				controller->setMovement(eClockwise,5);
				controller->setMovement(eC_Clockwise,6);
				curWheelMovement = eLeft;
			}

			wheelTicks++;
			return;
		}
	}
	else if (curHandLocation == eRight)
	{
		if (wheelTicks > MAX_RIGHT_WHEEL_TICKS)
		{
			if (curWheelMovement != eRight)
			{
				controller->setMovement(eC_Clockwise,5);
				controller->setMovement(eClockwise,6);
				curWheelMovement = eRight;
			}
			
			wheelTicks--;
			return;
		}
	}

	//Either stop position or max location
	if (curWheelMovement != eCenter)
	{
		controller->setMovement(eStopped,5);
		controller->setMovement(eStopped,6);
		curWheelMovement = eCenter;
	}
}

void Puppet::moveWheelsCenter()
{
	//adjust the wheel, until it gets to center: wheelTicks=0
	while (wheelTicks != eCenter)
	{
		//on left side
		if (wheelTicks > eCenter)
		{
			//move right
			controller->setMovement(eC_Clockwise,5);
			controller->setMovement(eClockwise,6);
			wheelTicks--;
		}
		else	//on right side
		{
			//move left
			controller->setMovement(eClockwise,5);
			controller->setMovement(eC_Clockwise,6);
			wheelTicks++;
		}

		Sleep(CYCLE_TIME);
	}

	controller->setMovement(eStopped,5);
	controller->setMovement(eStopped,6);
	curWheelMovement = eCenter;
}

Puppet::~Puppet(void)
{
	delete body["lLeg"];
	delete body["lHand"];
	delete body["head"]; 
	delete body["rHand"];
	delete body["rLeg"];
	
	delete controller;

	myfile.close();
}
