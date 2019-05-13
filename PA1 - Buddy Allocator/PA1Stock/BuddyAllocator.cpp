/* 
    File: my_allocator.cpp
*/
#include "BuddyAllocator.h"
#include <iostream>
using namespace std;

BuddyAllocator::BuddyAllocator (int _basic_block_size, int _total_memory_length){
	
}

BuddyAllocator::~BuddyAllocator (){
	
}

char* BuddyAllocator::alloc(int _length) {
  /* This preliminary implementation simply hands the call over the 
     the C standard library! 
     Of course this needs to be replaced by your implementation.
  */
  return new char [_length];
}

int BuddyAllocator::free(char* _a) {
  /* Same here! */
  delete _a;
  return 0;
}

void BuddyAllocator::debug (){
  
}

