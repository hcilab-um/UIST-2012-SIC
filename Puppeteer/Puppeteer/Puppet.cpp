#include "StdAfx.h"
#include "Puppet.h"
#include "ServoC.h"


Puppet::Puppet(void)
{
	controller = new ServoC();	//init servos controller

	body["lLeg"] = new PuppetPart("Left Leg", LEG_MAX_TICKS, controller, 2, true);
	body["lHand"] = new PuppetPart("Left Hand", HAND_MAX_TICKS, controller, 1, true);
	body["head"] = new PuppetPart("Head", HEAD_MAX_TICKS, controller, 0, true);
	body["rHand"] = new PuppetPart("Right Hand", HAND_MAX_TICKS, controller, 3, true);
	body["rLeg"] = new PuppetPart("Right Leg", LEG_MAX_TICKS, controller, 4, true);
}

void Puppet::move()
{
	//Order - Right Leg, Left Leg, Right Hand, Left Hand, Head?
	map<string, PuppetPart*>::iterator iter;
	for (iter = body.begin(); iter != body.end(); ++iter)
	{
		iter->second->move();	//decide if part needs moving
	}

}


Puppet::~Puppet(void)
{
	delete body["lLeg"];
	delete body["lHand"];
	delete body["head"]; 
	delete body["rHand"];
	delete body["rLeg"];
	
	delete controller;
}
