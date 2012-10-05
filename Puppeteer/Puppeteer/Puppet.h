#pragma once
#include "PuppetPart.h"
#include <string>
#include <map>
#include <windows.h>

#include <fstream>

using namespace std;

class Puppet
{
private:
	long wheelTicks;
	handLocation curWheelMovement;
	void moveWheels(handLocation curHandLocation);
public:
	ServoC* controller;
	ofstream myfile;
	map<string, PuppetPart*> body;
	Puppet(void);
	void move(handLocation curHandLocation, recordPlayMode mode, FILE* recordFile);
	void moveWheelsCenter();
	~Puppet(void);
};

