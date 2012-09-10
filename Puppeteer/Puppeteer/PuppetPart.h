#pragma once

#include <string>



using namespace std;
class Finger;

class PuppetPart
{
private:
	bool active;
	long curForce;
	string name;
public:
	Finger* finger;

	//Servo motor;
	PuppetPart(string name);
	void clearFinger();
	void setActive(bool activate);
	void linkFinger(Finger* finger);
	bool isActive();
	void move();
	string getName();
	long getForce();
	~PuppetPart(void);
};

