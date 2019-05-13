#include "common.h"
#include "reqchannel.h"
#include "SHMRequestChannel.h"

using namespace std;

//constructor & destructors

SHMRequestChannel::SHMRequestChannel(const string _name, const Side _side, int _bs) : RequestChannel(_name, _side, _bs)
{
	side1 = "/bb_" + my_name + "1";
	side2 = "/bb_" + my_name + "2";

	if (_side == SERVER_SIDE){
		buffer1 = new SMBB (side1, _bs);
		buffer2 = new SMBB (side2, _bs);
	}else{
		buffer1 = new SMBB (side2, _bs);
		buffer2 = new SMBB (side1, _bs);
	}
}

SHMRequestChannel::~SHMRequestChannel()
{
	delete buffer1;
	delete buffer2;
}

char* SHMRequestChannel::cread(int *len)
{
	vector<char> data = buffer2->pop();
	char* buf = new char [data.size()];
	memcpy (buf, data.data(), data.size());
	if (len)
		*len = data.size();
	return buf;
}

int SHMRequestChannel::cwrite(char* msg, int len){
	buffer1->push(vector<char>(msg, msg+len));
	return len;
}
