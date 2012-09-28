#include "StdAfx.h"
#include "Finger.h"


Finger::Finger(void)
{
	remove();
	partControlled = NULL;
}

void Finger::updateData(ISynPacket* data)
{
	data->GetProperty(SP_X, &_lX);
	data->GetProperty(SP_Y, &_lY);
	data->GetProperty(SP_ZForce, &_lZForce);
}

void Finger::remove()
{
	_lX = -1;
	_lY = -1;
	_lZForce = -1;
}

void Finger::updateControl(PuppetPart* puppetPart)
{
	if ((!puppetPart) && (partControlled))	//received NULL, disconnect
		partControlled->clearFinger();

	partControlled = puppetPart;
}

string Finger::getPartName()
{
	if (partControlled)
		return partControlled->getName();
	else 
		return "None";
}

double Finger::getPartTarget()
{
	if (partControlled)
		return partControlled->getTarget();

	else return -1;		//Signal no location
}

long Finger::getX()
{
	return _lX;
}

long Finger::getY()
{
	return _lY;
}

long Finger::getForce()
{
	return _lZForce;
}

Finger::~Finger(void)
{
}
