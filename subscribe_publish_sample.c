/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

/**
 * @file subscribe_publish_sample.c
 * @brief simple MQTT publish and subscribe on the same topic
 *
 * This example takes the parameters from the aws_iot_config.h file and establishes a connection to the AWS IoT MQTT Platform.
 * It subscribes and publishes to the same topic - "sdkTest/sub"
 *
 * If all the certs are correct, you should see the messages received by the application in a loop.
 *
 * The application takes in the certificate path, host name , port and the number of times the publish should happen.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"

/**
 * @brief Default cert location
 */

IoT_Error_t rc = FAILURE;

char certDirectory[PATH_MAX + 1] = "../../../certs";

char MESSAGE_GO[] = "I'M GOING";
char MESSAGE_TURN_LEFT[] = "I'M TURN ON THE LEFT";
char MESSAGE_TURN_RIGHT[] = "I'M TURN ON THE RIGHT";
char MESSAGE_STOP[] = "I'M STOPING";

char GO[10]="GO   ";
char LEFT[10]="LEFT ";
char RIGHT[10]="RIGHT";
char STOP[10]="STOP ";

IoT_Publish_Message_Params p;
AWS_IoT_Client client;
char message[] = "";

FILE *file;
bool flag = false;
int command;
char* comm;
int last_command;

int n, m;
int **fields;

/**
 * @brief Default MQTT HOST URL is pulled from the aws_iot_config.h
 */
char HostAddress[255] = AWS_IOT_MQTT_HOST;

/**
 * @brief Default MQTT port is pulled from the aws_iot_config.h
 */
uint32_t port = AWS_IOT_MQTT_PORT;

/**
 * @brief This parameter will avoid infinite loop of publish and exit the program after certain number of publishes
 */
uint32_t publishCount = 3;
bool next_position();
void readFile();
void turn_left();
void turn_right();
void make_step();
int where_look();
void print_and_send(char* nameTopic);

void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
									IoT_Publish_Message_Params *params, void *pData) {
	IOT_UNUSED(pData);
	IOT_UNUSED(pClient);
	//char myTopic[10] = "MyTopic";
	//IOT_INFO("%s",topicName);
		
	//if(strcmp(topicName, myTopic) == 0) {
	comm = (char*)params->payload;	
	//}
	IOT_INFO("%s",comm);
	
	if(strcmp(comm, GO)==0) {
		command = 1;
		IOT_INFO("%s", MESSAGE_GO);
		
	}
	else if (strcmp(comm, LEFT)==0) {
		command = 2;
		IOT_INFO("%s", MESSAGE_TURN_LEFT);
		
	}
	else if (strcmp(comm, RIGHT)==0) {
		command = 3;
		IOT_INFO("%s", MESSAGE_TURN_RIGHT);
		
	}
	else if (strcmp(comm, STOP)==0) {
		command = 4;
		IOT_INFO("%s", MESSAGE_STOP);
		
	}
	
}

void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data) {
	IOT_WARN("MQTT Disconnect");
	IoT_Error_t rc = FAILURE;

	if(NULL == pClient) {
		return;
	}

	IOT_UNUSED(data);

	if(aws_iot_is_autoreconnect_enabled(pClient)) {
		IOT_INFO("Auto Reconnect is enabled, Reconnecting attempt will start now");
	} else {
		IOT_WARN("Auto Reconnect not enabled. Starting manual reconnect...");
		rc = aws_iot_mqtt_attempt_reconnect(pClient);
		if(NETWORK_RECONNECTED == rc) {
			IOT_WARN("Manual Reconnect Successful");
		} else {
			IOT_WARN("Manual Reconnect Failed - %d", rc);
		}
	}
}
									



void parseInputArgsForConnectParams(int argc, char **argv) {
	int opt;

	while(-1 != (opt = getopt(argc, argv, "h:p:c:x:"))) {
		switch(opt) {
			case 'h':
				strcpy(HostAddress, optarg);
				IOT_DEBUG("Host %s", optarg);
				break;
			case 'p':
				port = atoi(optarg);
				IOT_DEBUG("arg %s", optarg);
				break;
			case 'c':
				strcpy(certDirectory, optarg);
				IOT_DEBUG("cert root directory %s", optarg);
				break;
			case 'x':
				publishCount = atoi(optarg);
				IOT_DEBUG("publish %s times\n", optarg);
				break;
			case '?':
				if(optopt == 'c') {
					IOT_ERROR("Option -%c requires an argument.", optopt);
				} else if(isprint(optopt)) {
					IOT_WARN("Unknown option `-%c'.", optopt);
				} else {
					IOT_WARN("Unknown option character `\\x%x'.", optopt);
				}
				break;
			default:
				IOT_ERROR("Error in command line argument parsing");
				break;
		}
	}

}

void readFile() 
{
	fscanf(file, "%d", &n);
	fscanf(file, "%d", &m);
	fields = (int**) calloc(n, sizeof(int *));
	for(int i = 0; i < n; i++) {
		fields[i] = (int*) calloc(m, sizeof(int));	
	}
	for(int i = 0; i < n; i++) {
		for(int j = 0; j < m; j++) {
			fscanf(file, "%d", &fields[i][j]);
		}
	}
}

int current_i, current_j, next_i, next_j;


