/*
Project: UW RemOs3 Camera-wiper Control Software
Author: Augusto Neubauer
email: augusto.neubauer@gmail.com / augusto.neubauer@awide
----
Create date: 21.06.2021
Last Modified: 03.05.2022
Modified by: NEUBAUER Augusto
----
Purpose:
----
To control the UW wiper for the RemOs3 cameras through a Command Line Interface Software
Controlled aspects:
1st: Wipers displacement: two movement patterns are possible, select "ROT" mode for full shaft rotations (360� each) or
     select "DEG" mode for angular displacement under 360 degrees
2nd: Wipers spinning direction: Clockwise (+) and Counterclockwise (-)
3rd: Wipers spinning speed: Rotations per minute [RPM]
------
System Configuration:

    Motor:                  Maxon ECX SP22M BL KL A STD 24V
    Positioning Controller: EPOS4 Module 50/5
    Gearhead:               GPX22HP 231:1
    Mechanicalgearratio:  1:1
------

*/


#include <iostream>
#include <string.h>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <chrono>
#include <thread>
#include "Definitions.h"

using namespace std;

#ifndef MMC_SUCCESS
#define MMC_SUCCESS 0
#endif

#ifndef MMC_FAILED
#define MMC_FAILED 1
#endif 

typedef void* HANDLE;
typedef int BOOL;
typedef unsigned long DWORD;


void* keyHandle = 0;
DWORD NodeId = 1;
DWORD baudrate = 0;
DWORD timeout = 0;
DWORD errorCode = 0;//Variable to store errors returned by EPOS functions
DWORD SizeName = 0;
int DialogMode = 0;

const string programName = "    UW RemOs3 camera wipers Control Software";

char* deviceName;
char* protocolStackName;
char* interfaceName;
char* portName;

//Declaring the functions used in the code

void LogError();
void LogInfo(string message);
void SeparatorLine();
void PrintHeader();
void SetDefaultParameters(string functionName, int lResult, DWORD errorCode);
void PrintSettings();
void PrintUsage();
int OpenDevice();
int Setup(DWORD& pErrorCode);
bool ReadState(DWORD& pErrorCode, long& pPositionIs);
bool SensorType(DWORD& pErrorCode);
bool DrivePosition(DWORD& pErrorCode, long shaft_speed);
int CloseDevice(DWORD& pErrorCode);

void LogError(string functionName, int lResult, DWORD errorCode)
{
    cerr << programName << ": " << functionName << " Failed (result = " << lResult << ", errorCode = 0x" << std::hex << errorCode << ")" << endl;
}

void LogInfo(string message)
{
    cout << message << endl;
}

void SeparatorLine()
{
    const int lineLength = 65;
    for (int i = 0; i < lineLength; i++)
    {
        cout << "-";
    }
    cout << endl;
}

void PrintHeader()
{
    SeparatorLine();

    LogInfo("    UW RemOs3 camera wipers Control Software");

    SeparatorLine();
}


void SetDefaultParameters()
{
    NodeId;
    deviceName = (char*)"EPOS4";
    protocolStackName = (char*)"MAXON SERIAL V2";
    interfaceName = (char*)"USB";
    portName = (char*)"USB0";
    baudrate = 1000000;
}

void PrintSettings()
{
    stringstream msg;

    msg << "    Default settings:" << endl;
    msg << "    Node id             = " << NodeId << endl;
    msg << "    Device name         = " << deviceName << endl;
    msg << "    Protocal stack name = " << protocolStackName << endl;
    msg << "    Interface name      = " << interfaceName << endl;
    msg << "    Port name           = " << portName << endl;
    msg << "    Baudrate            = " << baudrate << endl;

    LogInfo(msg.str());

    SeparatorLine();
}

void PrintUsage()
{
    cout << "   Usage instructions: " << endl;
    cout << "  *1st input: select the displacement mode: 'ROT' to move full rotations or 'DEG' for angular displacement up to 720 degrees " << endl;
    cout << "  *2nd input: Wipers displacement: if 'ROT' is selected, insert a value between -20 and 20 rotations. Else if 'DEG' is select, insert a value beteween -720 and 720 degrees  " << endl;
    cout << "   A negative value for displacement will set a  Clockwise (-) spinning direction. A positive value will set a Counterclockwise movement" << endl;
    cout << "  *3rd input: Wipers spinning speed: Rotations per minute [RPM]. Please, insert a value between Zero and 10 RPM " << endl;
    cout << "\n" << endl;
    cout << "   e.g. to move the wipers 3 full turns at 4 RPM clockwise: wiper_control ROT -3 4" << endl;
    cout << "   e.g. to move the wipers 45 degrees at 5 RPM counterclockwise: wiper_control DEG 45 5" << endl;
    SeparatorLine();
}


