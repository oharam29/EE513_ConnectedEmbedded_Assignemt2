/*
 * subscribertemplate.h
 *
 *  Created on: 15 Apr 2021
 *      Author: Nova6
 */

#ifndef SUBSCRIBERTEMPLATE_H_
#define SUBSCRIBERTEMPLATE_H_

class subscriber_template {
public:
	subscriber_template();
	virtual ~subscriber_template();
	subscriber_template(const subscriber_template &other);

	void connlost(void *context, char *cause);
	int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
	void delivered(void *context, MQTTClient_deliveryToken dt);
};

#endif /* SUBSCRIBERTEMPLATE_H_ */
