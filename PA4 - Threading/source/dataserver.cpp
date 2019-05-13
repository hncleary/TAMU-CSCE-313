#include <cassert>
#include <cstring>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include "FIFOreqchannel.h"
using namespace std;


int nchannels = 0;
pthread_mutex_t newchannel_lock;
void *handle_process_loop(void *_channel);
int bufsize = MAX_MESSAGE;
vector<string> all_data [NUM_PERSONS];


void process_newchannel_request (FIFORequestChannel *_channel)
{
	nchannels++;
	string new_channel_name = "data" + to_string(nchannels) + "_";
	char buf [30];
	strcpy (buf, new_channel_name.c_str());
	_channel->cwrite(buf, new_channel_name.size()+1);

	FIFORequestChannel *data_channel = new FIFORequestChannel(new_channel_name, FIFORequestChannel::SERVER_SIDE);

	pthread_t thread_id;
	if (pthread_create(&thread_id, NULL, handle_process_loop, data_channel) < 0){
		EXITONERROR("");
	}
}

void populate_file_data (int person){
	cout << "populating for person " << person << endl;
	string filename = "BIMDC/" + to_string(person) + ".csv";
	char line[100];
	ifstream ifs (filename.c_str());
	int count = 0;
	while (!ifs.eof()){
		line[0] = 0;
		ifs.getline(line, 100);
		if (ifs.eof())
			break;
		double seconds = stod (split (string(line), ',')[0]);
		if (line [0])
			all_data [person-1].push_back(string(line));
	}
}

double get_data_from_memory (int person, double seconds, int ecgno){
	int index = (int)round (seconds / 0.004);
	string line = all_data [person-1][index]; 
	vector<string> parts = split (line, ',');
	double sec = stod(parts [0]);
	double ecg1 = stod (parts [1]);
	double ecg2 = stod (parts [2]); 
	if (ecgno == 1)
		return ecg1;
	else
		return ecg2;
}

void process_file_request (FIFORequestChannel* rc, char* request){
	
	filemsg * f = (filemsg *) request;
	string filename = request + sizeof (filemsg);
	filename = "BIMDC/" + filename; // adding the path prefix to the requested file name
	cout << "Server received request for file " << filename << endl;

	if (f->offset == 0 && f->length == 0){ // means that the client is asking for file size
		__int64_t fs = get_file_size (filename);
		rc->cwrite ((char *)&fs, sizeof (__int64_t));
		return;
	}
	
	// make sure that client is not requesting too big a chunk
	assert (f->length <= bufsize);

	
	char buffer [bufsize];
	FILE* fp = fopen (filename.c_str(), "rb");
	if (!fp){
		EXITONERROR ("Cannot open " + filename);
	}
	fseek (fp, f->offset, SEEK_SET);
	int nbytes = fread (buffer, 1, f->length, fp);
	assert (nbytes == f->length);
	rc->cwrite (buffer, nbytes);
	fclose (fp);
}

void process_data_request (FIFORequestChannel* rc, char* request){
	datamsg* d = (datamsg* ) request;
	double data = get_data_from_memory (d->person, d->seconds, d->ecgno);
	rc->cwrite((char *) &data, sizeof (double));
}

void process_unknown_request(FIFORequestChannel *rc){
	char a = 0;
	rc->cwrite (&a, sizeof (a));
}


int process_request(FIFORequestChannel *rc, char* _request)
{
	MESSAGE_TYPE m = *(MESSAGE_TYPE *) _request;
	if (m == DATA_MSG){
		usleep (rand () % 5000);
		process_data_request (rc, _request);
	}
	else if (m == FILE_MSG){
		process_file_request (rc, _request);
			
	}else if (m == NEWCHANNEL_MSG){
		process_newchannel_request(rc);
	}
	else{
		process_unknown_request(rc);
	}
}

void *handle_process_loop(void *_channel)
{
	FIFORequestChannel *channel = (FIFORequestChannel *)_channel;
	for (;;){
		int len = 0;
		char * buffer = channel->cread(&len);
		if (len == 0)
			break;
		MESSAGE_TYPE m = *(MESSAGE_TYPE *) buffer;
		if (m == QUIT_MSG)
			break;
		process_request(channel, buffer);
		delete [] buffer;
	}
}


/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/


int main(int argc, char *argv[])
{
	srand(time_t(NULL));
	bufsize = MAX_MESSAGE; // modify this to accept bufsize m from the client side
	
	for (int i=0; i<NUM_PERSONS; i++){
		populate_file_data(i+1);
	}
	
	FIFORequestChannel control_channel("control", FIFORequestChannel::SERVER_SIDE);
	handle_process_loop(&control_channel);
}
