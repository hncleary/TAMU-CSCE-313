#include "common.h"
#include "BoundedBuffer.h"
#include "Histogram.h"
#include "common.h"
#include "HistogramCollection.h"
#include "FIFOreqchannel.h"
using namespace std;


class patient_thread_args{
public:
   /*put all the arguments for the patient threads */
   pthread_t thread;
   //added
   //how many reuests to make
   int requestCount;
   //person for whom to get info
   int patientNumber;
   //buffer to put requests into
   BoundedBuffer * requestBuffer;

   patient_thread_args(int RequestCount, int PatientNumber, BoundedBuffer* RequestBuffer){
     requestCount = RequestCount;
     patientNumber = PatientNumber;
     requestBuffer = RequestBuffer;
   }
   patient_thread_args(){

   }

};

class worker_thread_args{
public:
   /*put all the arguments for the worker threads */
   //added
   pthread_t thread;
   //request channel to server
   FIFORequestChannel *workerChannel;
   //buffer from which to get requests
   BoundedBuffer * requestBuffer;
   //histogram that is updated by data processes
   BoundedBuffer **statBuffer;

   worker_thread_args(FIFORequestChannel* Channel, BoundedBuffer* Buffer, BoundedBuffer** StatBuffer){
      workerChannel = Channel;
      requestBuffer = Buffer;
      //hist = Hist;
      statBuffer = StatBuffer;

   }
   worker_thread_args(){

   }

};

void *patient_thread_function(void *arg)
{
    /* What will the patient threads do? */

    //added
    patient_thread_args *args = (patient_thread_args *)arg;

    double seconds = 0.000;
    double stepSize = 0.004;

    datamsg dm(args->patientNumber, seconds, 1);

    for (int i = 0 ; i < args->requestCount; i ++){
      //arg->requestBuffer->push(args->name);
      args->requestBuffer->push((char*) &dm, sizeof(datamsg));
      dm.seconds += stepSize;
    }

}

void *worker_thread_function(void *arg)
{
    /*
		Functionality of the worker threads
    */

    //added
    worker_thread_args *args = (worker_thread_args *)arg;
    //FIFORequestChannel *workerChannel = args->channel;
    while(true){
        vector<char> request = args->requestBuffer->pop();
        char* command = (char *) request.data();
      //  workerChannel->cwrite(command, sizeof(MESSAGE_TYPE) );
        if(*(MESSAGE_TYPE *)command == QUIT_MSG){
          args->workerChannel->cwrite(command, sizeof(MESSAGE_TYPE));
          break;
        }
        else if(*(MESSAGE_TYPE *)command == DATA_MSG){
          datamsg * dm = (datamsg *) command;
          args->workerChannel->cwrite(command, sizeof(datamsg));
          char* response = args->workerChannel->cread();

          delete response;
        }
        else if (*(MESSAGE_TYPE *)command == FILE_MSG){
          //nothing
        }
        //Datamsg* data = (datamsg*) command;
        // MESSAGE_TYPE m = *(MESSAGE_TYPE *) command;
        // if(m == QUIT_MSG ){
        //   delete args->channel;
        //   break;
        // }
        // else{
        //   char* response = workerChannel->cread();
        //   args->hist->(request, response);
        // }
        //if(data.)
        // vector <char> request = buffer->pop()
        // Char* pointer = request.data()
        // Datamsg* data = (datamsg*) pointer
        //args->channel->cwrite(request );
        //args->channel->cwrite( command, sizeof(command) );
        //delete worker channel if request is quit
        // if (request == "quit"){
        //   delete args->channel;
        //   break;
        // }
        // else{
        // }
    }
}




/* Main Function */

