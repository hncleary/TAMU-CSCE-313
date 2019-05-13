
#ifndef _MQreqchannel_H_
#define _MQreqchannel_H_

#include "common.h"
#include <mqueue.h>
#include <fcntl.h>

class MQRequestChannel: public RequestChannel
{
private:
	mqd_t wfd;
	mqd_t rfd;

	string q1, q2;
	int open_queue(string _name, int mode);



public:
	MQRequestChannel(const string _name, const Side _side, int _bufs);

	~MQRequestChannel();

	char* cread(int *len=NULL);

	int cwrite(char *msg, int msglen);



};

#endif
