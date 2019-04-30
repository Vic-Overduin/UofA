/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2018

   Class members for memory

**************************************************************** */

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>
#include <cstdio>
#include <unordered_map>

#include "memory.h"
using namespace std;

// Constructor
memory::memory(bool verbose) {
 
}

// Read a word of data from an address
uint64_t memory::read_doubleword (uint64_t address) {
  
  uint64_t temp;
  long long int block_num = (long long int)(address/(BLOCK_SIZE*8));
  int offset = (int) (address%(BLOCK_SIZE*8));
  int offset_ = (int) (offset/8);
  
  unordered_map<long long int, uint64_t*>::iterator it;
  
  it = Blocks.find(block_num);
  
  if(it == Blocks.end()){
		Blocks.insert(pair<long long int, uint64_t*>(block_num, new uint64_t[BLOCK_SIZE]{0}));
  }
  
  temp = Blocks[block_num][offset_];
  return temp;
}

// Write a word of data to an address, mask contains 1s for bytes to be updated
void memory::write_doubleword (uint64_t address, uint64_t data, uint64_t mask) {
  
  uint64_t temp_data = (data & mask);
  uint64_t temp_old = (this->read_doubleword(address) & (~mask));
  uint64_t new_doubleword = temp_data | temp_old;
  
  int offset = (int) (address%(BLOCK_SIZE*8));
  int offset_ = (int) (offset/8);
  
  long long int block_num = (long long int)(address/(BLOCK_SIZE*8));
  
  unordered_map<long long int, uint64_t*>::iterator it;
  it = Blocks.find(block_num);
  if(it != Blocks.end()){
	  Blocks[block_num][offset_] = new_doubleword;
  }
  
}

// Display the word of data at an address
void memory::show_address (uint64_t address) {
   
  uint64_t temp = read_doubleword(address);
  
  cout << hex << setw(16) << setfill('0') << temp << endl;
}

// Set the word of data at an address
void memory::set_address (uint64_t address, uint64_t data) {
   
  write_doubleword (address, data, 0xFFFFFFFFFFFFFFFF);
}

// Load a hex image file
bool memory::load_file(string file_name, uint64_t &start_address) {
  ifstream input_file(file_name);
  string input;
  unsigned int line_count = 0;
  unsigned int byte_count = 0;
  char record_start;
  char byte_string[3];
  char halfword_string[5];
  unsigned int record_length;
  unsigned int record_address;
  unsigned int record_type;
  unsigned int record_data;
  unsigned int record_checksum;
  bool end_of_file_record = false;
  uint64_t load_address;
  //uint64_t load_data;
  //uint64_t load_mask;
  uint64_t load_base_address = 0x0000000000000000ULL;
  start_address = 0x0000000000000000ULL;
  if (input_file.is_open()) {
    while (true) {
      line_count++;
      input_file >> record_start;
      if (record_start != ':') {
	cout << "Input line " << dec << line_count << " does not start with colon character" << endl;
	return false;
      }
      input_file.get(byte_string, 3);
      sscanf(byte_string, "%x", &record_length);
      input_file.get(halfword_string, 5);
      sscanf(halfword_string, "%x", &record_address);
      input_file.get(byte_string, 3);
      sscanf(byte_string, "%x", &record_type);
      switch (record_type) {
      case 0x00:  // Data record
	for (unsigned int i = 0; i < record_length; i++) {
	  input_file.get(byte_string, 3);
	  sscanf(byte_string, "%x", &record_data);
	  load_address = (load_base_address | (uint64_t)(record_address)) + i;
	  write_byte(load_address, record_data, 0x00000000000000000FF);
	  byte_count++;
	}
	break;
      case 0x01:  // End of file
	end_of_file_record = true;
	break;
      case 0x02:  // Extended segment address (set bits 19:4 of load base address)
	load_base_address = 0x0000000000000000ULL;
	for (unsigned int i = 0; i < record_length; i++) {
	  input_file.get(byte_string, 3);
	  sscanf(byte_string, "%x", &record_data);
	  load_base_address = (load_base_address << 8) | (record_data << 4);
	}
	break;
      case 0x03:  // Start segment address (ignored)
	for (unsigned int i = 0; i < record_length; i++) {
	  input_file.get(byte_string, 3);
	  sscanf(byte_string, "%x", &record_data);
	}
	break;
      case 0x04:  // Extended linear address (set upper halfword of load base address)
	load_base_address = 0x0000000000000000ULL;
	for (unsigned int i = 0; i < record_length; i++) {
	  input_file.get(byte_string, 3);
	  sscanf(byte_string, "%x", &record_data);
	  load_base_address = (load_base_address << 8) | (record_data << 16);
	}
	break;
      case 0x05:  // Start linear address (set execution start address)
	start_address = 0x0000000000000000ULL;
	for (unsigned int i = 0; i < record_length; i++) {
	  input_file.get(byte_string, 3);
	  sscanf(byte_string, "%x", &record_data);
	  start_address = (start_address << 8) | record_data;
	}
	break;
      }
      input_file.get(byte_string, 3);
      sscanf(byte_string, "%x", &record_checksum);
      input_file.ignore();
      if (end_of_file_record)
	break;
    }
    input_file.close();
    cout << dec << byte_count << " bytes loaded, start address = "
	 << setw(16) << setfill('0') << hex << start_address << endl;
    return true;
  }
  else {
    cout << "Failed to open file" << endl;
    return false;
  }
}

void memory::write_byte(uint64_t address, uint64_t data, uint64_t mask){
	
	long long int block_num = (long long int)(address/(BLOCK_SIZE*8));
	
	int offset = (int) (address%(BLOCK_SIZE*8));
	int offset_ = (int) (offset/8);
	int offset_in = offset%8;
	
	unordered_map<long long int, uint64_t*>::iterator it;
	it = Blocks.find(block_num);
	
	uint64_t ms = 0xFF00000000000000 >> (56-offset_in*8);
	uint64_t ms1 = ~ms;
	
	uint64_t md = data << ((offset_in*8));
	
	if(it == Blocks.end()){
		Blocks.insert(pair<long long int, uint64_t*>(block_num, new uint64_t[BLOCK_SIZE]{0}));
	}
	
	uint64_t old_data = Blocks[block_num][offset_];
	
	uint64_t new_word = (old_data & ms1) | md;
	
	Blocks[block_num][offset_] = new_word;  
}