int main(int argc, char *argv[])
{
    int n = 100;    //default number of requests per "patient"
    int w = 10;    //default number of worker threads
    int b = 20; 	// default capacity of the request buffer, you should change this default

    int p = 1;     // number of patients [1,15]
    int m = MAX_MESSAGE; 	// default capacity of the file buffer
    srand(time_t(NULL));

    //file name
    //if f isn't null, switch modes
    string f = "";

    //added
    //accept input arguments
    int opt = 0;
    while ((opt = getopt(argc, argv, "n:p:w:b:m:")) != -1) {
        switch (opt) {
          //number of requests per patient
            case 'n':
                n = atoi(optarg);
                break;
          //number of patients
            case 'p' :
                p = atoi(optarg);
                break;
          //capacity of the request buffer
            case 'b':
                b = atoi(optarg);
                //This won't do a whole lot until you fill in the worker thread function
                break;
          //number of worker threads
            case 'w':
                w = atoi(optarg);
                //This won't do a whole lot until you fill in the worker thread function
                break;
          //name of the file to be retrieved
            case 'f':
                f = optarg;
          //capacity of the file buffer
            case 'm':
                m = atoi(optarg);
                break;
			}
    }


    int pid = fork();
    //dataserver side
    if (pid == 0){
		// modify this to pass along m
        execl ("dataserver", "dataserver", (char *)NULL);

    }
    //added
    //client side


      if(f == ""){
        cout << "n ==" << n << endl;
        cout << "w ==" << w << endl;
        cout << "p ==" << p << endl;
        cout << "b ==" << b << endl;

        cout << "Client Started" << endl;
        cout << "File Request" << endl;
      }
      else if(f != ""){
        cout << "File Name: " << f << endl;
        cout << "Client Started" << endl;
        cout << "Data Request" << endl;
      }


    cout << "Initializing Control Channel..." << endl;
	  FIFORequestChannel* chan = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
    cout << "Finished Initializing Control Channel." << endl;
    BoundedBuffer request_buffer(b);
	  HistogramCollection hc;


    string patientsList [] = {"Patient 1", "Patient 2", "Patient 3, Patient 4",
                          "Patient 5", "Patient 6", "Patient 7", "Patient 8",
                            "Patient 9", "Patient 10", "Patient 11", "Patient 12",
                            "Patient 13", "Patient 14", "Patient 15"};

    //establish patient_thread_args
    BoundedBuffer **statBuffer = new BoundedBuffer *[p];
    for (int i = 0; i < p; i++){
      statBuffer[i] = new BoundedBuffer(ceil(b/1.0/p));
    }
    //
    //
    // vector<patient_thread_args *> patients;
    // vector<worker_thread_args *> workers;
    //
    // //for files
    // // vector<patients_thread_for_file *> patientsf;
    // // vector<worker_thread_for_file *> workersf;
    //
    // for(int i = 0; i < p ; i++){
    //   patient_thread_args *ptemp = new patient_thread_args(n, i+1, &request_buffer);
    //   patients.push_back(ptemp);
    // }
    //
    // for(int i = 0; i < w; i++){
    //   MESSAGE_TYPE m = NEWCHANNEL_MSG;
    //   chan->cwrite ((char *) &m, sizeof (MESSAGE_TYPE));
    //   string chan_name = chan->cread();
    //   FIFORequestChannel* wchan = new FIFORequestChannel (chan_name, FIFORequestChannel::CLIENT_SIDE);
    //   worker_thread_args* temp = new worker_thread_args (wchan, &request_buffer, statBuffer);
    //   workers.push_back (temp);
    // }
    //

    //code for file
    struct timeval start, end;
    gettimeofday (&start, 0);



    //
    // for(int i = 0; i < p; i++){
    //   pthread_create(&patients[i]->thread, NULL, patient_thread_function, patients[i]);
    // }
    // for(int i = 0; i < w; ++i){
    //   pthread_create(&workers[i]->thread, NULL, worker_thread_function, workers[i]);
    // }
    //
    // for(int i = 0; i < p ; i++){
    //   pthread_join(patients[i]->thread, 0);
    // }
    // cout << "Sending Requests... " << endl;
    // for (int i = 0 ; i < w ; ++i){
    //     MESSAGE_TYPE m = QUIT_MSG;
    //     request_buffer.push((char*)&m, sizeof(MESSAGE_TYPE));
    // }
    //
    // cout << "Finished Requests. " << endl;
    // for (int i = 0; i < w ; i++){
    //   pthread_join(workers[i]->thread, 0);
    // }
    //
    // cout << "Does it get this far?" << endl;

    // for(int i = 0 ; i < p ; i++){
    //   pthread_join (stats[i]->thread, 0);
    // }

    //more file code



    //array of patients channels equal in size to p input

    patient_thread_args* ptas [p];
    for (int i = 0 ; i < p ; i++){
      ptas[i]->patientNumber = i;
      ptas[i]->requestCount = n;
      ptas[i]->requestBuffer = & request_buffer;
    }

    //establish worker_thread_args


    //array of worker thread chanels equal to input
    worker_thread_args* wtas [w];
    for (int i = 0 ; i < w ; i++){
      wtas[i]->requestBuffer = & request_buffer;
      for(int j = 0 ; j < w ; j++){
        wtas[i]->statBuffer[j] = statBuffer[j];
      }
      MESSAGE_TYPE q = NEWCHANNEL_MSG;
      chan->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));

      string s = chan->cread();
      FIFORequestChannel *workerChannel = new FIFORequestChannel(s, FIFORequestChannel::CLIENT_SIDE);
      wtas[i]->workerChannel = new FIFORequestChannel(s, FIFORequestChannel::CLIENT_SIDE);;

      pthread_t worker;
      //worker_thread_args *args = new worker_thread_args(workerChannel, &request_buffer, &hc);
      worker_thread_args* args = new worker_thread_args (workerChannel, &request_buffer, statBuffer);
      pthread_create(&worker, NULL, worker_thread_function, args);

      //workers.push_back(worker);
      //wtas[i] = args;
    }




    /* Start all threads here */


    pthread_t pts [p];
    for (int i = 0 ; i < p; i++){
      pthread_create (&pts [i], 0, patient_thread_function, &ptas [i]);
    }

    cout << "Running Worker Threads..." << endl;
    pthread_t wts [w];
    for (int i = 0; i < w ; i++){
      pthread_create (&wts[i], 0, worker_thread_function, &wtas [i]);
    }

    for(int i = 3; i < p; i++){
      pthread_join(pts [i], 0);
    }
    cout << "Patient Request Threads Completed" << endl;

    //quit all the worker Channels
    for(int i = 0 ; i < w ; i++){
      MESSAGE_TYPE q = QUIT_MSG;
      //chan->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));
      request_buffer.push((char *) &q, sizeof (MESSAGE_TYPE));
    }
    for(int i = 0 ; i < w ; i++){
      pthread_join (wts[i], 0);
    }
    cout << "Worker Threads Finished" << endl;


	  /* Join all threads here */
    gettimeofday (&end, 0);
	  hc.print ();
    int secs = (end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)/(int) 1e6;
    int usecs = (int)(end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)%((int) 1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micor seconds" << endl;


    //quits the server and then deletes the channel
    MESSAGE_TYPE q = QUIT_MSG;
    chan->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));
    cout << "All Done!!!" << endl;


    //deletes worker and patient Threads
    for (int i = 0 ; i < p ; i++){
      delete ptas[i];
    }
    for(int i = 0 ; i < w; i++){
      delete wtas[i]->workerChannel;
      delete wtas[i];
    }

    delete chan;


}
