/*
 * publish.cpp
 *
 *  Created on: 15 Apr 2021
 *      Author: Nova6
 */
#include "ADXL345.h"
// Based on the Paho C code example from www.eclipse.org/paho/
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include "MQTTClient.h"

#define  CPU_TEMP "/sys/class/thermal/thermal_zone0/temp"
using namespace std;

//Please replace the following address with the address of your server
#define ADDRESS    "tcp://192.168.1.51:1883"
#define CLIENTID   "rpi_publisher"
#define AUTHMETHOD "mike"
#define AUTHTOKEN  "password"
#define TOPIC      "ee513/test"
#define QOS        1
#define TIMEOUT    10000L

float getCPUTemperature() {        // get the CPU temperature
   int cpuTemp;                    // store as an int
   fstream fs;
   fs.open(CPU_TEMP, fstream::in); // read from the file
   fs >> cpuTemp;
   fs.close();
   return (((float)cpuTemp)/1000);
}

void getTimeonPi(char* curTime){
	time_t timeonPi = time(NULL);
	struct tm tm = *localtime(&timeonPi);
	sprintf(curTime, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
}

int main(int argc, char* argv[]) {
   char str_payload[1024];          // Set your max message size here
   MQTTClient client;
   MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
   MQTTClient_willOptions Willopts = MQTTClient_willOptions_initializer;
   MQTTClient_message pubmsg = MQTTClient_message_initializer;
   MQTTClient_deliveryToken token;
   MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
   opts.keepAliveInterval = 20;
   opts.cleansession = 1;
   opts.username = AUTHMETHOD;
   opts.password = AUTHTOKEN;
   
   //setting last will and test message
   opts.will = &Willopts;
   opts.will->message = "Raspberry Pi connection has been unexpectedly ended";
   opts.will->qos = 1;
   opts.will->retained = 0;
   opts.will->topicName = TOPIC;
   
   int rc;
   if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
      cout << "Failed to connect, return code " << rc << endl;
      return -1;
   }
   
   for (int qos=0; qos<3; qos++){
   	   //get temp
   	   float CPUt = getCPUTemperature();
   
   	   //get time
   	   char piTime[10];
   	   getTimeonPi(piTime);

   	   //init ADXL345 and get data from it
   	   ADXL345 sensor(1, 0x53);
   	   int x1, y1,z1 = 0;
   	   sensor.readAllADXL345Data(x1,y1,z1);
   
   	   //build payload
   	   sprintf(str_payload, "\n{\n");
   	   sprintf(str_payload + strlen(str_payload), "\"CPUTemp\": %f,\n", CPUt);
   	   sprintf(str_payload + strlen(str_payload),"\"Time(at publish)\": \"%s\", \n", piTime);
   	   sprintf(str_payload + strlen(str_payload), "\"ADXL345 Data\": {\n");
   	   sprintf(str_payload + strlen(str_payload), "\"X\": %d,\n", x1);
   	   sprintf(str_payload + strlen(str_payload), "\"Y\": %d,\n", y1);
   	   sprintf(str_payload + strlen(str_payload), "\"Z\": %d,\n", z1);
   	   sprintf(str_payload + strlen(str_payload), "   }\n");
   	   sprintf(str_payload + strlen(str_payload), "}");

   	   pubmsg.payload = str_payload;
   	   pubmsg.payloadlen = strlen(str_payload);
   	   pubmsg.qos = qos;
   	   pubmsg.retained = 0;
   	   MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
   	   cout << "Waiting for up to " << (int)(TIMEOUT/1000) <<
   			   " seconds for publication of " << str_payload <<
			   " \non topic " << TOPIC << " for ClientID: " << CLIENTID << endl;

   	   rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);

   	   cout << "Message with token " << (int)token << " delivered." << endl;
   	   cout << "QoS: " << (int)qos << "." << endl;
       usleep(100000);
   }
   
   MQTTClient_disconnect(client, 30000);
   MQTTClient_destroy(&client);
   return rc;
}
