#ifndef MEMORY_H
#define MEMORY_H

/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2018

   Class for memory

**************************************************************** */

#include <vector>
#include <map>
#include <algorithm>
#include <unordered_map>

#define BLOCK_SIZE 1024

using namespace std;

class memory {

 private:
	bool verbose;
	
	unordered_map<long long int, uint64_t*> Blocks;

 public:

  // Constructor
  memory(bool verbose);
  	 
  // Read a doubleword of data from a doubleword-aligned address
  uint64_t read_doubleword (uint64_t address);

  // Write a doubleword of data to a doubleword-aligned address,
  // mask contains 1s for bytes to be updated
  void write_doubleword (uint64_t address, uint64_t data, uint64_t mask);

  // Display the doubleword of data at an address
  void show_address (uint64_t address);

  // Set the doubleword of data at an address
  void set_address (uint64_t address, uint64_t data);

  // Load a hex image file
  bool load_file(string file_name, uint64_t &start_address);
  
  void write_byte (uint64_t address, uint64_t data, uint64_t mask);

};

#endif
