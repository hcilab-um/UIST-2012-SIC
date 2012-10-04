#pragma once

#include <SynKit.h>
#include <string>
#include "PuppetPart.h"

using namespace std;

class Finger
{
private:
	long _lX, _lY;
	PuppetPart* partControlled;

public:
	Finger(void);
	void remove(void);
	string getPartName();
	long getPartTarget();
	long getTicks();
	void updateControl(PuppetPart* puppetPart);
	void updateData(ISynPacket* data);
	long getX(void);
	long getY(void);
	long getForce(void);
	long _lZForce;
	~Finger(void);
};

