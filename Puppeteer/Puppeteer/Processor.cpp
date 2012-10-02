#include "StdAfx.h"
#include "Processor.h"
#include <time.h>

#include <algorithm>
#include <vector>
#include <fstream>

Processor::Processor(ISynDevice* device)
{
	createPacket(device);
	fingerMapRequired = true;
	curCondition=stop;
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
	{
		fingerMapRequired = true;
		disconnectFingers();
	}


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

	handCenter = getFingerAvg_x();

	handLocation curHandLocation = getHandCenterPosition();
	puppet->move(curHandLocation, curCondition, recordFile);		//send movement commands to servos
}

void Processor::print()
{
	//when playing file, puppet is not reading data from force pad
	if(curCondition==play)
	{
		printf("Playing...\n");
	}
	else
	{
		
		if(curCondition==record)
		{
			//show that program is recording data
			printf("Recording...\n");
		}

		for (long i = 0; i < MAX_FINGERS; ++i)
		{
			printf("Finger %d: Coords(%4d, %4d), force: %ld grams, controlling: %s (%4.1f)\n", i, fingers[i].getX(), fingers[i].getY(), fingers[i].getForce(), fingers[i].getPartName().c_str(), fingers[i].getPartTarget());
		}
		printf("Finger Center: %d\n", getHandCenterPosition());
	}
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

handLocation Processor::getHandCenterPosition()
{
	if (handCenter < 0)
		return eCenter;		//Default to center position

	if (curFingerCount == MAX_FINGERS)
	{
		if (handCenter < FIVE_FINGERS_LEFT)
			return eLeft;
		else if (handCenter > FIVE_FINGERS_RIGHT)
			return eRight;

	}
	else if (curFingerCount == 4)	//4 fingers average has a different threshold
	{
		if (handCenter < FOUR_FINGERS_LEFT)
			return eLeft;
		else if  (handCenter > FOUR_FINGERS_RIGHT)
			return eRight;
	}
	
	//Center position or less than 4 fingers
	return eCenter; 
}

double Processor::getFingerAvg_x()
{
	if (curFingerCount<4)
		return -1;	//not enough fingers

	long xFinger,sum = 0;
	for(int i=0; i<MAX_FINGERS; i++)
	{
		xFinger = fingers[i].getX();
		if(xFinger >= 0)
			sum += xFinger;
	}

	return (double) (sum/curFingerCount);
}

//setup for record, get record file, set condition as record
void Processor::startRecord()
{
	curCondition=record;
	recordFile =fopen("C:\\PuppetFile\\puppetRecord.txt","w+");
}

//terminate all record and play process
void Processor::stopRecordPlay()
{
	curCondition=stop;
}

void Processor::disconnectFingers()
{
	for (int i=0; i < MAX_FINGERS; ++i)
	{
		fingers[i].updateControl(NULL);	//clears all fingers
	}
}

Processor::~Processor(void)
{
}
