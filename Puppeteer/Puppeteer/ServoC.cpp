#include "StdAfx.h"
#include "ServoC.h"
#include <windows.h>

//Servo Controller class

int CCONV AttachHandler(CPhidgetHandle ADVSERVO, void *userptr)
{
	int serialNo;
	const char *name;

	CPhidget_getDeviceName (ADVSERVO, &name);
	CPhidget_getSerialNumber(ADVSERVO, &serialNo);
	printf("%s %10d attached!\n", name, serialNo);

	return 0;
}

int CCONV DetachHandler(CPhidgetHandle ADVSERVO, void *userptr)
{
	int serialNo;
	const char *name;

	CPhidget_getDeviceName (ADVSERVO, &name);
	CPhidget_getSerialNumber(ADVSERVO, &serialNo);
	printf("%s %10d detached!\n", name, serialNo);

	return 0;
}

int CCONV ErrorHandler(CPhidgetHandle ADVSERVO, void *userptr, int ErrorCode, const char *Description)
{
	printf("Error handled. %d - %s\n", ErrorCode, Description);
	return 0;
}

int CCONV PositionChangeHandler(CPhidgetAdvancedServoHandle ADVSERVO, void *usrptr, int Index, double Value)
{
	//printf("Motor: %d > Current Position: %f\n", Index, Value);
	return 0;
}

//Display the properties of the attached phidget to the screen.  We will be displaying the name, serial number and version of the attached device.
int display_properties(CPhidgetAdvancedServoHandle phid)
{
	int serialNo, version, numMotors;
	const char* ptr;

	CPhidget_getDeviceType((CPhidgetHandle)phid, &ptr);
	CPhidget_getSerialNumber((CPhidgetHandle)phid, &serialNo);
	CPhidget_getDeviceVersion((CPhidgetHandle)phid, &version);

	CPhidgetAdvancedServo_getMotorCount (phid, &numMotors);

	printf("%s\n", ptr);
	printf("Serial Number: %10d\nVersion: %8d\n# Motors: %d\n", serialNo, version, numMotors);

	return 0;
}

ServoC::ServoC(void)
{
	int result;
	double curr_pos;
	const char *err;
	int motorCount;

	//Declare an advanced servo handle
	servo = 0;

	//create the advanced servo object
	CPhidgetAdvancedServo_create(&servo);

	//Set the handlers to be run when the device is plugged in or opened from software, unplugged or closed from software, or generates an error.
	CPhidget_set_OnAttach_Handler((CPhidgetHandle)servo, AttachHandler, NULL);
	CPhidget_set_OnDetach_Handler((CPhidgetHandle)servo, DetachHandler, NULL);
	CPhidget_set_OnError_Handler((CPhidgetHandle)servo, ErrorHandler, NULL);

	//Registers a callback that will run when the motor position is changed.
	//Requires the handle for the Phidget, the function that will be called, and an arbitrary pointer that will be supplied to the callback function (may be NULL).
	CPhidgetAdvancedServo_set_OnPositionChange_Handler(servo, PositionChangeHandler, NULL);

	//open the device for connections
	CPhidget_open((CPhidgetHandle)servo, -1);

	//get the program to wait for an advanced servo device to be attached
	printf("Waiting for Phidget to be attached....");
	if((result = CPhidget_waitForAttachment((CPhidgetHandle)servo, 10000)))
	{
		CPhidget_getErrorDescription(result, &err);
		printf("Problem waiting for attachment: %s\n", err);
		return;
	}

	//Display the properties of the attached device
	display_properties(servo);
	CPhidgetAdvancedServo_getMotorCount(servo, &motorCount);

	//read event data
	printf("Reading.....\n");

	/*
	//Set up some initial acceleration and velocity values
	CPhidgetAdvancedServo_getAccelerationMax(servo, 0, &maxAccel);
	CPhidgetAdvancedServo_setAcceleration(servo, 0, maxAccel);
	CPhidgetAdvancedServo_getVelocityMax(servo, 0, &maxVel);
	CPhidgetAdvancedServo_setVelocityLimit(servo, 0, maxVel/2);
	*/
	//display current motor position
	if(CPhidgetAdvancedServo_getPosition(servo, 0, &curr_pos) == EPHIDGET_OK)
		printf("Motor: 0 > Current Position: %f\n", curr_pos);

	Sleep(1000);
	
	double test;
	int error;
	
	for (int i = 0; i < motorCount; i++)
	{
		CPhidgetAdvancedServo_setServoType(servo, i, PHIDGET_SERVO_SPRINGRC_SM_S4315R);
		CPhidgetAdvancedServo_setPositionMin(servo, i, MOVE_C_CLOCKWISE_LOCATION);
		CPhidgetAdvancedServo_setPositionMax(servo, i, MOVE_CLOCKWISE_LOCATION);

		printf("Move to position 50 (stop) and engage.\n");
		CPhidgetAdvancedServo_setPosition (servo, i, STOP_LOCATION);
		error = CPhidgetAdvancedServo_getCurrent(servo, i, &test);
		CPhidgetAdvancedServo_setEngaged(servo, i, 1);
		printf("error: %d , Pos: %f\n", error, test);

	}
	
}

void ServoC::setMovement(servoMovement movement, int motorId)
{
	double targetPosition;	//Used to control movement direction and speed, 50 stops, 0 C_Clockwise, 100 Clockwise
	
	switch (movement)
	{
		case eC_Clockwise:
			targetPosition = MOVE_C_CLOCKWISE_LOCATION;
			break;
		case eStopped:
			targetPosition = STOP_LOCATION;
			break;
		case eClockwise:
			targetPosition = MOVE_CLOCKWISE_LOCATION;
			break;
	}

	CPhidgetAdvancedServo_setPosition(servo, motorId, targetPosition);	//Set relevant motor's movement
}


ServoC::~ServoC(void)
{
	//Disengage
	printf("Disengage Servo\n");

	int motorCount;
	CPhidgetAdvancedServo_getMotorCount(servo, &motorCount);

	for (int i = 0; i < motorCount; i++)
	{
		CPhidgetAdvancedServo_setEngaged(servo, i, 0);
	}

	//since user input has been read, this is a signal to terminate the program so we will close the phidget and delete the object we created
	printf("Closing...\n");
	CPhidget_close((CPhidgetHandle)servo);
	CPhidget_delete((CPhidgetHandle)servo);
}
