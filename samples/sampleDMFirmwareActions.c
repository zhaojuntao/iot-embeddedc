/*******************************************************************************
 * Copyright (c) 2016 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    HariPrasada Reddy P - initial implementation and API implementation
 *******************************************************************************/

#include <stdio.h>
#include <signal.h>
//#include "iotfclient.h"
#include "devicemanagementclient.h"
#include <time.h>

volatile int interruption = 0;

// Handle signal interrupt
void signalHandler(int signo) {
	printf("SigINT received.\n");
	interruption = 1;
}

void myCallback (char* commandName, char* format, void* payload)
{
	printf("------------------------------------\n" );
	printf("The command received :: %s\n", commandName);
	printf("format : %s\n", format);
	printf("Payload is : %s\n", (char *)payload);

	printf("------------------------------------\n" );
}

void managedCallBack (char* Status, char* requestId, void* payload)
{
        printf("\n------------------------------------\n" );
        printf("Status :: %s\n", Status);
        printf("requestId : %s\n", requestId);
        printf("Payload is : %s\n", (char *)payload);

        printf("------------------------------------\n" );
}

void rebootCallBack (char* reqID, char* action, void* payload)
{
	printf("\n-----------REBOOT-------------------------\n" );
	printf("request Id :: %s\n", reqID);
	printf("action : %s\n", action);
	printf("Payload is : %s\n", (char *)payload);

	int rc = changeState(REBOOT_INITIATED);
	/*#if defined(_WIN32) || defined(_WIN64)
		system("C:\\WINDOWS\\System32\\shutdown -r");
	#else
		system("sudo shutdown -r now");
	#endif*/


	printf("------------------------------------\n" );
}

void factoryResetCallBack (char* reqID, char* action, void* payload)
{
	printf("\n--------------FACTORYRESET----------------------\n" );
	printf("request Id: %s\n", reqID);
	printf("action : %s\n", action);
	printf("Payload is : %s\n", (char *)payload);

	/**
	* This sample doesn't support factory reset, so respond accordingly
	*/
	int rc = changeState(FACTORYRESET_NOTSUPPORTED);
	printf("Factory reset is not supported in this sample\n" );
	printf("------------------------------------\n" );
}

void firmwareDownloadCallBack()
{
	printf("\n--------------Firmware Download----------------------\n" );
	//Add the code for downloading the firmware
	changeFirmwareState(FIRMWARESTATE_DOWNLOADING);
	sleep(5);
	changeFirmwareState(FIRMWARE_DOWNLOADED);
}

void firmwareUpdateCallBack()
 {
	printf("\n--------------Firmware Update----------------------\n");
	//Add the code for updating the firmware
	changeFirmwareUpdateState(FIRMWAREUPDATE_INPROGRESS);
	sleep(5);
	changeFirmwareUpdateState(FIRMWAREUPDATE_SUCCESS);
	sleep(5);
	changeFirmwareState(FIRMWARESTATE_IDLE);
}
//all the DeviceData string variables needs to be allocated with the enough memory
void populateMgmtConfig(){
	strcpy(dmClient.DeviceData.deviceInfo.serialNumber, "10087" );
	strcpy(dmClient.DeviceData.deviceInfo.manufacturer , "IBM");
	strcpy(dmClient.DeviceData.deviceInfo.model , "7865");
	strcpy(dmClient.DeviceData.deviceInfo.deviceClass , "A");
	strcpy(dmClient.DeviceData.deviceInfo.description , "My Ras");
	strcpy(dmClient.DeviceData.deviceInfo.fwVersion , "1.0.0");
	strcpy(dmClient.DeviceData.deviceInfo.hwVersion , "1.0");
	strcpy(dmClient.DeviceData.deviceInfo.descriptiveLocation , "EGL C");
	strcpy(dmClient.DeviceData.metadata.metadata ,"{}");
}

int main(int argc, char const *argv[])
{
	int rc = -1;

	//catch interrupt signal
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);

	char *configFilePath = "./device.cfg";

	rc = initialize_configfile_dm(configFilePath);

	if(rc != SUCCESS){
		printf("initialize failed and returned rc = %d.\n Quitting..", rc);
		return 0;
	}

	printf("Connecting to Watson Iot\n");
	rc = connectiotf_dm();

	if(rc != SUCCESS){
		printf("Connection failed and returned rc = %d.\n Quitting..", rc);
		return 0;
	}

	setCommandHandler_dm(myCallback);
	setManagedHandler_dm(managedCallBack );
	setFirmwareDownloadHandler(firmwareDownloadCallBack);
	setFirmwareUpdateHandler(firmwareUpdateCallBack);
	subscribeCommands_dm();

	char reqId[40];
	printf("\n publish manage ..\n");
	populateMgmtConfig();
	publishManageEvent(0,1,1, reqId);
	printf("\n Manage Event Exited: %s",reqId);

	/*printf("\n publish addLog ..\n");
	addLog(&client, "test","",1, reqId);
	printf("\n addLog Request Exit : %s",reqId);
*/
	while(!interruption)
	{
		printf("Publishing the event stat with rc ");
		rc= publishEvent_dm("status","json", "{\"d\" : {\"temp\" : 34 }}", QOS0);
		printf(" %d\n", rc);
		rc = yield_dm(100);
		sleep(2);
	}


	printf("Quitting!!\n");

	disconnect_dm();

	return 0;
}
