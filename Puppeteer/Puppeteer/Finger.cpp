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
	partControlled = puppetPart;
}

string Finger::getPartName()
{
	if (partControlled)
		return partControlled->getName();
	else 
		return "None";
}

PuppetPart* Finger::getPart()
{
	return partControlled;
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
