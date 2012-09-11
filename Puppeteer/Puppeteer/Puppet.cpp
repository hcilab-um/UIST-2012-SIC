#include "StdAfx.h"
#include "Puppet.h"


Puppet::Puppet(void)
{
	body["lLeg"] = new PuppetPart("Left Leg");
	body["lHand"] = new PuppetPart("Left Hand");
	body["head"] = new PuppetPart("Head");
	body["rHand"] = new PuppetPart("Right Hand");
	body["rLeg"] = new PuppetPart("Right Leg");
}

void Puppet::move()
{
	//Order - Right Leg, Left Leg, Right Hand, Left Hand, Head?
	map<string, PuppetPart*>::iterator iter;
	for (iter = body.begin(); iter != body.end(); ++iter)
	{
		if (!iter->second->isActive())
			continue;	//inactive part

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
}
