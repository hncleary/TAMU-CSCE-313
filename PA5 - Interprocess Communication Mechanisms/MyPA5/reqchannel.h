
#ifndef _reqchannel_H_
#define _reqchannel_H_

#include <iostream>
#include <fstream>
#include <exception>
#include <string>
using namespace std;


class RequestChannel
{
public:
	enum Side {SERVER_SIDE, CLIENT_SIDE};
	enum Mode {READ_MODE, WRITE_MODE};

	string my_name;
	Side my_side;

	int buffersize;


	RequestChannel(const string _name, const Side _side, int _buf);


	virtual ~RequestChannel();


	string name();

	virtual char* cread(int * len = NULL) = 0;

	virtual int cwrite(char* buffer, int len) = 0;


};

#endif
