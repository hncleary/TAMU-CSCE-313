#include "common.h"
#include "reqchannel.h"
#include "FIFOreqchannel.h"
using namespace std;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR FOR CLASS   R e q u e s t C h a n n e l  */
/*--------------------------------------------------------------------------*/

FIFORequestChannel::FIFORequestChannel(const string _name, const Side _side, int _bufs) : RequestChannel(_name, _side, _bufs)
{
	pipe1 = "fifo_" + my_name + "1";
	pipe2 = "fifo_" + my_name + "2";

	if (_side == SERVER_SIDE){
		wfd = open_pipe(pipe1, O_WRONLY);
		rfd = open_pipe(pipe2, O_RDONLY);
	}
	else{
		rfd = open_pipe(pipe1, O_RDONLY);
		wfd = open_pipe(pipe2, O_WRONLY);

	}
	buffersize = MAX_MESSAGE;

}

FIFORequestChannel::~FIFORequestChannel()
{
	//if (my_side== CLIENT_SIDE)
		//return;
	close(wfd);
	close(rfd);

	remove(pipe1.c_str());
	remove(pipe2.c_str());
}

int FIFORequestChannel::open_pipe(string _pipe_name, int mode)
{
	mkfifo (_pipe_name.c_str (), 0600);
	int fd = open(_pipe_name.c_str(), mode);
	if (fd < 0){
		EXITONERROR(_pipe_name);
	}
	return fd;
}

char* FIFORequestChannel::cread(int *len)
{
	char * buf = new char [buffersize];
	int length;
	if ((length = read(rfd, buf, buffersize)) <= 0){
		if (my_side == SERVER_SIDE)
			EXITONERROR("SERVER - cread");
		else
			EXITONERROR("CLIENT - cread");
	}
	if (len)	// the caller wants to know the length
		*len = length;
	return buf;
}

int FIFORequestChannel::cwrite(char* msg, int len)
{
	if (len > MAX_MESSAGE){
		EXITONERROR("cwrite");
	}
	if (write(wfd, msg, len) < 0){
		EXITONERROR("cwrite");
	}
	return len;
}
