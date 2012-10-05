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
	TESTCOUNTER = 0, TESTDIR =1;

	fillCorrections();
	movLength = 0;
	inMovement = false;
}


void PuppetPart::fillCorrections()
{
	if (getName() == "Left Leg")
	for (int i = 0; i < LEG_MAX_TICKS+THRESHOLD_TICKS; i++)
	{
		if (i == 0)
			correction[i] = eC_Clockwise;	//Servo stops wrong on counter clockwise
		//else if (i <= 3)
		//	correction[i] = 0;//1;
		//else if (i <= 9)
		//	correction[i] = 1;//2;
		else if (i <= 20)
			correction[i] = 0;//3;
		else if (i <= 30)
			correction[i] = 0;//4;
		else if (i <= 40)
			correction[i] = 2;//5;
		else if (i <= 50)
			correction[i] = 2;//6;
		//else if (i <= 60)
		//	correction[i] = 3;
		//else if (i <= 70)
		//	correction[i] = 3;

	}

	else if (getName() == "Left Hand")
	for (int i = 0; i < LEG_MAX_TICKS+THRESHOLD_TICKS; i++)
	{
		if (i == 0)
			correction[i] = eClockwise;		//Servo stops wrong on counter clockwise
		else if (i <= 20)
			correction[i] = 1;//3;
		else if (i <= 30)
			correction[i] = 1;//4;
		else if (i <= 40)
			correction[i] = 1;//5;
		else if (i <= 50)
			correction[i] = 1;//6;
		else if (i <= 60)
			correction[i] = 3;
		else if (i <= 70)
			correction[i] = 3;
	}

	else if (getName() == "Right Hand")
	for (int i = 0; i < LEG_MAX_TICKS+THRESHOLD_TICKS; i++)
	{
		if (i == 0)
			correction[i] = eClockwise;	//Servo stops wrong on clockwise
		else if (i <= 20)
			correction[i] = 4;
		else if (i <= 30)
			correction[i] = 4;
		else if (i <= 40)
			correction[i] = 4;
		else if (i <= 50)
			correction[i] = 4;
		//else if (i <= 60)
		//	correction[i] = 4;
		//else if (i <= 70)
		//	correction[i] = 4;
	}
	else if (getName() == "Right Leg")
	for (int i = 0; i < LEG_MAX_TICKS+THRESHOLD_TICKS; i++)
	{
		if (i == 0)
			correction[i] = eClockwise;		//Servo stops wrong on clockwise
		else if (i <= 9)
			correction[i] = 1;//2;
		else if (i <= 20)
			correction[i] = 2;//3;
		else if (i <= 30)
			correction[i] = 2;//4;
		else if (i <= 50)
			correction[i] = 2;//5;
		else if (i <= 60)
			correction[i] = 2;
		else if (i <= 70)
			correction[i] = 2;
	}
	else
		for (int i = 0; i < LEG_MAX_TICKS+THRESHOLD_TICKS; i++)
			correction[i] = 0;

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
	servoMovement targetMovement;

	calcTargetByForce();

	if (!inMovement)
		moveToTicks = targetTicks;	//Update target
	

	if (((curTicks < moveToTicks) && (inMovement)) || ((curTicks < moveToTicks-THRESHOLD_TICKS) && (!inMovement)))		//Need to pull string
	{
		if (isPullClockwise)	//figure release direction
			targetMovement = eClockwise;
		else
			targetMovement = eC_Clockwise;

		if (!inMovement)
			inMovement = true;
	}
	else if (((curTicks > moveToTicks) && (inMovement)) || ((curTicks > moveToTicks+THRESHOLD_TICKS) && (!inMovement)))	//Need to release string
	{
		if (isPullClockwise)	//figure release direction
			targetMovement = eC_Clockwise;		
		else 
			targetMovement = eClockwise;
		
		if (!inMovement)
			inMovement = true;
	}
	else 
	{
		if (targetTicks == moveToTicks)
		{
			targetMovement = eStopped;		//Correct position
			inMovement = false;
		}
		else 
		{
			if (moveToTicks < targetTicks)		//Need to pull string
			{
				if (isPullClockwise)	//figure release direction
					targetMovement = eClockwise;
				else
					targetMovement = eC_Clockwise;
			}
			
			else if (moveToTicks > targetTicks)	//Need to release string
			{
				if (isPullClockwise)	//figure release direction
					targetMovement = eC_Clockwise;		
				else targetMovement = eClockwise;
			}

			moveToTicks = targetTicks;
		}
	}
	

	oldTime = currentTime;

	if (getName() == "Right Hand")
	myfile << "curMov:" << curMovement << ",tarMov:" << targetMovement <<",ticks:" << curTicks <<","<< moveToTicks << ","<< targetTicks <<"|sp:" << speedLevel << "|len:" << movLength << endl;
	/*
	if (getName() == "Left Leg" && (getch() == 'z'))
	do
	{
		
		//test1
		motor->setMovement(eClockwise, 3);
		Sleep(50);
		motor->setMovement(eStopped, 3);
		Sleep(550);
		motor->setMovement(eC_Clockwise, 3);
		Sleep(50);
		motor->setMovement(eStopped, 3);
		Sleep(550);

		Sleep(500);
		//
		
		
		//motor->setMovement(eClockwise, 4);
		//Sleep(1250);
		//Sleep(1000);

		//motor->setMovement(eStopped, 4);
		//Sleep(3000);
		/*
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
	while (1);//!_kbhit());
	*/

	/*
	//  TEST SEGMENT
	if (getName() == "Left Leg")
	{
		if (TESTDIR == 1)
		{
			if (TESTCOUNTER < 67)
			{
				targetMovement = eClockwise;
				TESTCOUNTER++;
			}
			else if ((TESTCOUNTER >= 67) && (TESTCOUNTER < 100))
			{
				targetMovement = eStopped;
				TESTCOUNTER++;
			}
			else if (TESTCOUNTER == 100)
			{
				TESTDIR = -1;
				targetMovement = eC_Clockwise;
			}
		}
		else if (TESTDIR == -1)
		{
			if ((TESTCOUNTER >= 100) && (TESTCOUNTER < 100+60))
			{
				TESTCOUNTER++;
				targetMovement = eC_Clockwise;
			}
			else if (TESTCOUNTER < 200)
			{
				TESTCOUNTER++;
				targetMovement = eStopped;
			}
			else if (TESTCOUNTER == 200)
			{
				TESTDIR = 1;
				TESTCOUNTER = 0;
				targetMovement = eClockwise;
			}
		}
	*/

	updateTicks(targetMovement, moveToTicks);

	if (curMovement != targetMovement)//if ((curMovement != targetMovement) && (getName() == "Left Leg"))	//Change motor movement
	{
		if (motor) 
			motor->setMovement(targetMovement, motorId);

		curMovement = targetMovement;
	}
	
	//currentTime = GetCounter();
	
	//curForce = newForce;
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

	if (curMovement != targetMovement)	//Correct stopping
	{
			if ((curMovement == eC_Clockwise) && (correction[0] == eC_Clockwise))  //compensate for counter clockwise weak stopping
			{
				if (isPullClockwise)
					curTicks -= correction[movLength];
				else
					curTicks += correction[movLength];
			}
			else if ((curMovement == eClockwise) && (correction[0] == eClockwise))	//compensate for clockwise weak stopping
			{
				if (isPullClockwise)
					curTicks += correction[movLength];
				else
					curTicks -= correction[movLength];
			}

			if (targetMovement == eStopped)
				movLength = 0;
	}


	//update speed, update ticks
	if (targetMovement == eClockwise) 
	{
		if ((isPullClockwise) && (speedLevel != MAX_SPEED_LEVEL))
			speedLevel++;
		else if ((!isPullClockwise) && (speedLevel != MIN_SPEED_LEVEL))
			speedLevel--;
		
		if (curMovement == eC_Clockwise)	//change speed direction fast
			movLength = 0;

		movLength++;
	}
	else if (targetMovement == eC_Clockwise)
	{
		if ((isPullClockwise) && (speedLevel != MIN_SPEED_LEVEL))
			speedLevel--;
		else if ((!isPullClockwise) && (speedLevel != MAX_SPEED_LEVEL))
			speedLevel++;

		if (curMovement != eC_Clockwise)	//change speed direction
			movLength = 0;

		movLength++;
	}
	else	//Stopping
	{
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
}

void PuppetPart::calcTargetByForce()
{
	long newForce;

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

		string name = getName();
		if (name == "Right Leg" || name == "Left Leg")
		{
			targetTicks = (long) (forceRatio * maxTicks);	//move to relative location on motor
			if (targetTicks < LEG_TICKS_MID_LEVEL - THRESHOLD_TICKS)
				targetTicks = 0;	//Stay
			else if (targetTicks < LEG_TICKS_MID_LEVEL + THRESHOLD_TICKS)
				targetTicks = LEG_TICKS_MID_LEVEL;
			else targetTicks = LEG_MAX_TICKS;
		}

		else	//Hands + head
		{
			targetTicks = (long) (forceRatio * maxTicks);	//move to relative location on motor
			if (targetTicks < HAND_TICKS_MID_LEVEL - THRESHOLD_TICKS)
				targetTicks = 0;	//Stay
			else if (targetTicks < HAND_TICKS_MID_LEVEL + THRESHOLD_TICKS)
				targetTicks = HAND_TICKS_MID_LEVEL;
			else targetTicks = HAND_MAX_TICKS;
		}
	}

	curForce = newForce;
}


PuppetPart::~PuppetPart(void)
{
}
