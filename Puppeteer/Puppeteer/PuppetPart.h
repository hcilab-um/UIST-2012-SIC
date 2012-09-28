#pragma once

#include <string>
#include "Definitions.h"
#include "ServoC.h"



using namespace std;
class Finger;

class PuppetPart
{
private:
	bool active;
	long curForce;
	string name;
	double target;
	long curTicks;	//Current location of motor, tick = length of string pulled or released every ~10ms
	long maxTicks;	//Maximum time for part pull
	bool isPullClockwise;	//true if clockwise motion will pull string up
	servoMovement curMovement;
	int motorId;
	int speedLevel;
	void updateTicks(servoMovement& targetMovement, long& targetTicks);
	double oldTime, currentTime;	//QPC
public:
	Finger* finger;
	ServoC* motor;

	PuppetPart(string name, long maxTicks, ServoC* controller, int motorId, bool isPullClockwise );
	void clearFinger();
	void setActive(bool activate);
	void linkFinger(Finger* finger);
	bool isActive();
	void move(ofstream& myfile);
	string getName();
	double getTarget();
	long getForce();
	~PuppetPart(void);
};