/********************************************************************
    OpenDevice: Starts the available device
*********************************************************************/
int OpenDevice()
{
    int lResult = MMC_FAILED;   //initially define the local result as error

    //VCS_OpenDevice recognizes available interfaces capable to operate with EPOS and opens the selected interface for communication
    keyHandle = VCS_OpenDevice(deviceName, protocolStackName, interfaceName, portName, &errorCode);

    if (keyHandle != 0 && errorCode == 0)
    {
        DWORD lBaudrate = 0;
        DWORD lTimeout = 0;

        if (VCS_GetProtocolStackSettings(keyHandle, &lBaudrate, &lTimeout, &errorCode) != 0)
        {
            if (VCS_SetProtocolStackSettings(keyHandle, baudrate, lTimeout, &errorCode) != 0)
            {
                if (baudrate == (DWORD)lBaudrate)
                {
                    lResult = MMC_SUCCESS;  // if the code runs up to here we define as success
                }
            }
        }
    }
    else
    {
        keyHandle = 0;
    }

    return lResult;
}


/********************************************************************
    Setup: Setup to ensure the device is ready for use
*********************************************************************/
int Setup(DWORD& pErrorCode)
{
    int lResult = MMC_SUCCESS;
    BOOL oIsFault = 0;  // 0 device is not in fault state, 1 device is in fault state

    if (VCS_GetFaultState(keyHandle, NodeId, &oIsFault, &pErrorCode) == 0)
    {
        LogError("VCS_GetFaultState", lResult, pErrorCode);
        lResult = MMC_FAILED;
    }

    if (lResult == 0)
    {
        if (oIsFault) // = if(oIsFault !=0) | Thus it means oIsFault is different of Zero (therefore there is fault)
        {
            stringstream msg;
            msg << "clear fault, node = '" << NodeId << "'";
            LogInfo(msg.str());

            if (VCS_ClearFault(keyHandle, NodeId, &pErrorCode) == 0)
            {
                LogError("VCS_ClearFault", lResult, pErrorCode);
                lResult = MMC_FAILED;
            }
        }

        if (lResult == 0)
        {
            BOOL oIsEnabled = 0;

            if (VCS_GetEnableState(keyHandle, NodeId, &oIsEnabled, &pErrorCode) == 0)
            {
                LogError("VCS_GetEnableState", lResult, pErrorCode);
                lResult = MMC_FAILED;
            }

            if (lResult == 0)
            {
                if (!oIsEnabled)
                {
                    if (VCS_SetEnableState(keyHandle, NodeId, &pErrorCode) == 0)
                    {
                        LogError("VCS_SetEnableState", lResult, pErrorCode);
                        lResult = MMC_FAILED;
                    }
                }
            }
        }
    }
    return lResult;
}



/********************************************************************
    Sensor Type: Return the type of sensor used for measurements
*********************************************************************/
bool SensorType(DWORD& pErrorCode)
{
    WORD sensorType;
    int lResult = MMC_SUCCESS;
    stringstream msg;

    VCS_GetSensorType(keyHandle, NodeId, &sensorType, &pErrorCode);
    if (VCS_GetSensorType(keyHandle, NodeId, &sensorType, &pErrorCode) == 0)
    {
        LogError("VCS_GetSensorType", lResult, pErrorCode);
        lResult = MMC_FAILED;
    }
    else
    {
        msg << "Sensor Type  = " << sensorType << endl;
        LogInfo(msg.str());
    }

    return lResult;
}

