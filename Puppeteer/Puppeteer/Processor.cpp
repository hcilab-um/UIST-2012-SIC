#include "StdAfx.h"
#include "Processor.h"
#include <time.h>

#include <algorithm>
#include <vector>

Processor::Processor(ISynDevice* device)
{
	createPacket(device);
	fingerMapRequired = true;
}

void Processor::createPacket(ISynDevice* device)
{
	// Create an ISynPacket instance to receive per-touch data
	device->CreatePacket(&_packet);
}

void Processor::processData(Puppet* puppet, ISynGroup* dataGroup)
{
	if (!dataGroup)
	{
		printf("Error - No data group found!");
		return;
	}

	int newFingerCount = 0;
	for (int i = 0; i < MAX_FINGERS; ++i)	//Check new fingers state
    {    
		// Load data into the SynPacket object
		dataGroup->GetPacketByIndex(i, _packet);

		// Is there a finger present?
		long lFingerState;
		_packet->GetProperty(SP_FingerState, &lFingerState);

		if (lFingerState & SF_FingerPresent)
		{
		    ++newFingerCount;

		    // Extract the position and force of the touch
			fingers[i].updateData(_packet);
		}
		else
		{
			fingers[i].remove();
		}
	}


	if ((curFingerCount >= (MAX_FINGERS-1)) && (newFingerCount < (MAX_FINGERS-1)))		//unless only one finger was lifted we need to remap fingers
		fingerMapRequired = true;


	if ((fingerMapRequired) && (newFingerCount == MAX_FINGERS))	
	{	//5 fingers found - new detection formation, update finger roles
		
		vector <sortPair> xSortArray;	//sort fingers from left to right
		for (int i = 0; i < MAX_FINGERS; ++i)
		{
			xSortArray.push_back(sortPair(fingers[i].getX(), &fingers[i]));
		}

		sort(xSortArray.begin(), xSortArray.end(), cmpByX);		

		//Set fingers to puppet parts
		puppet->body["lLeg"]->linkFinger(xSortArray.at(0).second);
		puppet->body["lHand"]->linkFinger(xSortArray.at(1).second);
		puppet->body["head"]->linkFinger(xSortArray.at(2).second);
		puppet->body["rHand"]->linkFinger(xSortArray.at(3).second);
		puppet->body["rLeg"]->linkFinger(xSortArray.at(4).second);

		fingerMapRequired = false;	//done
	}

	curFingerCount = newFingerCount;

	puppet->move();		//send movement commands to servos
}

long Processor::getFingerAvg_x()
{
	long sum = 0;
	for(int i=0; i<MAX_FINGERS; i++)
	{
		if(fingers[i].getX() == -1)
			return -1;
		sum+=fingers[i].getX();

	}
	return sum/MAX_FINGERS;
}

long Processor::getFingerAvg_y()
{
	long sum = 0;
	for(int i = 0; i < MAX_FINGERS; i++)
	{
		if(fingers[i].getY() == -1)
			return -1;
		sum += fingers[i].getY();
	}
	return sum / MAX_FINGERS;
}

void Processor::print()
{
	for (long i = 0; i < MAX_FINGERS; ++i)
	{
		printf("Finger %d: Coords(%4d, %4d), force: %ld grams, controlling: %s (%4.1f)\n", i, fingers[i].getX(), fingers[i].getY(), fingers[i].getForce(), fingers[i].getPartName().c_str(), fingers[i].getPartTarget());
	}

	printf("Finger Center Coords(%4d, %4d)\n", getFingerAvg_x(), getFingerAvg_y());

	SYSTEMTIME st;  //Time printing
    GetSystemTime(&st);

	printf("time: %ld.%ld ms \n\n", st.wSecond, st.wMilliseconds);
	
}

bool cmpByX(const sortPair& a, const sortPair& b)
{
	if (a.first <= b.first)
		return true;
	else
		return false;
}

Processor::~Processor(void)
{
}
