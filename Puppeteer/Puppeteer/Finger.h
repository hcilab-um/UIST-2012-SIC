#pragma once

#include <SynKit.h>
#include <string>
#include "PuppetPart.h"

using namespace std;

class Finger
{
private:
	long _lX, _lY, _lZForce;
	PuppetPart* partControlled;

public:
	Finger(void);
	void remove(void);
	string getPartName();
	double getPartTarget();
	void updateControl(PuppetPart* puppetPart);
	void updateData(ISynPacket* data);
	long getX(void);
	long getY(void);
	long getForce(void);
	~Finger(void);
};