/********************************************************************
    DrivePosition: Selects operation mode and controls the movement
*********************************************************************/
bool DrivePosition(DWORD& pErrorCode, long new_targetPosition, long shaft_speed)
{
    int lResult = MMC_FAILED;

    shaft_speed;
    new_targetPosition;

    stringstream msg;

    bool absolute = false; // FALSE sets to a relative movement;  TRUE starts an absolute movement
    bool immediately = true; // TRUE stars movement immediatly; FALSE waits to end of last positioning


    DWORD profileVelocity = shaft_speed * 231 * (1);
    DWORD profileAcc = 10000;
    DWORD profileDec = 10000;
    long targetPosition = new_targetPosition;

    VCS_ActivateProfilePositionMode(keyHandle, NodeId, &pErrorCode);
    if (VCS_ActivateProfilePositionMode(keyHandle, NodeId, &pErrorCode) == 0)
    {
        LogError("VCS_ActivateProfilePositionMode", lResult, pErrorCode);
        lResult = MMC_FAILED;
    }

    VCS_SetPositionProfile(keyHandle, NodeId, profileVelocity, profileAcc, profileDec, &pErrorCode);
    if (VCS_SetPositionProfile(keyHandle, NodeId, profileVelocity, profileAcc, profileDec, &pErrorCode) == 0)
    {
        LogError("VCS_SetPositionProfile", lResult, pErrorCode);
        lResult = MMC_FAILED;
    }


    VCS_MoveToPosition(keyHandle, NodeId, targetPosition, absolute, immediately, &pErrorCode);
    if (VCS_MoveToPosition(keyHandle, NodeId, targetPosition, absolute, immediately, &pErrorCode) == 0)
    {
        LogError("VCS_MoveToPosition", lResult, pErrorCode);
        lResult = MMC_FAILED;
    }


    if (lResult == MMC_SUCCESS)
    {
        LogInfo("Halt position movement");

        if (VCS_HaltPositionMovement(keyHandle, NodeId, &pErrorCode) == 0)
        {
            LogError("VCS_HaltPositionMovement", lResult, pErrorCode);
            lResult = MMC_FAILED;
        }
    }

    // Section to count time needed to complete the displacement
    float time1;
    //time1 = ((float(targetPosition) / float(profileVelocity))*10*1000)+100; //time calculated (milliseconds) + 100 milliseconds  **insert a 'if' for negative target position (x1) as it is not possible to have negative time 
    int time;

    if (targetPosition < 0)
    {
        time1 = (((float(targetPosition) / float(profileVelocity)) * 10 * 1000) + 100) * (-1);
    }
    else
    {
        time1 = (((float(targetPosition) / float(profileVelocity)) * 10 * 1000) + 100);
    }

    time = time1;

    this_thread::sleep_for(chrono::milliseconds(time)); //Waits the system to reach the target position

    VCS_SetDisableState(keyHandle, NodeId, &pErrorCode); //Disable the motor to avoid humming noise
    if (VCS_SetDisableState(keyHandle, NodeId, &pErrorCode) == 0)
    {
        LogError("VCS_SetDisableState", lResult, pErrorCode);
        lResult = MMC_FAILED;
    }
    return lResult;
}


/********************************************************************
    CloseDevice: Shuts down the operation
*********************************************************************/
int CloseDevice(DWORD& pErrorCode)
{
    int lResult = MMC_FAILED;

    pErrorCode = 0;

    LogInfo("Close Device");

    if (VCS_CloseDevice(keyHandle, &pErrorCode) != 0 && pErrorCode == 0)
    {
        lResult = MMC_SUCCESS;
    }

    VCS_CloseDevice(keyHandle, &pErrorCode);
    VCS_CloseAllDevices(&pErrorCode);

    return lResult;
}



int main(int argc, char* argv[])
{
    int lResult = MMC_FAILED;
    DWORD pErrorCode = 0;

    long displacement;
    long shaft_speed;
    long new_targetPosition;

    long pPositionIs = 0;

    SetDefaultParameters();
    PrintHeader();
    PrintSettings();
    OpenDevice();
    Setup(pErrorCode);

    if (argc == 4)
    {
        displacement = stoul(argv[2]);
        shaft_speed = stoul(argv[3]);

        if (!strcmp(argv[1], "ROT"))
        {
            new_targetPosition = displacement * 231 * 6 * (1);

            if (displacement < -20 || displacement > 20)
            {
                cout << "   Number of rotations out of range, please insert a number between -20 and 20" << endl;
                CloseDevice(pErrorCode);
            }
        }
        else if (!strcmp(argv[1], "DEG"))
        {
            new_targetPosition = displacement * ((231 * 6 * 1) / 360);

            if (displacement < -720 || displacement > 720)
            {
                cout << "   Degree displacement out of range, please insert a number between -720 and 720" << endl;
                CloseDevice(pErrorCode);
            }
        }

        if (shaft_speed < 0 || shaft_speed > 10)
        {
            cout << "   Shaft Speed out of range, please insert a value between Zero and 10 [RPM]" << endl;
            CloseDevice(pErrorCode);
        }
        else
        {
            DrivePosition(pErrorCode, new_targetPosition, shaft_speed);
        }
    }
    else
    {
        cout << "   Incorrect input arguments" << endl;
        PrintUsage();
    }

    if ((lResult = OpenDevice()) != MMC_SUCCESS)
    {
        LogError("OpenDevice", lResult, errorCode);
        return lResult;
    }

    if ((lResult = Setup(pErrorCode)) != MMC_SUCCESS)
    {
        LogError("Setup", lResult, errorCode);
        return lResult;
    }

    if ((lResult = CloseDevice(pErrorCode)) != MMC_SUCCESS)
    {
        LogError("CloseDevice", lResult, pErrorCode);
        return lResult;
    }
    return lResult;
}
