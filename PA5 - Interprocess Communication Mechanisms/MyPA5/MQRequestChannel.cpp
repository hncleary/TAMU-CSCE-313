#include "common.h"
#include "reqchannel.h"
#include "MQRequestChannel.h"
using namespace std;

//constructors and destructors

MQRequestChannel::MQRequestChannel(const string _name, const Side _side, int _bs) : RequestChannel(_name, _side, _bs)
{
	q1 = "/mq_" + my_name + "1";
	q2 = "/mq_" + my_name + "2";

	if (_side == SERVER_SIDE){
		wfd = open_queue (q1, O_WRONLY);
		rfd = open_queue (q2, O_RDONLY);
	}
	else{
		rfd = open_queue (q1, O_RDONLY);
		wfd = open_queue (q2, O_WRONLY);

	}
}

MQRequestChannel::~MQRequestChannel()
{
	mq_close(wfd);
    mq_unlink(q1.c_str());

	mq_close(rfd);
    mq_unlink(q2.c_str());
}

int MQRequestChannel::open_queue (string _name, int mode){
	struct mq_attr attr;
	attr.mq_maxmsg = 5;
	attr.mq_msgsize = buffersize;

	int fd = mq_open(_name.c_str(), O_RDWR | O_CREAT, 0664, &attr);
	if (fd < 0) {
		EXITONERROR ("Failure");
	}
	return fd;
}

char* MQRequestChannel::cread(int *len){
	int nb;
	char* buf = new char [buffersize];
	if ((nb = mq_receive(rfd, buf, buffersize, NULL)) < 0){
		EXITONERROR ("Failure");
	}
	if (len)
		*len = nb;
	return buf;
}

int MQRequestChannel::cwrite(char* msg, int len){
	if (mq_send (wfd, msg, len, 0)<0){
		EXITONERROR ("Failure");
	}
	return len;
}
