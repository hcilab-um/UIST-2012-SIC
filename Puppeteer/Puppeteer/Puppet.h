#pragma once
#include "PuppetPart.h"
#include <string>
#include <map>

using namespace std;

class Puppet
{
public:
	map<string, PuppetPart*> body;
	Puppet(void);
	void move();
	~Puppet(void);
};

