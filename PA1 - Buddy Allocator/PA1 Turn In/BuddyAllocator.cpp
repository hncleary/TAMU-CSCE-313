/*
    File: my_allocator.cpp
*/
#include "BuddyAllocator.h"
#include <iostream>
using namespace std;

//Linked List Definitions
LinkedList::LinkedList(){
	listLength = 0;
}
int LinkedList::getListLength(){
	return listLength;
}
BlockHeader* LinkedList::getHead(){
	return listHead;
}

void LinkedList::insert(BlockHeader* b){
	if( ! listHead ){
		b -> next = nullptr;
	}
	else{
		b-> next  = listHead;
	}
	listHead = b;
	b->isFree = true;
	listLength = listLength + 1;
}

void LinkedList::remove(BlockHeader* b){
	BlockHeader* temp = listHead;
	//look at listHead
	//if listHead is the node to be deleted
	if(temp == b){
		if(temp->next != nullptr){
			listHead = temp->next;
			listLength = listLength -1;
			b->isFree = false;
		}
		else{
			listHead = nullptr;
			listLength = 0;
		}
	}
	//if listHead wasn't the node to be deleted
	//look for b
	else{
		for(uint i = 1; i < listLength; i++){
			if(temp->next == b){
				//next node is b
				if(temp->next->next != nullptr){
					temp->next = temp->next->next;
					listLength = listLength - 1;
					b-> isFree = false;
				}
				else{
					temp->next = nullptr;
					listLength = listLength - 1;
					b-> isFree = false;
				}
			}
			//if not, keep looking for b
			else {
				temp = temp->next;
			}
		}
	}
}


//Budy Allocator Function Definitions

BuddyAllocator::BuddyAllocator (int _basic_block_size, int _total_memory_length){
	int i = 1;
	while((_basic_block_size * i) < _total_memory_length){
		i = i*2;
	}
	maxMemory = _basic_block_size * i;
	//initial block will be maximum memory
	front = new BlockHeader [int(maxMemory)];
	// front = new char [int(maxMemory)];
	//first node points to the head
	BlockHeader* blheader = (BlockHeader*) front;
	blheader->isFree = true;
	blheader->sizeOfBlock = maxMemory;
	blheader->next = nullptr;
	smallestBlock = _basic_block_size;
	//maximum number of smallest blocks available
	freeListLength = int(log2(maxMemory/smallestBlock)) + 1;
	//linked list with length equal to max number of smallest blocks
	freeList = new LinkedList[freeListLength];
	//initialize first node
	freeList[this->nextFreeBlock(maxMemory)].insert(blheader);
}

BuddyAllocator::~BuddyAllocator (){
	//deallocation, array form
	delete [] front;
	delete [] freeList;
}

int BuddyAllocator::nextFreeBlock(uint sizeNeeded){
	// int x = int();
	// x = freeListLength - ( (int(log2(maxMemory/sizeNeeded)) + 1) );
	// return x;

	//issue to be addressed is here
	//receiving arithmetic null error 

	// if(sizeNeeded == 0){
	// 	return 0;
	// }
	// else{
		return freeListLength - ( (int(log2(maxMemory/sizeNeeded)) + 1) );
		// return 0;
	// }
}

BlockHeader* BuddyAllocator::getbuddy (BlockHeader* addr){
	// BlockHeader* blheader = (BlockHeader*) addr;
	uint blockSize = addr->sizeOfBlock;
	void* buddy = ((addr - front) ^ blockSize) + front;
	return ((addr - front) ^ blockSize) + front;
}

bool BuddyAllocator::arebuddies (BlockHeader* block1, BlockHeader* block2){
	// BlockHeader* blockHeader1 = block1;
	// BlockHeader* blockHeader2 = block2;
	//if the second block is free & the block's sizes are equal
	//then the blocks are buddies
	bool x  = block2->isFree && block1->sizeOfBlock == block2->sizeOfBlock;
	return x;
}

BlockHeader* BuddyAllocator::merge (BlockHeader* block1, BlockHeader* block2){
	BlockHeader* leftBuddy;
	BlockHeader* rightBuddy;
	if(block1 > block2 ){
		leftBuddy = block2;
		rightBuddy = block1;
	}
	else if ( block1 < block2){
		leftBuddy = block1;
		rightBuddy = block2;
	}
	uint originalSize = leftBuddy->sizeOfBlock;
	uint newSize = originalSize * 2;

	freeList[this->nextFreeBlock(originalSize)].remove(leftBuddy);
	freeList[this->nextFreeBlock(originalSize)].remove(rightBuddy);
	freeList[this->nextFreeBlock(newSize)].insert(leftBuddy);

	BlockHeader* mergeBlock = leftBuddy;
	mergeBlock->sizeOfBlock = newSize;
	return mergeBlock;
}

