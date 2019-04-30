/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2018

   Header for instruction

**************************************************************** */

#include "memory.h"
#include "processor.h"

uint64_t getImm_B (uint64_t ir);
uint64_t getImm_U (uint64_t ir);
uint64_t checkInstruction(memory* mem, processor* process, uint64_t* X, uint64_t& PC, uint64_t ir, bool verbose);
int checkType (uint64_t ir);