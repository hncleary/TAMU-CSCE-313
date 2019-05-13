#ifndef BoundedBuffer_h
#define BoundedBuffer_h

#include <stdio.h>
#include <queue>
#include <string>
#include <pthread.h>

using namespace std;

class BoundedBuffer
{
private:
  int cap;
  queue< vector<char> > q;
  //vector<char> v;
  pthread_mutex_t mutex;
  pthread_cond_t cond1;
  pthread_cond_t cond2;

public:
	BoundedBuffer(int _cap){
      cap = _cap;
      pthread_mutex_init (&mutex, 0);
      pthread_cond_init (&cond1, 0);
      pthread_cond_init (&cond2, 0);
	}
	~BoundedBuffer(){
      pthread_mutex_destroy (&mutex);
      pthread_cond_destroy(&cond1);
      pthread_cond_destroy(&cond2);
	}

	void push(char* data, int len){
    pthread_mutex_lock (&mutex);
    vector<char> v(data, data+len);
    while (q.size() >= cap){
      pthread_cond_wait (&cond2, &mutex);
    }
    q.push(v);
    pthread_cond_signal (&cond1);
    pthread_mutex_lock (&mutex);

	}

	vector<char> pop(){
    pthread_mutex_lock (&mutex);
		while (q.size() == 0){
      pthread_cond_wait (&cond1, &mutex);
    }
    vector<char> t = q.front();
    q.pop();
    pthread_cond_signal (&cond2);
    pthread_mutex_lock (&mutex);
    return t;
	}

  int size(){
    return q.size();
  }
};

#endif /* BoundedBuffer_ */
