#include "Ackerman.h"
#include "BuddyAllocator.h"
#include <unistd.h>
#include <cstdlib>
#include <iostream>


int main(int argc, char ** argv) {

  int basic_block_size = 128; //basic block size
  int memory_length = 524288; //basic memory length 512*1024
  bool compatible = true;
  //getopt for command line arguments
  int c;
  try{
  while((c = getopt(argc, argv, "b:s:")) != -1){
    switch(c) {
      //sets size of basic memory block
      case 'b':
        basic_block_size = atoi(optarg);
        std::cout << "Basic Block Size Set to:" << basic_block_size << std::endl;
        break;

      //sets size of total allowed memory
      case 's':
        memory_length = atoi(optarg);
        std::cout << "Memory Length Set to:" << memory_length << std::endl;
        break;

      case '?':
      std::cout << "Error in Command Arguments" << std::endl;

    }
  }
  if(basic_block_size > memory_length){
    std::cout << "Memory Values are Incompatible" << std::endl;
    compatible = false;
  }

  if(compatible){
    // create memory manager
    BuddyAllocator * allocator = new BuddyAllocator(basic_block_size, memory_length);

    // test memory manager
    Ackerman* am = new Ackerman ();
    am->test(allocator); // this is the full-fledged test.

    // destroy memory manager
    delete allocator;
  }
  }
  catch(...){
    std:cout << "An error caused the program to fail" << std::endl;
  }
}
