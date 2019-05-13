#include "common.h"

void EXITONERROR(string msg)
{
    perror(msg.c_str());
    exit(-1);
}

vector<string> split (string line, char separator){
	vector<string> result;
	while (line.size()){
		size_t found = line.find_first_of (separator);
		if (found!=std::string::npos){
			string part = line.substr(0, found);
			result.push_back(part);
			line = line.substr (found+1);
		}
		else{
			result.push_back (line);
			break;
		}
	}
	return result;
}

__int64_t get_file_size (string filename){
    struct stat buf;
    int fd = open (filename.c_str (), O_RDONLY);
    fstat(fd, &buf);
    __int64_t size = (__int64_t) buf.st_size;
    close (fd);
    return size;
}

RequestChannel* create_channel (string communicationMethod, string name, RequestChannel::Side side, int m){
	RequestChannel* channel;

	if (communicationMethod == "f"){
		channel = new FIFORequestChannel (name, side, m);
	}
	else if (communicationMethod == "q"){
		channel = new MQRequestChannel (name, side, m);
	}
	else if (communicationMethod == "m"){
		channel = new SHMRequestChannel (name,  side, m);
	}
	return channel;

}
