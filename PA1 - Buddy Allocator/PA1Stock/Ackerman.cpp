/* 
    File: ackerman.c

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 08/02/09

    This file implements the function "ackerman(n,m)", which is used 
    by the "memtest" program in MP1. 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */
    
/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include<iostream>
#include<sys/time.h>
#include<assert.h>
#include "Ackerman.h"
#include <string>
#include <cstring>
#include <sstream>
#include <stdlib.h>
using namespace std;


string Ackerman::get_time_diff(struct timeval * tp1, struct timeval * tp2) {
  /* Returns a string containing the difference, in seconds and micro seconds, between two timevals. */
  long sec = tp2->tv_sec - tp1->tv_sec;
  long musec = tp2->tv_usec - tp1->tv_usec;
  if (musec < 0) {
    musec += (int)1e6;
    sec--;
  }
  stringstream ss;
  ss<< " [sec = "<< sec <<", musec = "<<musec<< "]";
  return ss.str();
}

void Ackerman::test(BuddyAllocator *_ba) {
  /* This is function repeatedly asks the user for the two parameters "n" and "m" to pass to the ackerman function, and invokes the function.
     Before and after the invocation of the ackerman function, the value of the wallclock is taken, and the elapsed time for the computation
     of the ackerman function is output.
  */
  ba = _ba;
  while (true) {
    this->num_allocations = 0;
    cout<<"====================================================================="<<endl;
    cout<<"Please enter parameters n (<=3) and m (<=8) to ackerman function"<<endl;
    cout<<"Enter 0 for either n or m in order to exit."<<endl<<endl;

    int n, m;
    cout <<"  n = "; cin>>n;
    cout<<"  m = "; cin>>m;
    if (!n || !m) 
      break;
    struct timeval tp_start, tp_end; /* Used to compute elapsed time. */
    gettimeofday(&tp_start, 0); // start timer
    int result = Recurse(n, m); // compute ackerman value
    gettimeofday(&tp_end, 0);   // stop timer
    
    cout<<"Ackerman("<<n<<", "<<m<<"): "<<result<<endl; 
    cout<<"Time taken: "<< get_time_diff(&tp_start, &tp_end) << endl;
    cout<<"Number of allocate/free cycles: "<<this->num_allocations<<endl<<endl; 
  }
}

int Ackerman::Recurse(int a, int b) {
/* This is the implementation of the Ackerman function. The function itself is very function is very simple (just two recursive calls). We use it to exercise the
   memory allocator (see "my_alloc" and "my_free"). For this, there are additional calls to "gettimeofday" to measure the elapsed time.
 */

  /* The size "to_alloc" of the region to allocate is computed randomly: */
  int to_alloc =  ((2 << (rand() % 19)) * (rand() % 100)) / 100;
  if  (to_alloc < 4) to_alloc = 4;

  int result = 0;
  char* mem = ba->alloc(to_alloc);  // request memory
  num_allocations++;
  
  if (mem != NULL) {

    // testing the allocated memory
    // generate a random byte to fill the allocated block of memory
    char c = rand() % 128;
    memset(mem, c, to_alloc * sizeof(char));

    if (a == 0)
      result = b + 1;
    else if (b == 0)
      result = Recurse(a - 1, 1);
    else
      result = Recurse(a - 1, Recurse(a, b - 1) );

    // check memory value before deleting
    for (int i = 0; i < to_alloc; i++) {
        if (((char*)mem)[i] != c) {
            cout <<"Memory checking error!"<< endl;
            break;
        }
    }
    ba->free(mem);
  }

  return result;
}

