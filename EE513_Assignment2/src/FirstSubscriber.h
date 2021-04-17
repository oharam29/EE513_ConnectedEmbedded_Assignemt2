/*
 * FirstSubscriber.h
 *
 *  Created on: 17 Apr 2021
 *      Author: Nova6
 */

#ifndef FIRSTSUBSCRIBER_H_
#define FIRSTSUBSCRIBER_H_

class FirstSubsciber {
public:
	void connlost(void *context, char *cause);
	int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
	void delivered(void *context, MQTTClient_deliveryToken dt);
};

#endif /* FIRSTSUBSCRIBER_H_ */

