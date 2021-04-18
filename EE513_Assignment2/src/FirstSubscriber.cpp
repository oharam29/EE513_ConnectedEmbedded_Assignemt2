/*
 * FirstSubscriber.cpp
 *
 *  Created on: 17 Apr 2021
 *      Author: Nova6
 */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <iostream>
#include "MQTTClient.h"


#include<json-c/json.h>


using namespace std;

#define ADDRESS     "tcp://192.168.1.31:1883"
#define CLIENTID    "Subscriber1"
#define AUTHMETHOD  "mike"
#define AUTHTOKEN   "password"
#define TOPIC       "ee513/test"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt) {
    printf("Subscriber Number 1 has received Message with token value %d\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {

	struct json_object *parsed_json;
	struct json_object *CPUt;
	struct json_object *piTime;
	struct json_object *parsedX;
	struct json_object *parsedY;
	struct json_object *parsedZ;



	parsed_json = json_tokener_parse((char*)message->payload);
	json_object_object_get_ex(parsed_json, "CPUTemp", &CPUt);
	json_object_object_get_ex(parsed_json, "Time(at publish)", &piTime);
	json_object_object_get_ex(parsed_json, "X", &parsedX);
	json_object_object_get_ex(parsed_json, "Y", &parsedY);
	json_object_object_get_ex(parsed_json, "Z", &parsedZ);

	cout << "Outputting message:" << endl;
    printf("CPU Temp: %d degrees    (Topic Publihsed to: %s)\n", json_object_get_int(CPUt), topicName);
    printf("Current Time: %s    (Topic Publihsed to: %s)\n", json_object_get_string(piTime), topicName);
    printf("X Co-ord: %d    (Topic Publihsed to: %s)\n", json_object_get_int(parsedX), topicName);
    printf("Y Co-ord: %d    (Topic Publihsed to: %s)\n", json_object_get_int(parsedY), topicName);
    printf("Z Co-ord: %d    (Topic Publihsed to: %s)\n", json_object_get_int(parsedZ), topicName);


    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

int main(int argc, char* argv[]) {
    MQTTClient client;
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;
    int ch;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;

    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
    MQTTClient_subscribe(client, TOPIC, QOS);

    do {
        ch = getchar();
    } while(ch!='Q' && ch != 'q');
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}