BlockHeader* BuddyAllocator::split (BlockHeader* block){
	BlockHeader* leftBuddy = block;
	uint originalSize = leftBuddy->sizeOfBlock;
	//remove original block
	freeList[this->nextFreeBlock(originalSize)].remove(leftBuddy);
	uint splitSize = originalSize / 2;
	BlockHeader* rightBuddy = block + splitSize;
	leftBuddy->sizeOfBlock = splitSize;
	leftBuddy->next = nullptr;
	rightBuddy->sizeOfBlock = splitSize;
	rightBuddy->next = nullptr;
	//insert left block and right block of 1/2 size
	freeList[this->nextFreeBlock(splitSize)].insert(leftBuddy);
	freeList[this->nextFreeBlock(splitSize)].insert(rightBuddy);
	return leftBuddy;
}

//returns the address of the allocated block
char* BuddyAllocator::alloc(int _length) {
	int i = 0;
	int j = 1;
	//sizeof() returns size of the object in bytes
	if(_length > maxMemory - sizeof(BlockHeader)) {
		return nullptr;
	}
	//figure out the nearest power of 2 block size for initiation
	while (( smallestBlock * j) - sizeof(BlockHeader) < _length){
		j = j * 2;
		i = i + 1;
	}
	// for(int k = 0 ; (k * smallestBlock) - sizeof(BlockHeader) < _length ; k = k * 2 ){
	// 	j = j * 2;
	// 	i = i + 1;
	// }
	//initial pre-split block size length
	int largestBlock = smallestBlock * j;
		if(freeList[i].getListLength() > 0){
			BlockHeader* block = freeList[i].getHead();
			block->isFree = false;
			block->next = nullptr;
			freeList[i].remove(block);
			char* blockReturn = (char*) block;
			return (char*) (blockReturn + sizeof(BlockHeader));
		}
		else{
			BlockHeader* splitBlock;
			//char* splitBlock;
			while (freeList[i].getListLength() == 0){
				i++;
				if (i >= freeListLength){
					return nullptr;
				}
			}
			int currentBlockSize = freeList[i].getHead()->sizeOfBlock;
			while(largestBlock != currentBlockSize){
				//char* block = (char*) freeList[nextFreeBlock(currentBlockSize)].getHead();
				BlockHeader* block = (BlockHeader*) freeList[nextFreeBlock(currentBlockSize)].getHead();
				splitBlock = this->split(block);
				block = splitBlock;
				// block = (BlockHeader*) splitBlock;
				currentBlockSize = currentBlockSize / 2;
			}
			freeList[nextFreeBlock(currentBlockSize)].remove((BlockHeader*) splitBlock);
			BlockHeader* splitBlockHeader = (BlockHeader*) splitBlock;
			char* splitBlock2 = (char*) splitBlock;
			return (splitBlock2 + sizeof(BlockHeader) );
		}
  // hands over to standar c library, handle with own implementation
  // return new char [_length];
}

//return memory to the system that was retrieved in alloc()
int BuddyAllocator::free(char* _a) {
	// get the start of the memory taken up by the blockheader
	 BlockHeader* startingAddress = (BlockHeader*) ( _a - sizeof(BlockHeader) );
	//char* startingAddress =  _a - sizeof(BlockHeader) ;
	// char* startingAddress = _a - sizeof(BlockHeader);
	// BlockHeader* bh = (BlockHeader*) startingAddress;
	BlockHeader* bh =  startingAddress;
	freeList[nextFreeBlock(bh->sizeOfBlock)].insert(bh);
	bool merging = true;
	uint newSize = bh->sizeOfBlock;
	//merge all blocks back together as long as there is > 1 split blocks
	while (merging == true){
		BlockHeader* buddyBlockHeader = getbuddy(startingAddress);
		// BlockHeader* buddyBlockHeader = (BlockHeader*) buddy;
		if(arebuddies(startingAddress, buddyBlockHeader)){
			startingAddress = merge(startingAddress, buddyBlockHeader);
			bh = (BlockHeader*) startingAddress;
		}
		else merging = false;
	}
  // standard C library implementation, fix
  //delete _a;
  //return 0;
	return 0;
}

int BuddyAllocator::getFreeListLength(){
	return freeListLength;
}

LinkedList* BuddyAllocator::getFreeList(){
	return freeList;
}

void BuddyAllocator::debug (){
	int freeListLength = this->getFreeListLength();
	LinkedList* freeList = this->getFreeList();
	for(int i = 0 ; i < freeListLength ; i++){
		if(freeList[i].getListLength() > 0 ){
			BlockHeader* temp = freeList[i].getHead();
			for(int j = 0 ; freeList[i].getListLength() ; j++){
				std::cout << (void*) temp << "-" << std::endl;
				std::cout << freeList[i].getListLength() << std::endl;
				temp = temp->next;
			}
		}
	}
}

// ostream& operator<<(ostream& os, BuddyAllocator& ba)
// {
// 		int freeListLength = ba.getFreeListLength();
// 		LinkedList* freeList = ba.getFreeList();
// 		for(int i = 0 ; i < freeListLength ; i++){
// 			if(freeList[i].getListLength() > 0 ){
// 				BlockHeader* temp = freeList[i].getHead();
// 				for(int j = 0 ; freeList[i].getListLength() ; j++){
// 					os << (void*) temp << "-" << endl;
// 					os << freeList[i].getListLength() << endl;
// 					temp = temp->next;
// 				}
// 			}
// 		}
//     return os;
// }
