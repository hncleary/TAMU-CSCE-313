#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

#include "reqchannel.h"
using namespace std;



RequestChannel::RequestChannel(const string _name, const Side _side, int _s) : my_name(_name), my_side(_side), buffersize(_s)
{}

RequestChannel::~RequestChannel(){

}

string RequestChannel::name()
{
    return my_name;
}
