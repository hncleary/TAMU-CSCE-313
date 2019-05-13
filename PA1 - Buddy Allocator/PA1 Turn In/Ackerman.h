/* 
    File: ackerman.h

    Original Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 08/02/08
*/

#ifndef _ackerman_h_                              /* include file only once */
#define _ackerman_h_
#include "BuddyAllocator.h"
#include <iostream>
using namespace std;

class Ackerman {
    BuddyAllocator* ba;
    unsigned int num_allocations;
public:
    int Recurse(int a, int b);
    string get_time_diff(struct timeval * tp1, struct timeval * tp2);
    void test(BuddyAllocator* _ma);
};


#endif
