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

	ISynGroup* pGroupPeak = NULL;
    pDevice->CreateGroup(&pGroupPeak);
    
    // Stop the ForcePad reporting to the operating system
    pDevice->Acquire(SF_AcquireAll);

	/********************* START ****************************/
    printf("Welcome to the Puppeteer JedEye app!\n");

    LONG lFingerCount = 0;

	Puppet* puppet = new Puppet();
	
	Processor processor(pDevice);

	//printf("\nPlace hand on ForcePad to start!\n");

	SYSTEMTIME st1,st2;  //Time checking
	FILETIME ft1,ft2;

	GetSystemTime(&st1);
	SystemTimeToFileTime(&st1,&ft1);
	ULARGE_INTEGER uli;
	uli.LowPart = ft1.dwLowDateTime;
	uli.HighPart = ft1.dwHighDateTime;
	ULONGLONG systemTimeIn_ms1( uli.QuadPart/10000 );

	int timeDiff=0;

	//get command: r(read), p(play), s(stop) and q(quit)
	char filename [256] = {'\0'};
	char command=-1;

	do	
	{

		do	//Menu
		{
			

			printf("\nPress 'o' to operate the puppet\n");
			printf("Press 'r' to record a movement segment\n");
			printf("Press 'p' to play a movement segment\n");
			printf("Press 'q' to quit\n");
			

			command=_getch();
			switch (command)
			{
				case 'o':
					printf("\nPlace hand on ForcePad to start operating!\n");
					processor.curMode = operate;
					break;
				case 'r':
					printf("\nEnter a filename for your recording:\n");
					do
					{
						gets_s(filename);
					}
					while (filename[0] == '\0');

					printf("\nNow recording! Press 's' to stop your recording\n");
					processor.startRecord(filename);

					break;
				case 'p':
					printf("\nEnter a filename to play:\n");
					do
					{
						gets_s(filename);
						printf("%s",filename);
					}
					while (filename[0] == '\0');

					processor.startPlay(filename);
					
					break;
				case 'q':
					break;
				default:
					command = -1;
			}
		}
		while (command == -1);

		do	//User selected an option
		{
			
		    // Wait until the event signals that data is ready
		    WaitForSingleObject(hEvent, MAX_INPUT_WAIT);

		    // Load data into the ISynGroup instance
		    do
			{

				pGroup = NULL;
				while (pDevice->LoadGroup(pGroupPeak) != SYNE_FAIL)
					pGroup = pGroupPeak;

				GetSystemTime(&st2);
				SystemTimeToFileTime(&st2,&ft2);
 				uli.LowPart = ft2.dwLowDateTime;
 				uli.HighPart = ft2.dwHighDateTime;
 				ULONGLONG systemTimeIn_ms2( uli.QuadPart/10000 );

				timeDiff = (int) (systemTimeIn_ms2-systemTimeIn_ms1);

				if ((timeDiff >= 0) && (timeDiff < CYCLE_TIME))
					Sleep(CYCLE_TIME-timeDiff);		//keep constant cycle time of x ms

				processor.processData(puppet, pGroup);	//Read data and perform logic
				processor.print();	//give status

				GetSystemTime(&st1);
				SystemTimeToFileTime(&st1,&ft1);
 				uli.LowPart = ft1.dwLowDateTime;
 				uli.HighPart = ft1.dwHighDateTime;
 				systemTimeIn_ms1 = uli.QuadPart/10000 ;

				if(_kbhit())
				{
					command=_getch();
					printf("Received: %c\n",command);
				}
			}	//as long as no exit command was given, repeat
			while (pGroup && ((command != 'q') && (command != 'm') && (processor.curMode != stop)));
			
		}
		while ((command != 'q') && (command != 'm') && (processor.curMode != stop));	//Until q(quit) key is pressed

		for (int i=0; i<LEG_MAX_TICKS; i++)		//Stop sequence
		{
			Sleep(CYCLE_TIME);
			processor.processData(puppet, NULL);	//send empty packets
			processor.print();	//give status
		}

		processor.stopRecordPlay();

    }
    while ((command != 'q'));	//Until key is pressed
	
	/****************** Shutdown sequence **********************/

	delete puppet;

    // Don't signal any more data
    pDevice->SetEventNotification(NULL);
    
    // Release the COM objects we have created
    if (pGroup) 
		pGroup->Release();
    pDevice->Release();
    pAPI->Release();

    // Release the wait object
    CloseHandle(hEvent);

    return EXIT_SUCCESS;
    
}