int main(int argc, char **argv) {
	bool infinitePublishFlag = true;

	char rootCA[PATH_MAX + 1];
	char clientCRT[PATH_MAX + 1];
	char clientKey[PATH_MAX + 1];
	char CurrentWD[PATH_MAX + 1];
	char cPayload[100];
	comm = (char*) malloc(sizeof(char) * 100);
	int32_t i = 0;
	
	IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
	IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

	
	parseInputArgsForConnectParams(argc, argv);

	IOT_INFO("\nAWS IoT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

	getcwd(CurrentWD, sizeof(CurrentWD));
	snprintf(rootCA, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_ROOT_CA_FILENAME);
	snprintf(clientCRT, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_CERTIFICATE_FILENAME);
	snprintf(clientKey, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_PRIVATE_KEY_FILENAME);

	IOT_DEBUG("rootCA %s", rootCA);
	IOT_DEBUG("clientCRT %s", clientCRT);
	IOT_DEBUG("clientKey %s", clientKey);
	mqttInitParams.enableAutoReconnect = false; // We enable this later below
	mqttInitParams.pHostURL = HostAddress;
	mqttInitParams.port = port;
	mqttInitParams.pRootCALocation = rootCA;
	mqttInitParams.pDeviceCertLocation = clientCRT;
	mqttInitParams.pDevicePrivateKeyLocation = clientKey;
	mqttInitParams.mqttCommandTimeout_ms = 20000;
	mqttInitParams.tlsHandshakeTimeout_ms = 5000;
	mqttInitParams.isSSLHostnameVerify = true;
	mqttInitParams.disconnectHandler = disconnectCallbackHandler;
	mqttInitParams.disconnectHandlerData = NULL;

	rc = aws_iot_mqtt_init(&client, &mqttInitParams);
	if(SUCCESS != rc) {
		IOT_ERROR("aws_iot_mqtt_init returned error : %d ", rc);
		return rc;
	}

	connectParams.keepAliveIntervalInSec = 10;
	connectParams.isCleanSession = true;
	connectParams.MQTTVersion = MQTT_3_1_1;
	connectParams.pClientID = AWS_IOT_MQTT_CLIENT_ID;
	connectParams.clientIDLen = (uint16_t) strlen(AWS_IOT_MQTT_CLIENT_ID);
	connectParams.isWillMsgPresent = false;

	IOT_INFO("Connecting...");
	rc = aws_iot_mqtt_connect(&client, &connectParams);
	if(SUCCESS != rc) {
		IOT_ERROR("Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
		return rc;
	}
	/*
	 * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
	 *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
	 *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
	 */
	rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
	if(SUCCESS != rc) {
		IOT_ERROR("Unable to set Auto Reconnect to true - %d", rc);
		return rc;
	}

	IOT_INFO("Subscribing...");
	rc = aws_iot_mqtt_subscribe(&client, "MyTopic", 7, QOS0, iot_subscribe_callback_handler, NULL);
	//aws_iot_mqtt_subscribe(&client, "MyTopic_2", 9, QOS0, iot_subscribe_callback_handler, NULL);
	if(SUCCESS != rc) {
		IOT_ERROR("Error subscribing : %d ", rc);
		return rc;
	}
	
	p.qos = QOS0;
	p.isRetained = 0;
	
	file=fopen("field.txt", "r");
	readFile();
	
	current_i = 1;
	current_j = 1;
	next_i = 0;
	next_j = 1;
	IOT_INFO("Current position: (%d,%d)", current_i, current_j);
	IOT_INFO("I look on: (%d,%d)", next_i, next_j);
	next_position();
	
	
	bool first = true;
	while((NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc)
		 || infinitePublishFlag) {
		
		
		//Max time the yield function will wait for read messages
		rc = aws_iot_mqtt_yield(&client, 100);
		if(NETWORK_ATTEMPTING_RECONNECT == rc) {
			// If the client is attempting to reconnect we will skip the rest of the loop.
			continue;
		}
		
		
		

		switch(command) {
			case 1:
				if(!next_position()) 
					break; 
				//while(next_position() && command == 1 && command != 4) {
					make_step();				
					sleep(3);				
				//} 
				//command = 0;
				break;
			case 2:
				turn_left();
				sprintf(message, "(%d,%d)", current_i, current_j);
				print_and_send("MyTopic_2");
				IOT_INFO( "Current position (%d,%d)", current_i, current_j);
				//sprintf(message, "I look on: (%d,%d)", next_i, next_j);
				//print_and_send("MyTopic_2");
				IOT_INFO( "I look on: (%d,%d)", next_i, next_j);
				command = 0;
				break;
			case 3:
				turn_right();
				sprintf(message, "(%d,%d)", current_i, current_j);
				print_and_send("MyTopic_2");
				IOT_INFO( "Current position: (%d,%d)", current_i, current_j);
				//sleep(3);
				
				//sprintf(message, "I look on: (%d,%d)", next_i, next_j);
				//print_and_send("MyTopic_2");
				IOT_INFO( "I look on: (%d,%d)", next_i, next_j);
				
				command = 0;
				break;
			case 4:
				sprintf(message, "(%d,%d)", current_i, current_j);
				print_and_send("MyTopic_2");
				IOT_INFO( "Current position: (%d,%d)", current_i, current_j);
				IOT_INFO( "I look on: (%d,%d)", next_i, next_j);
				command = 0;	
				break;
		}
		
	}

	if(SUCCESS != rc) {
		IOT_ERROR("An error occurred in the loop.\n");
	} else {
		IOT_INFO("Publish done\n");
	}

	return rc;
}




