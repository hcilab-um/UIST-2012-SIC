#pragma once

#include "Definitions.h"
#include "Finger.h"
#include "Puppet.h"

typedef pair<long, Finger*> sortPair;

bool cmpByX(const sortPair& a, const sortPair& b);

class Processor
{
private:
	Finger fingers[MAX_FINGERS];
	int curFingerCount;
	ISynPacket* _packet;
	bool fingerMapRequired;
public:
	void createPacket(ISynDevice* device); 
	void processData(Puppet* puppet, ISynGroup* dataGroup);
	long getFingerAvg_x();
	long getFingerAvg_y();
	void print(void);	

	Processor(ISynDevice* device);
	~Processor(void);
};

