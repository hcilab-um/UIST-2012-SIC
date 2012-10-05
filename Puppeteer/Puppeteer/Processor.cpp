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
	handCenter = -1;
	curHandLocation = eCenter;
}

void Processor::createPacket(ISynDevice* device)
{
	// Create an ISynPacket instance to receive per-touch data
	device->CreatePacket(&_packet);
}

void Processor::processData(Puppet* puppet, ISynGroup* dataGroup)
{
	int newFingerCount = 0;
	curHandLocation = eCenter;

	if (curMode == play)	//Play from file
	{

		//check if the file is being correctly read(if it gets to end of file)
		if(6==fscanf_s(playFile, "%ld,%ld,%ld,%ld,%ld,%d\n", &fingers[0]._lZForce, &fingers[1]._lZForce, &fingers[2]._lZForce, 
			&fingers[3]._lZForce, &fingers[4]._lZForce, &curHandLocation))
		{
			//check if the finger hasn't been mapped already
			if(fingerMapRequired == true)
			{
				//if not, simply map it in sequence
				puppet->body["lLeg"]->linkFinger(&fingers[0]);
				puppet->body["lHand"]->linkFinger(&fingers[1]);
				puppet->body["head"]->linkFinger(&fingers[2]);
				puppet->body["rHand"]->linkFinger(&fingers[3]);
				puppet->body["rLeg"]->linkFinger(&fingers[4]);
				fingerMapRequired = false;
				newFingerCount=MAX_FINGERS;
			}
		}
		else
		{
			//if cant read from file, stop replaying
			stopRecordPlay();
			fingerMapRequired = true;
			disconnectFingers();

			//set default value for center point
			handCenter = -1;
			printf("Done playing!\n");

			//move puppet back to center
			puppet->moveWheelsCenter();
			
			//terminate this process
			return;
		}

	}
	else if(curMode != play)	//Data found
	{
		if (!dataGroup)		//No data - Remove all fingers 
		{
			disconnectFingers();
		}
		else
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
					fingers[i].remove();
				}
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

		curHandLocation = getHandCenterPosition(puppet);
	}

	puppet->move(curHandLocation, curMode, recordFile);		//send movement commands to servos
}

void Processor::print()
{
	
	//when playing file, puppet is not reading data from force pad
	if(curMode==play)
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
		if(curMode==record)
		{
			//show that program is recording data
			printf("Recording...\n");
		}
		for (long i = 0; i < MAX_FINGERS; ++i)
		{
			printf("Finger %d: Coords(%4d, %4d), f: %ld grams, ctrl: %s (%ld/%ld)\n", i, fingers[i].getX(), fingers[i].getY(), fingers[i].getForce(), fingers[i].getPartName().c_str(), fingers[i].getTicks(), fingers[i].getPartTarget());
		}

		printf("Finger Center: %.2f,(%d)\n", handCenter, curHandLocation);
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

handLocation Processor::getHandCenterPosition(Puppet* puppet)
{
	/*
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
	*/

	if ((curFingerCount < MAX_FINGERS-1) || (fingerMapRequired == true))
		return eCenter;	//default

	int leftFinger, rightFinger;

	leftFinger = puppet->body["lLeg"]->finger->getX();
	rightFinger = puppet->body["rLeg"]->finger->getX();

	if (leftFinger == -1)	//Left leg finger was lifted
		leftFinger =  puppet->body["lHand"]->finger->getX();

	if (rightFinger == -1)  //right leg finger was lifted
		rightFinger =  puppet->body["rHand"]->finger->getX();

	if ((leftFinger < FIVE_FINGERS_LEFT) &&
		(rightFinger < FIVE_FINGERS_RIGHT))
		return eLeft;
	else if ((leftFinger > FIVE_FINGERS_LEFT) &&
		(rightFinger > FIVE_FINGERS_RIGHT))
		return eRight;

	//Center position
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


void Processor::disconnectFingers()
{
	for (int i=0; i < MAX_FINGERS; ++i)
	{
		fingers[i].updateControl(NULL);	//clears all fingers
	}
}


//setup for record, get record file, set condition as record
void Processor::startRecord(char* filename)
{
	curMode=record;
	recordFile = fopen(filename,"w");
	if (recordFile)
		printf("RECORDING STARTED\n");
	else
	{
		printf("Error opening file _%s_!\n", filename);
		curMode = stop;
	}

}

//setup for play
void Processor::startPlay(char* filename)
{
	curMode=play;
	playFile = fopen(filename,"r");
	if (playFile)
		printf("PLAYING STARTED\n");
	else
	{
		printf("Error opening file _%s_!\n", filename);
		curMode = stop;
	}
}

//terminate all record and play process
void Processor::stopRecordPlay()
{
	if(curMode == record)
	{
		fclose(recordFile);
		printf("RECORDING STOPPED\n");
	}
	else if(curMode == play)
	{
		fclose(playFile);
		printf("PLAYING STOPPED\n");
	}

	curMode = stop;
}

Processor::~Processor(void)
{
}
