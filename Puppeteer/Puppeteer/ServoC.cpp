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
	const char *err;
	int motorCount = 7;	//4 on servo, 3 on servo2

	//Declare an advanced servo handle
	servo = 0;	//177737
	servo2 = 0;	//177698

	//create the advanced servo object
	CPhidgetAdvancedServo_create(&servo);
	CPhidgetAdvancedServo_create(&servo2);

	//Set the handlers to be run when the device is plugged in or opened from software, unplugged or closed from software, or generates an error.
	CPhidget_set_OnAttach_Handler((CPhidgetHandle)servo, AttachHandler, NULL);
	CPhidget_set_OnDetach_Handler((CPhidgetHandle)servo, DetachHandler, NULL);
	CPhidget_set_OnError_Handler((CPhidgetHandle)servo, ErrorHandler, NULL);

	CPhidget_set_OnAttach_Handler((CPhidgetHandle)servo2, AttachHandler, NULL);
	CPhidget_set_OnDetach_Handler((CPhidgetHandle)servo2, DetachHandler, NULL);
	CPhidget_set_OnError_Handler((CPhidgetHandle)servo2, ErrorHandler, NULL);

	//Registers a callback that will run when the motor position is changed.
	//Requires the handle for the Phidget, the function that will be called, and an arbitrary pointer that will be supplied to the callback function (may be NULL).
	CPhidgetAdvancedServo_set_OnPositionChange_Handler(servo, PositionChangeHandler, NULL);
	CPhidgetAdvancedServo_set_OnPositionChange_Handler(servo2, PositionChangeHandler, NULL);

	//open the device for connections
	CPhidget_open((CPhidgetHandle)servo, -1);
	CPhidget_open((CPhidgetHandle)servo2, -1);

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
	
	printf("Waiting for Phidget to be attached....");
	if((result = CPhidget_waitForAttachment((CPhidgetHandle)servo2, 10000)))
	{
		CPhidget_getErrorDescription(result, &err);
		printf("Problem waiting for attachment: %s\n", err);
		return;
	}

	//Display the properties of the attached device
	display_properties(servo2);
	
	//read event data
	printf("Reading.....\n");

	/*
	//Set up some initial acceleration and velocity values
	CPhidgetAdvancedServo_getAccelerationMax(servo, 0, &maxAccel);
	CPhidgetAdvancedServo_setAcceleration(servo, 0, maxAccel);
	CPhidgetAdvancedServo_getVelocityMax(servo, 0, &maxVel);
	CPhidgetAdvancedServo_setVelocityLimit(servo, 0, maxVel/2);
	
	//display current motor position
	if(CPhidgetAdvancedServo_getPosition(servo, 0, &curr_pos) == EPHIDGET_OK)
		printf("Motor: 0 > Current Position: %f\n", curr_pos);
	*/

	Sleep(1000);
	
	double acceleration;
	double current;
	int error;
	
	for (int i = 0; i < 4; i++)
	{
		CPhidgetAdvancedServo_setServoType(servo, i, PHIDGET_SERVO_SPRINGRC_SM_S4315R);
		CPhidgetAdvancedServo_setPositionMin(servo, i, MOVE_C_CLOCKWISE_LOCATION);
		CPhidgetAdvancedServo_setPositionMax(servo, i, MOVE_CLOCKWISE_LOCATION);

		CPhidgetAdvancedServo_getAccelerationMax(servo, i, &acceleration);
		CPhidgetAdvancedServo_setAcceleration(servo, i, acceleration);

		printf("Move to position 50 (stop) and engage.\n");
		CPhidgetAdvancedServo_setPosition (servo, i, STOP_LOCATION);

		CPhidgetAdvancedServo_setEngaged(servo, i, 1);
		
		error = CPhidgetAdvancedServo_getCurrent(servo, i, &current);

		printf("error: %d , accel: %f, current: [%f]\n", error, acceleration, current);

		
	}
	printf("\nServo2:\n");

	for (int i = 0; i < 3; i++)
	{
		CPhidgetAdvancedServo_setServoType(servo2, i, PHIDGET_SERVO_SPRINGRC_SM_S4315R);
		CPhidgetAdvancedServo_setPositionMin(servo2, i, MOVE_C_CLOCKWISE_LOCATION);
		CPhidgetAdvancedServo_setPositionMax(servo2, i, MOVE_CLOCKWISE_LOCATION);

		CPhidgetAdvancedServo_getAccelerationMax(servo2, i, &acceleration);
		CPhidgetAdvancedServo_setAcceleration(servo2, i, acceleration);

		printf("Move to position 50 (stop) and engage.\n");
		CPhidgetAdvancedServo_setPosition (servo2, i, STOP_LOCATION);

		CPhidgetAdvancedServo_setEngaged(servo2, i, 1);
		
		error = CPhidgetAdvancedServo_getCurrent(servo2, i, &current);

		printf("error: %d , accel: %f, current: [%f]\n", error, acceleration, current);

		
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

	if (motorId < 4)	//These motors will need servo1
		CPhidgetAdvancedServo_setPosition(servo, motorId, targetPosition);	//Set relevant motor's movement
	else	//motors of servo2 controller
	{
		motorId -= 4;	//the 4-6 motor id's map to 0-2 on servo2
		CPhidgetAdvancedServo_setPosition(servo2, motorId, targetPosition);	//Set relevant motor's movement
	}
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
