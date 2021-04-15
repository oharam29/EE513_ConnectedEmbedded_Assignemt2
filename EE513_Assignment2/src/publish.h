/*
 * publish.h
 *
 *  Created on: 15 Apr 2021
 *      Author: Nova6
 */

#ifndef PUBLISH_H_
#define PUBLISH_H_

class publish {
public:
	publish();
	virtual ~publish();
	publish(const publish &other);
	float getCPUTemperature();
};

#endif /* PUBLISH_H_ */
