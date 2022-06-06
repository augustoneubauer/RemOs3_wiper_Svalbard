**********************************************************************************************************************************************************************************
Project: UW Webcam Scheibenwischer Control Software
Author: Augusto Neubauer
email: augusto.neubauer@gmail.com

Create date:  21.06.2021
Last Modified: 22.06.2021
Modified by: NEUBAUER Augusto
**********************************************************************************************************************************************************************************
Installation instructions:

If the "Definition File" is not included, please:
> Download the library files at https://www.maxongroup.com/medias/sys_master/root/8839866548254/EPOS-Windows-DLL-En.zip
> Unzip the downloaded file and access \Microsoft Visual C++\Example VC++ and move the files EposCmd.dll to C:\Windows\SysWOW64 and EposCmd64.dll to C:\Windows\System32
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

UW Webcam Scheibenwischer Control Software System Configuration:

    Motor:                  Maxon ECX SP22M BL KL A STD 24V
    Positioning Controller: EPOS4 Module 50/5
    Gearhead:               GPX22HP 231:1
    Mechanical gear ratio:  100/25

Purpose:
To control the UW Webcam wipers through a Command Line Interface Software.
Controlled aspects:
1st: Wipers displacement: two movement patterns are possible, select "ROT" mode for full shaft rotations (360� each) or 
     select "DEG" mode for angular displacement under 360 degrees
2nd: Wipers spinning direction: Clockwise (+) and Counterclockwise (-)
3rd: Wipers spinning speed: Rotations per minute [RPM]
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Usage Instructions:

1st argument: Mode selection
> select the displacement mode: 'ROT' to move full rotations or 'DEG' for angular displacement under 360 degrees

2nd argument: Wipers displacement
> if 'ROT' is selected, insert a value between -20 and 20 rotations;
> if 'DEG' is select, insert a value beteween -360 and 360 degrees;
NOTE: A negative value for displacement will set a  Clockwise (-) spinning direction. A positive value will set a Counterclockwise movement

3rd argument: Wipers spinning speed
> Rotations per minute [RPM]. Please, insert a value between Zero and 10 RPM

e.g. to move the wipers 3 full turns at 4 RPM clockwise:      wiper_control ROT -3 4
e.g. to move the wipers 45 degrees at 5 RPM counterclockwise: wiper_control DEG 45 5

-------!!! Still under development !!!--------

1st argument others mode selection: "SETHOME" , "GOHOME" , SENSOR"  (only one argument functions)

> if "SETHOME" is selected, the current position is defined as Home Position;
> if "GOHOME" is selected, the shaft moves to reach the Home Position;
> if "SENSOR" is selected, the program answers with the system sensor type (currently: 3 (Hall sensors)
