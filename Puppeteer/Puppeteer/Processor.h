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
	void disconnectFingers();
	double handCenter;
	handLocation curHandLocation;

public:
	FILE* recordFile;
	FILE* playFile;
	void createPacket(ISynDevice* device);
	void processData(Puppet* puppet, ISynGroup* dataGroup);
	void print(void);
	double getFingerAvg_x();
	handLocation getHandCenterPosition(Puppet* puppet);
	void startRecord(char* filename);
	void startPlay(char* filename);
	void stopRecordPlay();
	recordPlayMode curMode;

	Processor(ISynDevice* device);
	~Processor(void);
};

