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
char** field_steps;
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
void print_field_steps();

void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
									IoT_Publish_Message_Params *params, void *pData) {
	IOT_UNUSED(pData);
	IOT_UNUSED(pClient);
	
	comm = (char*)params->payload;	
	
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
	field_steps = (char**)calloc(n, sizeof(char *));
	for(int i = 0; i < n; i++) {
		fields[i] = (int*) calloc(m, sizeof(int));
		field_steps[i] = (char*) calloc(m,sizeof(char));	
	}
	for(int i = 0; i < n; i++) {
		for(int j = 0; j < m; j++) {
			fscanf(file, "%d", &fields[i][j]);
			if(fields[i][j] == 1)
				field_steps[i][j] = '1';
			else field_steps[i][j] = ' ';
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
	rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
	if(SUCCESS != rc) {
		IOT_ERROR("Unable to set Auto Reconnect to true - %d", rc);
		return rc;
	}

	IOT_INFO("Subscribing...");
	rc = aws_iot_mqtt_subscribe(&client, "MyTopic", 7, QOS0, iot_subscribe_callback_handler, NULL);
	
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
	//IOT_INFO("Current position: (%d,%d)", current_i, current_j);
	//IOT_INFO("I look on: (%d,%d)", next_i, next_j);
	next_position();
	field_steps[current_i][current_j] = 'x';
	print_field_steps();
	field_steps[current_i][current_j] = ' ';
	
	
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
					make_step();
					field_steps[current_i][current_j] = 'x';
					print_field_steps();
					field_steps[current_i][current_j] = ' ';
					sleep(3);	
				break;
			case 2:
				turn_left();
				sprintf(message, "(%d,%d)", current_i, current_j);
				print_and_send("MyTopic_2");
				field_steps[current_i][current_j] = 'x';
				print_field_steps();
				field_steps[current_i][current_j] = ' ';
				//IOT_INFO( "Current position (%d,%d)", current_i, current_j);
				//IOT_INFO( "I look on: (%d,%d)", next_i, next_j);
				command = 0;
				break;
			case 3:
				turn_right();
				sprintf(message, "(%d,%d)", current_i, current_j);
				print_and_send("MyTopic_2");
				field_steps[current_i][current_j] = 'x';
				print_field_steps();
				field_steps[current_i][current_j] = ' ';
				//IOT_INFO( "Current position: (%d,%d)", current_i, current_j);
				//IOT_INFO( "I look on: (%d,%d)", next_i, next_j);
				command = 0;
				break;
			case 4:
				sprintf(message, "(%d,%d)", current_i, current_j);
				print_and_send("MyTopic_2");
				field_steps[current_i][current_j] = 'x';
				print_field_steps();
				field_steps[current_i][current_j] = ' ';
				//IOT_INFO( "Current position: (%d,%d)", current_i, current_j);
				//IOT_INFO( "I look on: (%d,%d)", next_i, next_j);
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

bool next_position() {
	if(fields[next_i][next_j] == 1) {
		if(command != 0) {
			sprintf(message, "FORWARD FALL!");
			print_and_send("MyTopic_2"); }
		IOT_INFO("%s", "FORWARD FALL!");
		command = 0;	
		return false;
	}
	return true;
}

void turn_left() {
	switch(where_look()) {
		case 1: // forward
			next_i = current_i;
			next_j = current_j - 1;
			break;
		case 2: // right
			next_i = current_i - 1;
			next_j = current_j;
			break;
		case 3: // down
			next_i = current_i;
			next_j = current_j + 1;
			break;
		case 4: // left
			next_i = current_i + 1;
			next_j = current_j;
			break;
	}
}


void turn_right() {
	switch(where_look()) {
		case 1: // forward
			next_i = current_i;
			next_j = current_j + 1;
			break;
		case 2: // right
			next_i = current_i + 1;
			next_j = current_j;
			break;
		case 3: // down
			next_i = current_i;
			next_j = current_j - 1;
			break;
		case 4:	// left
			next_i = current_i - 1;
			next_j = current_j;
			break;
	}
}

int where_look() {
	if(current_i > next_i) // forward
		return 1;
	else if(current_j < next_j) // right
		return 2;
	else if(current_i < next_i) // down
		return 3;
	else if(current_j > next_j) // left
		return 4;
}


void make_step() {
	switch(where_look()) {
		case 1: // forward
			current_i--;
			next_i--;
			break;
		case 2: // right
			current_j++;
			next_j++;
			break;
		case 3: // down
			current_i++;
			next_i++;
			break;
		case 4: // left
			current_j--;
			next_j--;
			break;
	}
	
	
	sprintf(message, "(%d,%d)", current_i, current_j);
	print_and_send("MyTopic_2");
	IOT_INFO( "Current position: (%d,%d)", current_i, current_j);
	IOT_INFO( "I look on: (%d,%d)", next_i, next_j);
	
		
}

void print_and_send(char* nameTopic) {
	p.payload = (void*)message;
	p.payloadLen = strlen(message);
	rc = aws_iot_mqtt_publish(&client, nameTopic, 9, &p);
}

void print_field_steps() {
	for(int i = 0; i < n; i++) {
		for(int j = 0; j < m; j++) {
			printf("%c ", field_steps[i][j]);
		}
		printf("\n");	
	}
}
