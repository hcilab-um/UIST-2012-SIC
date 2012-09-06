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
public:
	void createPacket(ISynDevice* device);
	void processData(Puppet* puppet, ISynGroup* dataGroup);
	void print(void);	

	Processor(void);
	~Processor(void);
};

