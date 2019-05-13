#include "common.h"
#include "reqchannel.h"
#include "BoundedBuffer.h"
#include "Histogram.h"
#include "common.h"

#include "HistogramCollection.h"
#include "FIFOreqchannel.h"
#include "MQRequestChannel.h"
#include "SHMRequestChannel.h"
using namespace std;

//default set to f
//communication method will be
//“f” for FIFO
// “q” for message queue
// “s” for shared memory
string communicationMethod = "f";

class patient_thread_args{
public:
   /*put all the arguments for the patient threads */
   pthread_t thread;
   //added
   //person for whom to get info
   int patientNumber;
   //how many reuests to make
   int requestCount;
   //buffer to put requests into
   BoundedBuffer * requestBuffer;
   string fileName;
   RequestChannel* chan;
   int m;

   patient_thread_args(int PatientNumber, int RequestCount,
      BoundedBuffer* RequestBuffer, string FileName,
      RequestChannel* Chan, int mx){
     requestCount = RequestCount;
     patientNumber = PatientNumber;
     requestBuffer = RequestBuffer;

     fileName = FileName;
     chan = Chan;
     m = mx;
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
   BoundedBuffer *requestBuffer;
   //histogram that is updated by data processes
   HistogramCollection* hc;

   worker_thread_args(FIFORequestChannel* Channel,
     BoundedBuffer* Buffer, HistogramCollection* hcx){
      workerChannel = Channel;
      requestBuffer = Buffer;

      hc = hcx;

   }
   worker_thread_args(){

   }

};

void *patient_thread_function(void *arg) {

  patient_thread_args *temp = (patient_thread_args *)arg;
  if (temp->fileName.length() == 0){
      double seconds = 0.000;
      double increment = 0.004;
      datamsg dm (temp->patientNumber, seconds, 1);
      for (int i = 0; i < temp->requestCount; i++){
          temp->requestBuffer->push((char*) &dm, sizeof (datamsg));
          dm.seconds += increment;
      }
  }else{
      char buf [sizeof (filemsg) + temp->fileName.length() + 1];
      filemsg f (0, 0);
      *(filemsg *) buf = f;
      strcpy (buf + sizeof (filemsg), temp->fileName.c_str());
      temp->chan->cwrite (buf, sizeof (buf));
      __int64_t fs = *(__int64_t *) temp->chan->cread();

      cout << "Going to create " << temp->fileName << endl;
      creat (temp->fileName.c_str(), S_IRWXU);

      filemsg* ptr = (filemsg* ) buf;
      __int64_t rem = fs;
      cout << "m value " << temp->m << endl;
      while (rem > 0){
          int chunksize = (int) min (rem, (__int64_t)temp->m);
          ptr->length = chunksize;
          temp->requestBuffer->push (buf, sizeof (buf));
          ptr->offset += chunksize;
          rem -= chunksize;
      }


  }
    /*
    patient_thread_args *args = (patient_thread_args *)arg;

    double seconds = 0.000;
    double stepSize = 0.004;

    datamsg dm(args->patientNumber, seconds, 1);

    for (int i = 0 ; i < args->requestCount; i ++){
      //arg->requestBuffer->push(args->name);
      args->requestBuffer->push((char*) &dm, sizeof(datamsg));
      dm.seconds += stepSize;
    }
    */

}

void *worker_thread_function(void *arg)
{
    //Function of the Worker Threads
    worker_thread_args *temp = (worker_thread_args *)arg;
    while (true)
    {
        vector<char> vrequest = temp->requestBuffer->pop();
        char* request = vrequest.data();
        temp->workerChannel->cwrite (request, vrequest.size());

        if (*(MESSAGE_TYPE *)request == QUIT_MSG){
            delete temp->workerChannel; // this line will remove the temporary fifo* files
            break;
        }
        int len = 0;
        char* response = temp->workerChannel->cread (&len);

        if (*(MESSAGE_TYPE *)request == DATA_MSG){
            datamsg * dm = (datamsg*) request;
            temp->hc->update (dm->patientNumber, *(double *) response);
        }
        else if (*(MESSAGE_TYPE *)request == FILE_MSG){
            filemsg* f = (filemsg*) request;
            char *filename = (char *)(f+1);
            //cerr << "WT" <<  filename << endl;
            FILE* fp = fopen (filename, "r+");
            if (!fp){
                EXITONERROR ("Could not open file----");
            }
            fseek (fp, f->offset, SEEK_SET);
            fwrite (response, 1, len , fp);
            fclose (fp);
        }
    }

    /*
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

    }
    */
}




/* Main Function */

int main(int argc, char *argv[])
{
    int n = 100;    //default number of requests per "patient"
    int p = 1;      // number of patients [1,15]
    int w = 10;    //default number of worker threads
    int b = 20; 	// default capacity of the request buffer, you should change this default

    int m = MAX_MESSAGE; 	// default capacity of the file buffer
    string strm = to_string (m);
    srand(time_t(NULL));

    //file name
    //if f isn't null, switch modes
    string filename = "";

    //added
    //accept input arguments
    int opt = 0;
    while ((opt = getopt(argc, argv, "n:p:w:b:m:i:")) != -1) {
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
          //method of inter process communication
            case 'i':
                communicationMethod = optarg;
                break;
			}
    }


