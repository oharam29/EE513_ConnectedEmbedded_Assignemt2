/*
 * SecondSubscriber.h
 *
 *  Created on: 17 Apr 2021
 *      Author: Nova6
 */

#ifndef SECONDSUBSCRIBER_H_
#define SECONDSUBSCRIBER_H_

class SecondSubscriber {
public:
	void connlost(void *context, char *cause);
	int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
	void delivered(void *context, MQTTClient_deliveryToken dt);
};

#endif /* SECONDSUBSCRIBER_H_ */
