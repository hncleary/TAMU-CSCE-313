/*
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date  : 2/8/19
 */
#include "common.h"
#include "FIFOreqchannel.h"

#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/wait.h>

using namespace std;

int main(int argc, char *argv[]){
  int pid = fork();
  if ( pid == 0 ) {
    char *args[] = { "./dataserver", NULL};
    execvp(args[0], args);
  }
  else {
    int n = 100;    // default number of requests per "patient"
	  int p = 15;		// number of patients
    srand(time_t(NULL));
    //requesting channel for client side with name "control"
    //used to transmit commands and recieve replies to the commands
    // -requesting points -requesting files -requesting new channel creation
    FIFORequestChannel chan ("control", FIFORequestChannel::CLIENT_SIDE);
    //create a buffer thats a char array
    //fit all information that needs to be sent inside the buffer
    //testing request data point
    //char* msg = (char*) datapointreq;
    //*(double*)buffer = ____ ;
    // 1/./c/s/v/"null delimiter"
    //*(string*)(buffer + sizeof(datapointreq)) = filename

    //char* buffer = new char[datapointreq,filename];
    //*(datamsg*)buffer = datapointreq;
    //double* result = (double*) chan.cread();
    //cout << *(double*)ptr << endl;
    datamsg d = datamsg( 2 , .008 , 1);
    string filename = "x1.csv";
    //char* buffer = new char [sizeof(d) + sizeof(filename) + 1];
    char* buffer = new char [sizeof(d)]; // + 1
    *(datamsg*)buffer = d;
    //*(string*)(buffer + sizeof(d)) = filename;
    chan.cwrite( buffer, sizeof(datamsg) );

    char* ptr = chan.cread();
    double data = *(double*)ptr;
    //cout << data << endl;

    //DONE REQUESTING SINGLE DATA POINT

    // start timer.
    struct timeval start, end;
    gettimeofday(&start, NULL);
    // sync the I/O of C and C++.
    ios_base::sync_with_stdio(false);
    //getting the entirety of data points in 1.csv
    ofstream myfile;
    myfile.open("received/x1.csv");
    double i = 0;
  //while ( i < .012){
  cout << "Recieving Data Points..." << endl;
    while ( i < 59.996 ){
      myfile << i << ",";

      datamsg d1 = datamsg(1 , i , 1);
      datamsg d2 = datamsg(1 , i , 2);

      char* buffer1 = new char[sizeof(d1)];
      *(datamsg*)buffer1 = d1;
      chan.cwrite( buffer1, sizeof(datamsg));
      char* ptr1 = chan.cread();
      double data1 = *(double*)ptr1;

      myfile << data1 << ",";

      char* buffer2 = new char[sizeof(d2)];
      *(datamsg*)buffer2 = d2;
      chan.cwrite( buffer2, sizeof(datamsg));
      char* ptr2 = chan.cread();
      double data2 = *(double*)ptr2;

      myfile << data2 << "\n";

      i = i + .004;
    }
    myfile.close();
    cout << "Sucessfully Transferred File Data Points" << endl;


    // stop timer.
    gettimeofday(&end, NULL);
    // Calculating total time taken by the program.
    double time_taken;

    time_taken = (end.tv_sec - start.tv_sec) * 1e6;
    time_taken = (time_taken + (end.tv_usec -
                              start.tv_usec)) * 1e-6;

    cout << "Time taken to transfer all 1.csv datapoints : " << fixed
         << time_taken << setprecision(6);
    cout << " sec" << endl;

    //DONE REQUESTING DATA POINTS

    //get length
    filemsg f = filemsg(0,0);
    char t[] = {'1','.','c','s','v'};
    char* buffer3 = new char[sizeof(filemsg) + sizeof(t) + 1];
    *(filemsg*)buffer3 = f;
    //*(string*)(buffer3 + sizeof(filemsg) ) = textfilename; //use char[] over string
    for(int i = 0 ; i < 5 ; i ++ ) {
      *(char*)(buffer3 + sizeof(filemsg) + i ) = t[i];
    }
    *(char*)(buffer3+sizeof(filemsg)+sizeof(t)) = '\0';
    cout << sizeof(buffer3) << endl;
    chan.cwrite( buffer3, sizeof(filemsg) + sizeof(t) + 1);

    int length;
    //char* fptr = chan.cread(&length);
    char* fptr = chan.cread();
    __int64_t fileLength = *(__int64_t*)fptr;
    cout << fileLength << endl;

  //read data
  // start timer.
  struct timeval start2, end2;
  gettimeofday(&start2, NULL);
  // sync the I/O of C and C++.
  ios_base::sync_with_stdio(false);

  FILE * pFile;
  pFile = fopen("y1.csv","wb");

  int j = 0;
  int k = 256;
  while ( j < fileLength ){
    //FUNCTION
    filemsg f2 = filemsg(j,k);
    char t1 [] {'1','.','c','s','v'};
    char* bufferf = new char[sizeof(filemsg) + sizeof(t1) + 1];
    *(filemsg*)bufferf = f2;
    for(int i = 0 ; i < 5 ; i ++ ) {
      *(char*)(bufferf + sizeof(filemsg) + i ) = t1[i];
    }
    *(char*)(bufferf + sizeof(filemsg) + sizeof(t1)) = '\0';
    chan.cwrite( bufferf, sizeof(filemsg) + sizeof(t1) + 1);
    int length;
    char* fptr2 = chan.cread();
    //int lengthf;
    //lengthf = chan.cread(lengthf);

    fwrite(fptr2, sizeof(char), sizeof(fptr2), pFile);


    cout << j << " , " << k << endl;
    j += 256 + 1;
    if ( fileLength - j < 256 ){
      k = fileLength - j;
    }
  }
fclose(pFile);

double time_taken2;

time_taken2 = (end2.tv_sec - start2.tv_sec) * 1e6;
time_taken2 = (time_taken2 + (end2.tv_usec -
                          start2.tv_usec)) * 1e-6;
/*
gettimeofday(&end, NULL);
// Calculating total time taken by the program.
double time_taken;

time_taken = (end.tv_sec - start.tv_sec) * 1e6;
time_taken = (time_taken + (end.tv_usec -
                          start.tv_usec)) * 1e-6;

cout << "Time taken to transfer all 1.csv datapoints : " << fixed
     << time_taken << setprecision(6);
cout << " sec" << endl;
*/
/*
cout << "Time taken to transfer file 1.csv : " << fixed
     << time_taken2 / 1000000000 << setprecision(6);
cout << " sec" << endl;
*/
/*
while(j < fileLength){
      cout << j << " ," << k<< endl;
      filemsg f2 = filemsg(j,k);
      //char t[] = {'1','.','c','s','v'};
      char* bufferf = new char[sizeof(filemsg) + sizeof(t) + 1];
      *(filemsg*)bufferf = f;
      //*(string*)(buffer3 + sizeof(filemsg) ) = textfilename; //use char[] over string
      for(int i = 0 ; i < 5 ; i ++ ) {
        *(char*)(bufferf + sizeof(filemsg) + i ) = t[i];
      }
      *(char*)(bufferf+sizeof(filemsg)+sizeof(t)) = '\0';
      //cout << sizeof(bufferf) << endl;
      chan.cwrite( bufferf, sizeof(filemsg) + sizeof(t) + 1);

      void* fptr2 = chan.cread();
      //use fwrite
      //myfile << fdata <<

      FILE * pfile;
      if (j == 0 ){
        pfile = fopen("y1.csv", "w"); //w is the write tag, wb for write binary
      }
      else{
        pfile = fopen("y1.csv", "a"); //w is the write tag, wb for write binary
      }
      fwrite(fptr2, sizeof(char), sizeof(fptr2) ,pfile);

      j = j + 256 + 1;
      if(fileLength - j < 256){
        k = fileLength - j;
      }
      else{
        //j = k + 1;
        //k = fileLength - k - 1;
      }

}
*/
//requesting new channel


newchannelmsg ncm = newchannelmsg();
char* ncmbuffer = new char [sizeof(ncm)];
*(newchannelmsg*)ncmbuffer = ncm;
chan.cwrite( ncmbuffer , sizeof(newchannelmsg) );
FIFORequestChannel channel2 ("data1_", FIFORequestChannel::CLIENT_SIDE);

//using newly requested channel
cout << "Testing Data Point Request on New Channel ... " << endl;
cout << "(2,.008,1)" << endl;
datamsg d2 = datamsg( 2 , .008 , 1);
char* buffertest = new char [sizeof(d2)];
*(datamsg*)buffertest = d2;
channel2.cwrite( buffertest, sizeof(datamsg) );
char* ptrtest = channel2.cread();
double datatest = *(double*)ptrtest;
cout << datatest << endl;

cout << "Testing Data Point Request on New Channel ... " << endl;
cout << "(1,.012,2)" << endl;
datamsg d3 = datamsg( 2 , .008 , 1);
char* buffertest2 = new char [sizeof(d3)];
*(datamsg*)buffertest2 = d3;
channel2.cwrite( buffertest2, sizeof(datamsg) );
char* ptrtest2 = channel2.cread();
double datatest2 = *(double*)ptrtest2;
cout << datatest2 << endl;




quitmsg q = quitmsg();
char* quitbuffer = new char [sizeof(q)];
*(quitmsg*)quitbuffer = q;
chan.cwrite( buffer, sizeof(quitmsg) );


}
	return 0;
}