    int pid = fork();
    //dataserver side
    if (pid == 0){
		// modify this to pass along m
    execl ("dataserver", "dataserver", (char *)"-m", (char *)strm.c_str(), (char *)"-i", (char *)communicationMethod.c_str(), (char *) NULL);

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
    HistogramCollection hc;
    struct timeval start, end;
    RequestChannel* chan = create_channel (communicationMethod, "control", RequestChannel::CLIENT_SIDE, m);
    cout << "Control Channel Initialized" << endl;

    BoundedBuffer request_buffer(b);
    vector<patient_thread_args *> patients;
    vector<worker_thread_args *> workers;

    if (filename.length() == 0){
        for (int i = 0; i < p; i++) // populate request/stat thread vectors
        {
            patient_thread_args *rtemp = new patient_thread_args(i+1, n, &request_buffer, filename, chan, m);
            patients.push_back(rtemp);

            Histogram* h = new Histogram (10, -2, 2);
            hc.add (h);
        }
    }else{
        p = 1;
        patient_thread_args *rtemp = new patient_thread_args(-1, n, &request_buffer, filename, chan, m);
        patients.push_back(rtemp);

    }



    for (int i=0; i<w; i++){
        MESSAGE_TYPE ncm = NEWCHANNEL_MSG;
        chan->cwrite ((char *) &ncm, sizeof (MESSAGE_TYPE));
        string chan_name = chan->cread();
        RequestChannel* wchan = create_channel (communicationMethod, chan_name, RequestChannel::CLIENT_SIDE, m);
        worker_thread_args* temp = new worker_thread_args (wchan, &request_buffer, &hc);
        workers.push_back (temp);
    }

    //struct timeval start, end;
    gettimeofday (&start, 0);


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
      for (int i = 0; i < p; i++){
          pthread_create(&patients[i]->thread, NULL, patient_thread_function, patients[i]);
      }

      for (int i = 0; i < workers.size(); i++){
          pthread_create(&workers[i]->thread, NULL, worker_thread_function, workers[i]);
      }

      for (int i = 0; i < p; i++){
          pthread_join(patients[i]->thread, 0);
      }
      cout << "Quitting request Channels ";
      for (int i = 0; i < w; ++i)
      {
          MESSAGE_TYPE m = QUIT_MSG;
          request_buffer.push((char*)&m, sizeof (MESSAGE_TYPE));
      }

      cout << "Joining Threads... " << endl;

      for (int i = 0; i < w; i++)
      {
          pthread_join(workers[i]->thread, 0);
      }





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
    /*
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
    */



    /* Start all threads here */

    /*
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
    */
    cout << "Worker Threads Finished" << endl;


	  /* Join all threads here */
    gettimeofday (&end, 0);
    int secs = (end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)/(int) 1e6;
    int usecs = (int)(end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)%((int) 1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;


    //quits the server and then deletes the channel
    MESSAGE_TYPE q = QUIT_MSG;
    chan->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));
    cout << "All Done!!!" << endl;

    hc.print ();

    delete chan;
    //threads are already joined

    //deletes worker and patient Threads

    //idk if i need this
    /*
    for (int i = 0 ; i < p ; i++){
      delete ptas[i];
    }
    for(int i = 0 ; i < w; i++){
      delete wtas[i]->workerChannel;
      delete wtas[i];
    }*/




}
