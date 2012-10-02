#include "stdafx.h"
#include <iostream>
#include <conio.h>

#include <SynKit.h>
#include "Processor.h"
#include "Puppet.h"
#include "ServoC.h"
#include <fstream>
#include "Definitions.h"
using namespace std;

#pragma comment(lib, "SynCOM.lib") // For access point SynCreateAPI

int main(int argc, char** argv)
{
	/********************* INIT sequence ****************************/

    // Wait object will indicate when new data is available
    HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    
    // Entry point to Synaptics API
    ISynAPI* pAPI = NULL;
    SynCreateAPI(&pAPI);
    
    // Find the first USB TouchPad device connected to the system
    LONG lHandle = -1;
    if (pAPI->FindDevice(SE_ConnectionUSB, SE_DeviceTouchPad, &lHandle) == SYNE_NOTFOUND)
    {
        printf("ForcePad not found\n");
        return EXIT_FAILURE;
    }
    
    // Create an interface to the ForcePad
    ISynDevice* pDevice = NULL;
    pAPI->CreateDevice(lHandle, &pDevice);

    // Tell the device to signal hEvent when data is ready
    pDevice->SetEventNotification(hEvent);

    // Enable multi-finger touch and grouped reporting
    pDevice->SetProperty(SP_IsMultiFingerReportEnabled, 1);
    pDevice->SetProperty(SP_IsGroupReportEnabled, 1);

    // Get the maximum number of fingers the device will report
    LONG lNumMaxReportedFingers;
    pDevice->GetProperty(SP_NumMaxReportedFingers, &lNumMaxReportedFingers);

    // Create an ISynGroup instance to receive per-frame data
    ISynGroup* pGroup = NULL;
    pDevice->CreateGroup(&pGroup);
    
    // Stop the ForcePad reporting to the operating system
    pDevice->Acquire(SF_AcquireAll);

	/********************* START ****************************/
    printf("Welcome to the Puppeteer JedEye app!\n");

    LONG lFingerCount = 0;

	Puppet* puppet = new Puppet();
	
	Processor processor(pDevice);

	printf("\nPlace hand on ForcePad to start!\n");
	
	//char variable to receive r(read), p(play), s(stop) and q(quit) command
	char command=-1;
	bool toRecord=false;
	bool toPlay=false;
    do
    {
        // Wait until the event signals that data is ready
        WaitForSingleObject(hEvent, MAX_INPUT_WAIT);

        // Load data into the ISynGroup instance, repeating until there is no more data
        while (pDevice->LoadGroup(pGroup) != SYNE_FAIL)
		{
			//playing data
			if(toPlay)
			{
				printf("Playing...\n");
			}
			else
			{
				processor.processData(puppet, pGroup);	//Read data and perform logic
				processor.print();	//give status
			}
        }

		//when a key is pressed, check if it is r(read), p(play), q(quit) command
		if(_kbhit())
		{
			command=_getch();
			printf("%c\n",command);
			//if user press r(record), program should only record data
			if(command == 'r')
			{
				processor.startRecord();
			}
			//if user press p(play), program should only play data
			if(command == 'p')
			{
				//processor.startPlay();
			}
			//if user press s(stop), program should not play or record
			if(command == 's')
			{
				processor.stopRecordPlay();
			}
		}
    }while (command!='q');	//Until q(quit) key is pressed
	
	/****************** Shutdown sequence **********************/

	delete puppet;

    // Don't signal any more data
    pDevice->SetEventNotification(NULL);
    
    // Release the COM objects we have created
    pGroup->Release();
    pDevice->Release();
    pAPI->Release();

    // Release the wait object
    CloseHandle(hEvent);

    return EXIT_SUCCESS;
    
}