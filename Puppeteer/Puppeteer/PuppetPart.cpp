#include "StdAfx.h"
#include "PuppetPart.h"
#include <windows.h>

#include "Finger.h"

#include <iostream>	//file writing
#include <fstream>
#include <conio.h>

double PCFreq = 0.0;	//QPC
__int64 CounterStart = 0;

/*****************/
void StartCounter()
{
    LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);

    PCFreq = double(li.QuadPart)/1000.0;

    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
}

double GetCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart-CounterStart)/PCFreq;
}
/*****************/

PuppetPart::PuppetPart(string name, long maxTicks, ServoC* controller, int motorId, bool isPullClockwise)
{
	clearFinger();
	curForce = 0;
	curMovement = eStopped;
	curTicks = 0;
	speedLevel = 0; oldTime =0; currentTime = 0; StartCounter();
	targetTicks = 0;
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

long PuppetPart::getCurrentTicks()
{
	return curTicks;
}

long PuppetPart::getTargetTicks()
{
	return targetTicks;
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

void PuppetPart::move(ofstream& myfile)
{
	long newForce;
	servoMovement targetMovement;

	if (!active)		//Finger has been lifted, release part
	{
		targetTicks = MIN_FORCE;
		newForce = MIN_FORCE;
	}
	else
	{	
		newForce = finger->getForce();

		if (newForce > MAX_FORCE)	//Don't pull more than maximum
			newForce = MAX_FORCE;

		if (newForce < MIN_FORCE)
			newForce = MIN_FORCE;
		
		double forceRatio = (double) newForce / MAX_FORCE;	//How much force is exerted relative to the max
		//double relativeLocationFactor = (MAX_SERVO_LOCATION - MIN_SERVO_LOCATION);

		targetTicks = (long) (forceRatio * maxTicks);	//move to relative location on motor
		int pulleyLevel = (targetTicks / THRESHOLD_TICKS);
		targetTicks = (long) (pulleyLevel * THRESHOLD_TICKS);
	}
	
	if (curTicks < targetTicks)		//Need to pull string
	{
		if ((curTicks <= (targetTicks-THRESHOLD_TICKS)) || (targetTicks == maxTicks))		//move if target further than threshold or at top edge
		//if ((curTicks > 0) || (targetTicks >= THRESHOLD_TICKS))	//don't move when force is very weak
		{
			if (isPullClockwise)	//figure pull direction
				targetMovement = eClockwise;		
			else targetMovement = eC_Clockwise;

			//++curTicks;
		}
		else targetMovement = eStopped;
	}
	
	else if (curTicks > targetTicks)	//Need to release string
	{
		if ((curTicks >= (targetTicks+THRESHOLD_TICKS)) || ((targetTicks <= 0) && (curTicks > 2*MAX_SPEED_LEVEL)))
		//if ((curTicks < maxTicks) || (targetTicks <= (maxTicks-THRESHOLD_TICKS)))	//don't move when force is slightly weaker than max
		{
			if (isPullClockwise)	//figure release direction
				targetMovement = eC_Clockwise;		
			else targetMovement = eClockwise;

			//--curTicks;
		}
		else targetMovement = eStopped;
	}
	else targetMovement = eStopped;		//Correct position

	//updateSpeed(targetMovement);
	
		oldTime = currentTime;
	if (getName() == "Right Leg")
	myfile << "curMov:" << curMovement << ",tarMov:" << targetMovement <<",ticks:" << curTicks <<","<< targetTicks << "|" << speedLevel << endl;
	/*
	if (getName() == "Left Leg" && (getch() == 'z'))
	do
	{
		//test1
		motor->setMovement(eClockwise, 4);
		Sleep(50);
		motor->setMovement(eStopped, 4);
		Sleep(550);
		motor->setMovement(eC_Clockwise, 4);
		Sleep(72);
		motor->setMovement(eStopped, 4);
		Sleep(550);
		//
		
		
		motor->setMovement(eClockwise, 4);
		Sleep(1250);
		//Sleep(1000);

		motor->setMovement(eStopped, 4);
		Sleep(3000);

		for (int i=0; i<3;i++)
		{
			motor->setMovement(eClockwise, 4);
			Sleep(30);
			//Sleep(100);

			motor->setMovement(eStopped, 4);
			Sleep(1000);

			motor->setMovement(eC_Clockwise, 4);
			Sleep(60);
			//Sleep(100);

			motor->setMovement(eStopped, 4);
			Sleep(1000);
		}
		
		printf(" done\n");
		getch();
		 

	}
	while (!_kbhit());
	*/

	updateTicks(targetMovement, targetTicks);

	if (curMovement != targetMovement)	//Change motor movement
	{
		if (motor) 
			motor->setMovement(targetMovement, motorId);

		curMovement = targetMovement;
	}
	
	//currentTime = GetCounter();


	
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

//Used to keep track of accelerations, stoppage and movement reversals of the servo
void PuppetPart::updateTicks(servoMovement& targetMovement, long& targetTicks)
{
	/* ticks update when changing movement types
	if (targetMovement == eStopped)
	{
		if (curMovement == eC_Clockwise)
		{
			if (isPullClockwise)
				curTicks -= STOPPING_TICKS;
			else curTicks += STOPPING_TICKS;
		}
		else	//Clockwise 
		{
			if (isPullClockwise)
				curTicks += STOPPING_TICKS;
			else curTicks -= STOPPING_TICKS;
		}
	}
	else if (targetMovement == eC_Clockwise)
	{
		if (curMovement == eClockwise)
		{
			if (isPullClockwise)
				curTicks += 2*STOPPING_TICKS;
			else curTicks -= 2*STOPPING_TICKS;
		}
		else	//Stopped
		{
			if (isPullClockwise)
				curTicks += STOPPING_TICKS;	//Don't reduce ticks yet
			else curTicks -= STOPPING_TICKS;
		}
	}
	else	//Clockwise
	{
		if (curMovement == eC_Clockwise)
		{
			if (isPullClockwise)
				curTicks -= 2*STOPPING_TICKS;
			else curTicks += 2*STOPPING_TICKS;
		}
		else	//Stopped
		{
			if (isPullClockwise)
				curTicks -= STOPPING_TICKS;	//Don't increment ticks yet
			else curTicks += STOPPING_TICKS;
		}
	}

	if (curTicks < MOVE_C_CLOCKWISE_LOCATION)	//Fix over adjustment
		curTicks = MOVE_C_CLOCKWISE_LOCATION;
	
	else if (curTicks > MOVE_CLOCKWISE_LOCATION)  //Fix over adjustment
		curTicks = MOVE_CLOCKWISE_LOCATION;
		*/


	//currentTime = GetCounter();
	
	//printf("time DIFF: %f" , currentTime - oldTime);

	//oldTime = currentTime;
	//measure time passed from start of movement, multiply by speed?, small movements should not count much?
	
	//update speed, update ticks
	if (targetMovement == eClockwise) 
	{
		if ((isPullClockwise) && (speedLevel != MAX_SPEED_LEVEL))
			speedLevel++;
		else if ((!isPullClockwise) && (speedLevel != MIN_SPEED_LEVEL))
			speedLevel--;
		
		if (curMovement == eC_Clockwise)	//change speed direction fast
			speedLevel = 1;
	}
	else if (targetMovement == eC_Clockwise)
	{
		if ((isPullClockwise) && (speedLevel != MIN_SPEED_LEVEL))
			speedLevel--;
		else if ((!isPullClockwise) && (speedLevel != MAX_SPEED_LEVEL))
			speedLevel++;

		if (curMovement != eC_Clockwise)	//change speed direction
			speedLevel = -1;
	}
	else	//Stopping
	{
		if (curMovement == eClockwise) //ccompensate for clockwise weak stopping
		{
			if (isPullClockwise)
				curTicks++;
			else
				curTicks--;
		}

		/*
		if (speedLevel > 0)
			speedLevel--;
		else if (speedLevel < 0)
			speedLevel++;
			*/
		speedLevel = 0;
	}

	//update ticks
	curTicks += speedLevel;
	/*if (speedLevel)
	{
		if (curTicks < targetTicks)
			curTicks += speedLevel;
		else if (curTicks < targetTicks)
			curTicks -= speedLevel;
	}*/
}

PuppetPart::~PuppetPart(void)
{
}
