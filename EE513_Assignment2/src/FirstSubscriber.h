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
	subscriber_template();
	virtual ~subscriber_template();
	subscriber_template(const subscriber_template &other);

	void connlost(void *context, char *cause);
	int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
	void delivered(void *context, MQTTClient_deliveryToken dt);
};

#endif /* FIRSTSUBSCRIBER_H_ */

