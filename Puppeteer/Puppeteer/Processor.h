#pragma once

#include "Definitions.h"
#include "Finger.h"
#include "Puppet.h"

typedef pair<long, Finger*> sortPair;

bool cmpByX(const sortPair& a, const sortPair& b);

class Processor
{
private:
	static const int THRESHOLD_LEFT_FIVE = 2900;
	static const int THRESHOLD_LEFT_FOUR = 3300;
	static const int THRESHOLD_RIGHT_FIVE = 3900;
	static const int THRESHOLD_RIGHT_FOUR = 3500;
	Finger fingers[MAX_FINGERS];
	int curFingerCount;
	ISynPacket* _packet;
	bool fingerMapRequired;
public:
	void createPacket(ISynDevice* device); 
	void processData(Puppet* puppet, ISynGroup* dataGroup);
	long getFingerAvg_x();
	int detectCenterPosition();
	void print(void);	

	Processor(ISynDevice* device);
	~Processor(void);
};

