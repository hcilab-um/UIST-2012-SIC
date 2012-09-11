#pragma once

#include <string>
#include "Definitions.h"
#include "Servo.h"



using namespace std;
class Finger;

class PuppetPart
{
private:
	bool active;
	long curForce;
	string name;
	double target;
public:
	Finger* finger;

	Servo* motor;
	PuppetPart(string name);
	void clearFinger();
	void setActive(bool activate);
	void linkFinger(Finger* finger);
	bool isActive();
	void move();
	string getName();
	double getTarget();
	long getForce();
	~PuppetPart(void);
};

