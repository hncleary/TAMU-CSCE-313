
#ifndef _SHMreqchannel_H_
#define _SHMreqchannel_H_

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>

#include "common.h"

class Sempahore;

class SHMRequestChannel: public RequestChannel
{
private:


	string side1, side2;
	Sempahore* buffer1;
	Sempahore* buffer2;


public:
	SHMRequestChannel(const string _name, const Side _side, int _bs);

	~SHMRequestChannel();

	char* cread(int *len=NULL);

	int cwrite(char *msg, int msglen);


};



class Sempahore{
private:
  string name;
  sem_t* prod_done;
	sem_t* cons_done;

	string semname1, semname2;
	string shmname;
	int shmfd;
	char* data;
	int buffersize;


public:
    	Sempahore (string _name, int _bufsz):buffersize(_bufsz), name(_name){

      shmfd = shm_open(name.c_str(), O_RDWR|O_CREAT, 0644 );

      if (shmfd < 0){
    		exit (0);
    	}
    	ftruncate(shmfd, buffersize);  //set the length
    	data = (char *) mmap(NULL, buffersize, PROT_READ|PROT_WRITE, MAP_SHARED, shmfd, 0);
    	if (!data){
    	    exit (0);
    	}

    	semname1 = name + "1";
    	prod_done = sem_open (semname1.c_str(), O_CREAT, 0644, 0);
    	semname2 = name + "2";
    	cons_done = sem_open (semname2.c_str(), O_CREAT, 0644, 1);
    }

    ~Sempahore(){
        munmap (data, buffersize);
    	close (shmfd);
    	shm_unlink (name.c_str());

    	sem_close (prod_done);
    	sem_close (cons_done);

    	sem_unlink (semname1.c_str());
    	sem_unlink (semname2.c_str());
    }

    void push (vector<char> msg){
        sem_wait (cons_done);
        memcpy (data, msg.data(), msg.size());
        sem_post (prod_done);
    }

    vector<char> pop (){
        sem_wait (prod_done);
        vector<char> t (data, data + buffersize);
        sem_post (cons_done);
        return t;
    }
};


#endif
