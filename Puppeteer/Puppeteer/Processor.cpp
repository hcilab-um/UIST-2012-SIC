#include "StdAfx.h"
#include "Processor.h"
#include <time.h>

#include <algorithm>
#include <vector>


Processor::Processor(ISynDevice* device)
{
	createPacket(device);
	fingerMapRequired = true;
	curFingerCount = -1;
	curHandLocation = eCenter;
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
	curHandLocation=eCenter;
	
	//if not playing from file, just do as it was
	if(curCondition != play)
	{
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
				//set all data to -1
				fingers[i].remove();
			}
		}

		//re-map data: should not in play process
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
		
			//sort process should not happen in play
			sort(xSortArray.begin(), xSortArray.end(), cmpByX);		

			//Set fingers to puppet parts								
			puppet->body["lLeg"]->linkFinger(xSortArray.at(0).second);				//this coulds be in play process once
			puppet->body["lHand"]->linkFinger(xSortArray.at(1).second);
			puppet->body["head"]->linkFinger(xSortArray.at(2).second);
			puppet->body["rHand"]->linkFinger(xSortArray.at(3).second);
			puppet->body["rLeg"]->linkFinger(xSortArray.at(4).second);

			fingerMapRequired = false;	//done
		}
		handCenter = getFingerAvg_x();

		curHandLocation = getHandCenterPosition();
	}
	else
	{
		//if playing from file, make sure it gives same time delaying as before
		Sleep(13);
		
		//check if the file is being correctly read(if it gets to end of file)
		if(6==fscanf(playFile, "%ld,%ld,%ld,%ld,%ld,%d\n", &fingers[0]._lZForce, &fingers[1]._lZForce, &fingers[2]._lZForce, &fingers[3]._lZForce, &fingers[4]._lZForce, &curHandLocation))
		{
			//check if the finger has been mapped
			if(curFingerCount==-1)
			{
				//if not, simply map it in sequence
				puppet->body["lLeg"]->linkFinger(&fingers[0]);				//this coulds be in play process once
				puppet->body["lHand"]->linkFinger(&fingers[1]);
				puppet->body["head"]->linkFinger(&fingers[2]);
				puppet->body["rHand"]->linkFinger(&fingers[3]);
				puppet->body["rLeg"]->linkFinger(&fingers[4]);
				newFingerCount=MAX_FINGERS;
			}
		}
		else
		{
			//if cant read from file, stop replaying
			curCondition=stop;
			//set default value for center point
			handCenter = -1;
			printf("Finish Playing\n");

			//move puppet back to center
			puppet->moveWheelsCenter();
			
			//terminate this process
			return;
		}

	}

	curFingerCount = newFingerCount;

	puppet->move(curHandLocation, curCondition, recordFile);		//send movement commands to servos
}

void Processor::print()
{
	//when playing file, puppet is not reading data from force pad
	if(curCondition==play)
	{
		printf("Playing...\n");
		for (long i = 0; i < MAX_FINGERS; ++i)
		{
			//coordinate value is not available, just show finger force value
			printf("Finger %d: f: %ld grams, ctrl: %s\n", i, fingers[i].getForce(), fingers[i].getPartName().c_str());
		}
		printf("Finger Center: (%d)\n", curHandLocation);
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
			printf("Finger %d: Coords(%4d, %4d), f: %ld grams, ctrl: %s (%ld/%ld)\n", i, fingers[i].getX(), fingers[i].getY(), fingers[i].getForce(), fingers[i].getPartName().c_str(), fingers[i].getTicks(), fingers[i].getPartTarget());
		}

		printf("Finger Center: %.2f,(%d)\n", handCenter, getHandCenterPosition());
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
	recordFile =fopen("C:\\PuppetFile\\puppetRecord.txt","w");
	printf("START RECORD\n");
}

//setup for play
void Processor::startPlay()
{
	curCondition=play;
	playFile =fopen("C:\\PuppetFile\\puppetRecord.txt","r");
	printf("START PLAY\n");
}

//terminate all record and play process
void Processor::stopRecordPlay()
{
	if(curCondition==record)
	{
		fclose(recordFile);
		printf("STOP RECORD\n");
	}
	else if(curCondition==play)
	{
		fclose(playFile);
		printf("STOP PLAY\n");
	}

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