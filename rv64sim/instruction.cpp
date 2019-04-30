#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iomanip>
#include <cstdio>
#include "memory.h"
#include "processor.h"
#include "instruction.h"

using namespace std;


uint64_t test = 0x00;

bool checkVisit (uint64_t pc){
	if(pc % 0x0000000000000008 == 0)
		return true;
	return false;
}

int checkOffBits(uint64_t p){
	int steps = 0;
	while(p%0x0000000000000008 != 0){
		p--;
		steps++;
	}
	return steps;
}

uint64_t closet_Mem(uint64_t p){
	uint64_t temp = p;
	while(temp%0x0000000000000008 != 0){
		temp--;
	}
	return temp;
}

uint64_t getOPCODE(uint64_t ir){
	return ir & 0x000000000000007f;
}

uint64_t getFUNC3(uint64_t ir){
	return ((ir & 0x0000000000007000) >> 12);
}

uint64_t getFUNC7(uint64_t ir){
	return (ir & 0x00000000fe000000) >> 25;
}

uint64_t getFUN7_64(uint64_t ir){
	return (ir & 0x00000000fc000000) >> 25;
}

uint64_t getCSR(uint64_t ir){
	return (ir >> 20);
}

uint64_t getRS1(uint64_t ir){
	return (ir >> 15) & 0x1f;
}

uint64_t getRS2(uint64_t ir){
    return ((ir & 0x0000000001f00000)>>20);
}

uint64_t getRD(uint64_t ir){
	return (ir >> 7) & 0x1f;
}

uint64_t getImm_I(uint64_t ir){
    return ((ir >> 20) & 0xfff);
}

uint64_t getImm_B(uint64_t ir){
    return (((ir >> 8) & 0xf) << 1) | (((ir >> 25) & 0x3f) << 5) | ((ir & 0x80) << 4) | ((ir & 0x0000000080000000) >> 19);
}

uint64_t getImm_U(uint64_t ir){
    return ((ir & 0x00000000fffff000) >> 12);
}

uint64_t getImm_UJ(uint64_t ir){
    return ((ir & 0x000000007fe00000) >> 20) | (ir & 0x00000000000ff000) | ((ir & 0x0000000000100000) >> 9) | ((ir & 0x0000000080000000) >> 11);
}

uint64_t getImm_S(uint64_t ir){
    return ((ir >> 7) & 0x1f) | (ir >> 25 << 5);
}


int checkType(uint64_t ir){
  ir = getOPCODE(ir);
  if(ir == 0x03 || ir == 0x0f || ir == 0x13 || ir == 0x1b || ir == 0x67 || ir == 0x73){
    return 1;
  }else if(ir == 0x17 || ir == 0x37){
    return 2;
  }else if(ir == 0x23){
    return 3;
  }else if(ir == 0x33 || ir == 0x3b){
    return 4;
  }else if(ir == 0x63){
    return 5;
  }else if(ir == 0x6f){
    return 6;
  }
  return 0;
}


void EXE_ADDI(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    uint64_t rd= getRD(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t imm = getImm_I(ir);

    if((imm & 0x0000000000000800)== 0x0000000000000800){
        imm = imm | 0xfffffffffffff000;
    }

    if(verbose){
        cout << dec <<"addi: rd = " <<rd<< ", rs1 = " << rs1 << ", immed_I = " << hex <<imm<<endl;
    }

    if((imm & 0x0000000000000800) == 0x00000800){
        imm = ~imm;
        imm += 1;
        processor->set_reg(rd,processor->get_reg(rs1) - imm);
    }
	else {
        processor->set_reg(rd,processor->get_reg(rs1) + imm);
    }
    processor->count_cycle(1);
}

void EXE_BNE(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    uint64_t imm = getImm_B(ir);
	//uint64_t rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t rs2 = getRS2(ir);
    if((imm & 0x0000000000001000) == 0x0000000000001000){
        imm = imm | 0xffffffffffffe000;
    }

    uint64_t ret = (int)imm + processor->get_PC();

    if(processor->get_reg(rs1) != processor->get_reg(rs2))
    {
        processor->set_pc(ret -4 );
        processor->count_cycle(1);
    }

    if(verbose){
        cout<< dec  <<"bne: rs1 = " << getRS1(ir)<< ", rs2 = " << getRS2(ir)  << ", immed_B = " << setw(16) << setfill('0') << hex << imm << endl;
    }

    processor->count_cycle(1);
}

void EXE_JALR(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    int rd = getRD(ir);
    int rs1 = getRS1(ir);

    uint64_t imm = getImm_I(ir);
    if((imm & 0x0000000000000800) == 0x0000000000000800){
        imm = imm | 0xfffffffffffff000;
    }
    uint64_t ret = imm + processor->get_reg(rs1);

    int64_t tmp = processor->get_PC();
    processor->set_pc((ret & 0xfffffffffffffffe) -4);
    processor->set_reg(rd,tmp+4);

    if(verbose){
        cout<< dec  <<"jalr: rd = " <<rd << ", rs1 = " << rs1 << ", immed_I = " << setw(16)<<setfill('0') << hex <<getImm_I(ir)<<endl;
    }
    processor->count_cycle(2);
}

void EXE_LUI(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){


    uint64_t imm = (getImm_U(ir) << 12);
    uint64_t rd = getRD(ir);
    uint64_t ret =  imm ;
    if((imm >= 0x80000000)){
        ret = (imm | 0xffffffff00000000);
    }
    if(verbose){
        cout<< dec  <<"lui: rd = " <<rd << ", immed_U = "  << hex <<imm <<endl;
    }
    //X[rd] = imm;
    processor->set_reg(rd,ret);

    processor->count_cycle(1);

}

void EXE_AUIPC(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    uint64_t imm = getImm_U(ir) << 12;
	if((imm & 0x0000000080000000) == 0x0000000080000000){
		imm = imm | 0xffffffff00000000;
	}
    int rd = getRD(ir);
    if(verbose){
        cout<< dec  <<"auipc: rd = " <<rd << ", immed_U = " << setw(16) << setfill('0') << hex << imm << endl;
    }
    uint64_t ret = imm + processor->get_PC();
    //X[rd] = ret;
    processor->set_reg(rd,ret);
    processor->count_cycle(1);
}

void EXE_JAL(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    uint64_t imm = getImm_UJ(ir);
    int rd = getRD(ir);
    if((imm & 0x0000000000010000) == 0x0000000000010000){
        imm = imm | 0xfffffffffffe0000;
    }
    if(verbose){
        cout << dec <<"jal: rd = " <<rd << ", immed_J = " << setw(16)<<setfill('0') << hex <<imm<<endl;
    }
    uint64_t ret = imm + processor->get_PC();
    uint64_t temp = processor->get_PC();
	
    processor->set_pc(ret - 4);
    processor->set_reg(rd,temp + 4);
    processor->count_cycle(2);
}

void EXE_BEQ(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    uint64_t imm = getImm_B(ir);
    //int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t rs2 = getRS2(ir);
    uint64_t ret = 0x00;
    if((imm & 0x0000000000001000) == 0x0000000000001000){
        imm = imm | 0xffffffffffffe000;
    }
    if(verbose){
        cout << dec  <<"beq: rs1 = " <<rs1<< ", rs2 = " << rs2 << ", immed_B = " << setw(16)<<setfill('0') << hex <<imm<<endl;
    }

    ret = imm + processor->get_PC();
    if(X[rs1] == X[rs2]){
        processor->set_pc(ret - 4);
        processor->count_cycle(1);
    }
    processor->count_cycle(1);
}

void EXE_BLT(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    uint64_t imm = getImm_B(ir);
    //uint64_t rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t rs2 = getRS2(ir);
    if((imm & 0x0000000000001000) == 0x0000000000001000){
        imm = imm | 0xffffffffffffe000;
    }
    if(verbose){
        cout<< dec  <<"blt: rs1 = " <<rs1  <<", rs2 = " << rs2 << ", immed_B = " << setw(16) << setfill('0') << hex << imm << endl;
    }

    uint64_t ret = imm + processor->get_PC();
	
    if((int)processor->get_reg(rs1) < (int)processor->get_reg(rs2)){
        processor->set_pc(ret - 4);
        processor->count_cycle(1);
    }
    processor->count_cycle(1);
}

void EXE_BGE(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    uint64_t imm = getImm_B(ir);
    //uint64_t rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t rs2 = getRS2(ir);
    if((imm & 0x0000000000001000) == 0x0000000000001000){
        imm = imm | 0xffffffffffffe000;
    }
    if(verbose){
        cout<< dec  <<"bge: rs1 = " <<rs1 <<", rs2 = " << rs2 << ", immed_B = " << setw(16) << setfill('0') << hex << imm << endl;
    }

    uint64_t ret = imm + processor->get_PC();

    if((int)X[rs1] >= (int)X[rs2]){
        processor->set_pc(ret - 4);
        processor->count_cycle(1);
    }
    processor->count_cycle(1);
}

void EXE_BLTU(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    uint64_t imm = getImm_B(ir);
    //uint64_t rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t rs2 = getRS2(ir);
    if((imm & 0x0000000000001000) == 0x0000000000001000){
        imm = imm | 0xffffffffffffe000;
    }
    if(verbose){
        cout<< dec  <<"bltu: rs1 = "  << rs1 <<", rs2 = " << rs2 << ", immed_B = " << setw(16) << setfill('0') << hex << imm << endl;
    }

    uint64_t ret = imm + processor->get_PC();
	
    if(X[rs1] < X[rs2]){
        processor->set_pc(ret - 4);
        processor->count_cycle(1);
    }
    processor->count_cycle(1);
}

void EXE_BGEU(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    uint64_t imm = getImm_B(ir);
    //uint64_t rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t rs2 = getRS2(ir);
    if((imm & 0x0000000000001000) == 0x0000000000001000){
        imm = imm | 0xffffffffffffe000;
    }
    if(verbose){
        cout << dec <<"bgeu: rs1 = " << rs1 <<", rs2 = " << rs2 << ", immed_B = " << setw(16) << setfill('0') << hex << imm << endl;
    }
    uint64_t ret = imm + processor->get_PC();

    if(processor->get_reg(rs1) >= processor->get_reg(rs2)){
        processor->set_pc(ret - 4);
        processor->count_cycle(1);
    }
    processor->count_cycle(1);
}

void EXE_LB(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    uint64_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
    if((imm & 0x0000000000000800) == 0x0000000000000800){
        imm = imm | 0xfffffffffffff000;
    }
    if(verbose){
        cout<< dec  <<"lb: rd = " <<rd << ", rs1 = " << rs1 << ", immed_I = " << setw(16) << setfill('0') << hex << imm << endl;
    }

    uint64_t p = (imm + X[rs1]);

    uint64_t ret = mem->read_doubleword(p);
	
	int pos = checkOffBits(p);
	
	if(pos<4){
		if(pos<2){
			if(pos<1){
				ret = (ret & 0x00000000000000ff);
			}
			else{
				ret = (ret & 0x000000000000ff00) >> 8;
			}
		}
		else{
			if(pos<3){
				ret = (ret & 0x0000000000ff0000) >> 16;
			}
			else{
				ret = (ret & 0x00000000ff000000) >> 24;
			}
		}
	}
	else{
		if(pos<6){
			if(pos<5){
				ret = (ret & 0x000000ff00000000) >> 32;
			}
			else{
				ret = (ret & 0x0000ff0000000000) >> 40;
			}
		}
		else{
			if(pos<7){
				ret = (ret & 0x00ff000000000000) >> 48;
			}
			else{
				ret = (ret & 0xff00000000000000) >> 56;
			}
		}
	}
	
	if((ret & 0x0000000000000080) == 0x0000000000000080){
        ret = (ret | 0xffffffffffffff00);
    }
	
	X[rd] = ret;
    processor->set_reg(rd,ret);
    processor->count_cycle(3);
}

void EXE_LH(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    uint64_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
    if(processor->get_reg(rs1)%2==0){
        if((imm & 0x0000000000000800) == 0x0000000000000800){
            imm = imm | 0xfffffffffffff000;
        }
        if(verbose){
            cout<< dec  <<"lh: rd = " <<rd << ", rs1 = " << rs1 << ", immed_I = " << setw(16)<<setfill('0') << hex <<imm<<endl;
        }

        uint64_t p = (imm + X[rs1]);

        uint64_t ret = mem->read_doubleword(p);

        int pos = checkOffBits(p);


        if(pos<4){
            if(pos < 2) {
                ret = ret & 0x000000000000ffff;
            } else {
                ret = (ret & 0x00000000ffff0000) >> 16;
            }
        }else{
            if(pos < 6) {
                ret = (ret & 0x0000ffff00000000) >> 32;
            } else {
                ret = (ret & 0xffff000000000000) >> 48;
            }
        }

        if((ret & 0x0000000000008000) == 0x0000000000008000)
            ret = ret | 0xffffffffffff0000;

        if(verbose){
            cout << "X[rs1]: " << hex << X[rs1] << "  , p:" << hex << p << endl;
            cout << "p%0x08:  " << dec << p%0x0000000000000008 << endl;
            cout << "pos " << pos << endl;
            cout << "Memory read word: address = " << hex << setw(16) << setfill('0') << p  << ", data = "
                 << setw(16) << setfill('0') << hex << mem->read_doubleword(p) << endl;
            cout << "store ret: " << hex << ret << endl;
        }
        X[rd] = ret;
        processor->count_cycle(p%4 ? 5 : 3);
    }

    // If X[rs1] is not aligned
    else{
        processor->ebreak_cause_code = 4;
        processor->set_csr(0x342,0x0000000000000004);
        processor->set_csr(0x341,PC);
        processor->set_csr(0x300,0x0000000200001800);
        processor->set_csr(0x343,processor->get_reg(rs1));
        processor->set_pc(processor->get_csr(0x305));
        processor->ebreak = true;
        processor->instruction_count--;

    }
}

void EXE_LW(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    uint64_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
    // If X[rs1] is aligned
    if(processor->get_reg(rs1)%4==0){
        if((imm & 0x0000000000000800) == 0x0000000000000800){
            imm = imm | 0xfffffffffffff000;
        }
        if(verbose){
            cout<< dec  <<"lw: rd = " <<rd << ", rs1 = " << rs1 << ", immed_I = " << setw(16)<<setfill('0') << hex <<imm<<endl;
        }

        uint64_t p = imm + X[rs1];

        uint64_t ret = mem->read_doubleword(p);

        if(checkVisit(p)){
            ret = (ret & 0x00000000ffffffff);
        }else{
            ret = (ret & 0xffffffff00000000) >> 32;
        }

        if((ret & 0x0000000080000000) == 0x0000000080000000)
            ret = ret | 0xffffffff00000000;

        if(verbose){
            cout << "Memory read word: address = " << hex << setw(16) << setfill('0') << p  << ", data = "
                 << setw(16) << setfill('0') << hex << mem->read_doubleword(p) << endl;
            cout << "store ret: " << hex << ret << endl;
        }
        X[rd] = ret;
        processor->count_cycle(p%4 ? 5 : 3);
    }

    // If X[rs1] is not aligned
    else{
        processor->ebreak_cause_code = 4;
        processor->set_csr(0x342,0x0000000000000004);
        processor->set_csr(0x341,PC);
        processor->set_csr(0x300,0x0000000200001800);
        processor->set_csr(0x343,processor->get_reg(rs1));
        processor->set_pc(processor->get_csr(0x305));
        processor->ebreak = true;
        processor->instruction_count--;

    }
}

void EXE_LD(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
	
    uint64_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);

    // If X[rs1] is aligned
    if(processor->get_reg(rs1)%8==0){

        if((imm&0x0000000000000800) == 0x0000000000000800){
            imm = imm | 0xfffffffffffff000;
        }
        uint64_t p = imm + X[rs1];
        uint64_t ret = mem->read_doubleword(p);


        if(verbose){
            cout << "Memory read word: address = " << hex << setw(16) << setfill('0') << p  << ", data = "
                 << setw(16) << setfill('0') << hex << mem->read_doubleword(p) << endl;
        }
        X[rd] = ret;
        processor->count_cycle(p%4 ? 5 : 3);
    }

    // If X[rs1] is not aligned
    else{
        processor->ebreak_cause_code = 4;
        processor->set_csr(0x342,0x0000000000000004);
        processor->set_csr(0x341,PC);
        processor->set_csr(0x300,0x0000000200001800);
        processor->set_csr(0x343,processor->get_reg(rs1));
        processor->set_pc(processor->get_csr(0x305));
        processor->ebreak = true;
        processor->instruction_count--;

    }
}

void EXE_LBU(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    uint64_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
    if((imm & 0x0000000000000800) == 0x0000000000000800){
        imm = imm | 0xfffffffffffff000;
    }
    if(verbose){
        cout<< dec  <<"lb: rd = " <<rd << ", rs1 = " << rs1 << ", immed_I = " << setw(16) << setfill('0') << hex << imm << endl;
    }

    uint64_t p = (imm + X[rs1]);

    uint64_t ret = mem->read_doubleword(p);
	
	int pos = checkOffBits(p);
	
	if(pos<4){
		if(pos<2){
			if(pos<1){
				ret = (ret & 0x00000000000000ff);
			}
			else{
				ret = (ret & 0x000000000000ff00) >> 8;
			}
		}
		else{
			if(pos<3){
				ret = (ret & 0x0000000000ff0000) >> 16;
			}
			else{
				ret = (ret & 0x00000000ff000000) >> 24;
			}
		}
	}
	else{
		if(pos<6){
			if(pos<5){
				ret = (ret & 0x000000ff00000000) >> 32;
			}
			else{
				ret = (ret & 0x0000ff0000000000) >> 40;
			}
		}
		else{
			if(pos<7){
				ret = (ret & 0x00ff000000000000) >> 48;
			}
			else{
				ret = (ret & 0xff00000000000000) >> 56;
			}
		}
	}
	
    ret = ret & 0x00000000000000ff;
	
	X[rd] = ret;
    processor->set_reg(rd,ret);
    processor->count_cycle(3);
}

void EXE_LHU(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    uint64_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);

    // If X[rs1] is aligned
    if(processor->get_reg(rs1)%2==0){
        if((imm & 0x0000000000000800) == 0x0000000000000800){
            imm = imm | 0xfffffffffffff000;
        }
        if(verbose){
            cout << dec <<"lhu: rd = " <<rd << ", rs1 = " << rs1 << ", immed_I = " << setw(16)<<setfill('0') << hex <<imm<<endl;
        }
        uint64_t p = (imm + X[rs1]);

        uint64_t ret = mem->read_doubleword(p);

        int pos = checkOffBits(p);


        if(pos<4){
            if(pos < 2) {
                ret = ret & 0x000000000000ffff;
            } else {
                ret = (ret & 0x00000000ffff0000) >> 16;
            }
        }else{
            if(pos < 6) {
                ret = (ret & 0x0000ffff00000000) >> 32;
            } else {
                ret = (ret & 0xffff000000000000) >> 48;
            }
        }


        if(verbose){
            cout << "X[rs1]: " << hex << X[rs1] << "  , p:" << hex << p << endl;
            cout << "p%0x08:  " << dec << p%0x0000000000000008 << endl;
            cout << "pos " << pos << endl;
            cout << "Memory read word: address = " << hex << setw(16) << setfill('0') << p  << ", data = "
                 << setw(16) << setfill('0') << hex << mem->read_doubleword(p) << endl;
            cout << "store ret: " << hex << ret << endl;
        }

        ret = ret & 0x000000000000ffff;
        X[rd] = ret;

        processor->count_cycle(p % 4==3 ? 5 : 3);

    }// If X[rs1] is not aligned
    else{
        processor->ebreak_cause_code = 4;
        processor->set_csr(0x342,0x0000000000000004);
        processor->set_csr(0x341,PC);
        processor->set_csr(0x300,0x0000000200001800);
        processor->set_csr(0x343,processor->get_reg(rs1));
        processor->set_pc(processor->get_csr(0x305));
        processor->ebreak = true;
        processor->instruction_count--;

    }
 }

void EXE_LWU(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    uint64_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);

    // If X[rs1] is aligned
    if(processor->get_reg(rs1)%4==0){
        if((imm & 0x0000000000000800) == 0x0000000000000800){
            imm = imm | 0xfffffffffffff000;
        }
        if(verbose){
            cout<< dec  <<"lwu: rd = " <<rd << ", rs1 = " << rs1 << ", immed_I = " << setw(16)<<setfill('0') << hex <<imm<<endl;
        }

        uint64_t p = imm + X[rs1];

        uint64_t ret = mem->read_doubleword(p);

        if(checkVisit(p)){
            ret = (ret & 0x00000000ffffffff);
        }else{
            ret = (ret & 0xffffffff00000000) >> 32;
        }

        ret = ret & 0x00000000ffffffff;

        if(verbose){
            cout << "Memory read word: address = " << hex << setw(16) << setfill('0') << p  << ", data = "
                 << setw(16) << setfill('0') << hex << mem->read_doubleword(p) << endl;
            cout << "store ret: " << hex << ret << endl;
        }
        X[rd] = ret;
        processor->count_cycle(p%4 ? 5 : 3);

    }

    // If X[rs1] is not aligned
    else{
        processor->ebreak_cause_code = 4;
        processor->set_csr(0x342,0x0000000000000004);
        processor->set_csr(0x341,PC);
        processor->set_csr(0x300,0x0000000200001800);
        processor->set_csr(0x343,processor->get_reg(rs1));
        processor->set_pc(processor->get_csr(0x305));
        processor->ebreak = true;
        processor->instruction_count--;

    }
}

void EXE_SB(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    uint64_t imm = getImm_S(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t rs2 = getRS2(ir);
    if((imm & 0x0000000000000800) == 0x0000000000000800){
        imm = imm | 0xfffffffffffff000;
    }
    if(verbose){
        cout << dec <<"sw: rs1 = "<< rs1 << ", rs2 = "<< rs2 << ", immed_S = " << setw(16)<<setfill('0') << hex <<imm<<endl;
    }

    uint64_t ret = imm;
    ret = imm + X[rs1];
    uint64_t mask = 0x00000000000000ff;
	uint64_t data_address = ret;
	uint64_t write_data;
	int pos = checkOffBits(ret);
	
	if (pos <4){
		if (pos<2){
			if (pos<1){
				mask = 0x00000000000000ff;
				write_data = (X[rs2] & 0x00000000000000ff);
			}
			else{
				mask = 0x000000000000ff00;
				write_data = (X[rs2] & 0x00000000000000ff) << 8;
			}
		}
		else{
			if (pos<3){
				mask = 0x0000000000ff0000;
				write_data = (X[rs2] & 0x00000000000000ff) << 16;
			}
			else{
				mask = 0x00000000ff000000;
				write_data = (X[rs2] & 0x00000000000000ff) << 24;
			}
		}
	}	
	else{
		if (pos<6){
			if (pos<5){
				mask = 0x000000ff00000000;
				write_data = (X[rs2] & 0x00000000000000ff) << 32;
			}
			else{
				mask = 0x0000ff0000000000;
				write_data = (X[rs2] & 0x00000000000000ff) << 40;
			}
		}
		else{
			if (pos<7){
				mask = 0x00ff000000000000;
				write_data = (X[rs2] & 0x00000000000000ff) << 48;
			}
			else{
				mask = 0xff00000000000000;
				write_data = (X[rs2] & 0x00000000000000ff) << 56;
			}
		}
	}

	mem->write_doubleword(data_address, write_data, mask);
    //processor->count_cycle(ret % 4 ? 4 : 2);
}

void EXE_SH(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    uint64_t imm = getImm_S(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t rs2 = getRS2(ir);
	
	if(processor->get_reg(rs1)%2==0){
    if((imm & 0x0000000000000800) == 0x0000000000000800){
        imm = imm | 0xfffffffffffff000;
    }
    if(verbose){
        cout << dec <<"sw: rs1 = "<< rs1 << ", rs2 = "<< rs2 << ", immed_S = " << setw(16)<<setfill('0') << hex <<imm<<endl;
    }

    uint64_t ret = imm;
    ret = imm + X[rs1];
    uint64_t mask = 0x000000000000ffff;
	uint64_t data_address = ret;
	uint64_t write_data;
	int pos = checkOffBits(ret);
	
	if (pos <4){
		if (pos<2){
			mask = 0x000000000000ffff;
			write_data = (X[rs2] & 0x000000000000ffff);
		}
		else{
			mask = 0x00000000ffff0000;
			write_data = (X[rs2] & 0x000000000000ffff) << 16;
		}		
	}
	else{
		if (pos<6){
			mask = 0x0000ffff00000000;
			write_data = (X[rs2] & 0x000000000000ffff) << 32;
		}
		else{
			mask = 0xffff000000000000;
			write_data = (X[rs2] & 0x000000000000ffff) << 48;
		}		
	}

	mem->write_doubleword(data_address, write_data, mask);
	}
	else{
		processor->ebreak_cause_code = 6;
		processor->set_csr(0x342,0x0000000000000006);
        processor->set_csr(0x341,PC);
        processor->set_csr(0x300,0x0000000200001800);
        processor->set_csr(0x343,processor->get_reg(rs1));
        processor->set_pc(processor->get_csr(0x305));
        processor->ebreak = true;
        processor->instruction_count--;
	}
}

void EXE_SW(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    uint64_t imm = getImm_S(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t rs2 = getRS2(ir);
	
	if(processor->get_reg(rs1)%4==0){
	
		if((imm & 0x0000000000000800) == 0x0000000000000800){
			imm = imm | 0xfffffffffffff000;
		}
		if(verbose){
			cout << dec <<"sw: rs1 = "<< rs1 << ", rs2 = "<< rs2 << ", immed_S = " << setw(16)<<setfill('0') << hex <<imm<<endl;
		}

		uint64_t ret = imm;
		ret = imm + X[rs1];
		uint64_t mask = 0x00000000ffffffff;
		uint64_t data_address = ret;
		uint64_t write_data;
		int pos = checkOffBits(ret);
	
		if (pos <4){
			mask = 0x00000000ffffffff;
			write_data = (X[rs2] & 0x00000000ffffffff);
		}
		else{
			mask = 0xffffffff00000000;
			write_data = (X[rs2] & 0x00000000ffffffff) << 32;
		}
		mem->write_doubleword(data_address, write_data, mask);
    }
	else{
		processor->ebreak_cause_code = 6;
		processor->set_csr(0x342,0x0000000000000006);
        processor->set_csr(0x341,PC);
        processor->set_csr(0x300,0x0000000200001800);
        processor->set_csr(0x343,processor->get_reg(rs1));
        processor->set_pc(processor->get_csr(0x305));
        processor->ebreak = true;
        processor->instruction_count--;
	}
}

void EXE_SD(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    uint64_t imm = getImm_S(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t rs2 = getRS2(ir);

    // If X[rs1] is aligned
    if(processor->get_reg(rs1)%8==0){
        if((imm & 0x0000000000000800) == 0x0000000000000800){
            imm = imm | 0xfffffffffffff000;
        }
        if(verbose){
            cout << dec <<"sd: rs1 = "<< rs1 << ", rs2 = "<< rs2 << ", immed_S = " << setw(16)<<setfill('0') << hex <<imm<<endl;
        }

        uint64_t ret = imm;
        ret = imm + X[rs1];
        uint64_t mask = 0xffffffffffffffff;


        mem->write_doubleword(ret,X[rs2],mask);



        processor->count_cycle(ret % 4 ? 4 : 2);
    }
    else{
        processor->ebreak_cause_code = 6;
        processor->set_csr(0x342,0x0000000000000006);
        processor->set_csr(0x341,PC);
        processor->set_csr(0x300,0x0000000200001800);
        processor->set_csr(0x343,processor->get_reg(rs1));
        processor->set_pc(processor->get_csr(0x305));
        processor->ebreak = true;
        processor->instruction_count--;

    }
}

void EXE_SLTI(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    int rd = getRD(ir);
	uint64_t imm = getImm_I(ir);
    uint64_t rs1 = getRS1(ir);

    if((imm & 0x0000000000000800) == 0x0000000000000800){
        imm = imm | 0x00000000fffff000;
    }
    if(verbose){
        cout<< dec  <<"slti: rd = " <<rd << ", rs1 = "<< rs1  << ", immed_I = " << setw(16) << setfill('0') << hex << imm << endl;
    }

    uint64_t ret = (int)X[rs1] < (int)imm ? 1 : 0;
    X[rd] = ret;
    processor->count_cycle(1);
}

void EXE_SLTIU(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    int rd = getRD(ir);
	uint64_t imm = getImm_I(ir);
    uint64_t rs1 = getRS1(ir);

    if((imm & 0x0000000000000800) == 0x0000000000000800){
        imm = imm | 0xfffffffffffff000;
    }
    if(verbose){
        cout<< dec  <<"sltiu: rd = " <<rd << ", rs1 = "<< rs1  << ", immed_I = " << setw(16) << setfill('0') << hex << imm << endl;
    }

    uint64_t ret = (uint64_t)X[rs1] < (uint64_t)imm ? 1 : 0;
    X[rd] = ret;
    processor->count_cycle(1);
}

void EXE_ANDI(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    int rd = getRD(ir);
	uint64_t imm = getImm_I(ir);
    uint64_t rs1 = getRS1(ir);
    if((imm & 0x0000000000000800) == 0x0000000000000800){
        imm = imm | 0xfffffffffffff000;
    }
    if(verbose){
        cout << dec <<"andi: rd = " <<rd << ", rs1 = "<< rs1  << ", immed_I = " << setw(16)<<setfill('0') << hex <<imm<<endl;
    }

    uint64_t ret = X[rs1] & imm;
    X[rd] = ret;
    processor->count_cycle(1);
}

void EXE_ORI(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    int rd = getRD(ir);
	uint64_t imm = getImm_I(ir);
    uint64_t rs1 = getRS1(ir);
    if((imm & 0x0000000000000800) == 0x0000000000000800){
        imm = imm | 0xfffffffffffff000;
    }
    if(verbose){
        cout<< dec  <<"ori: rd = " <<rd << ", rs1 = "<< rs1  << ", immed_I = " << setw(16)<<setfill('0') << hex <<imm<<endl;
    }

    uint64_t ret = X[rs1] | imm;

    X[rd] = ret;
    processor->count_cycle(1);


}

void EXE_XORI(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    int rd = getRD(ir);
	uint64_t imm = getImm_I(ir);
    uint64_t rs1 = getRS1(ir);

    if((imm & 0x00000800) == 0x0000000000000800){
        imm = imm | 0xfffffffffffff000;
    }
    if(verbose){
        cout << dec <<"xori: rd = " << rd << ", rs1 = "<< rs1  << ", immed_I = " << setw(16) << setfill('0') << hex << imm << endl;
    }

    uint64_t ret = X[rs1] ^ imm;

    X[rd] = ret;
    processor->count_cycle(1);

}

void EXE_SLLI(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    int rd = getRD(ir);
	uint64_t imm = getImm_I(ir);
    uint64_t rs1 = getRS1(ir);
    if((imm & 0x0000000000000800) == 0x0000000000000800){
        imm = imm | 0x00000000fffff000;
    }
    if(verbose){
        cout<< dec  <<"slli: rd = " <<rd << ", rs1 = "<< rs1  << ", shamt = " << dec <<imm<<endl;
    }
    uint64_t ret = X[rs1] << getImm_I(ir);
    X[rd] = ret;
	
    if(verbose){
        cout<< dec  <<"slli: rd = " <<rd << ", rs1 = "<< rs1  << ", shamt = " << dec <<imm<<endl;
    }
    processor->count_cycle(1);
}

void EXE_SRLI(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    int rd = getRD(ir);
	uint64_t imm = getImm_I(ir);
    uint64_t rs1 = getRS1(ir);
    if((imm & 0x0000000000000800) == 0x0000000000000800){
        imm = imm | 0x00000000fffff000;
    }

    uint64_t ret = X[rs1] >> getImm_I(ir);
    X[rd] = ret;
	
	if(verbose){
        cout << dec  <<"srli: rd = " << rd << ", rs1 = "<< rs1  << ", shamt = " << dec << imm << endl;
    }
    processor->count_cycle(1);
}

void EXE_SRAI(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
	uint64_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);


    uint64_t ret = X[rs1];
    uint64_t bs = imm % 64;
    uint64_t s = ret & 0x8000000000000000;

    for(uint64_t i=0;i<bs;i++){
        ret >>= 1;
        if(s){
            ret |= 0x8000000000000000;
        }
    }

    X[rd] = ret;

    if(verbose){
        cout << dec <<"srai: rd = " <<rd << ", rs1 = "<< rs1  << ", shamt = " << dec <<(imm&0x00000000000000ff)<<endl;
    }
    processor->count_cycle(1);	
}

void EXE_ADD(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t rs2 = getRS2(ir);
    uint64_t ret = X[rs1] + X[rs2];
    X[rd] = ret;
    if(verbose){
        cout << dec <<"add: rd = " << rd << ", rs1 = "<< rs1 << ", rs2 = " << rs2  <<endl;
    }
    processor->count_cycle(1);
}

void EXE_SUB(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t rs2 = getRS2(ir);
    uint64_t ret = X[rs1] - X[rs2];
    X[rd] = ret;
    if(verbose){
        cout<< dec  <<"sub: rd = " << rd << ", rs1 = "<< rs1 << ", rs2 = " << rs2  <<endl;
    }
    processor->count_cycle(1);
}

void EXE_SLL(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t rs2 = getRS2(ir);
    uint64_t ret = X[rs1] << X[rs2];
    X[rd] = ret;
    if(verbose){
        cout<< dec  <<"sll: rd = " << rd << ", rs1 = "<< rs1 << ", rs2 = " << rs2  <<endl;
    }
    processor->count_cycle(1);
}

void EXE_SLT(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t rs2 = getRS2(ir);
    uint64_t ret = (int)X[rs1] < (int)X[rs2] ? 1 : 0;
    X[rd] = ret;
    if(verbose){
        cout<< dec  <<"slt: rd = " << rd << ", rs1 = "<< rs1 << ", rs2 = " << rs2  <<endl;
    }
    processor->count_cycle(1);
}

void EXE_SLTU(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t rs2 = getRS2(ir);
    uint64_t ret = X[rs1] < X[rs2] ? 1 : 0;
    X[rd] = ret;
    if(verbose){
        cout << dec <<"sltu: rd = " << rd << ", rs1 = "<< rs1 << ", rs2 = " << rs2  <<endl;
    }
    processor->count_cycle(1);
}

void EXE_XOR(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t rs2 = getRS2(ir);
    uint64_t ret = X[rs1] ^ X[rs2];
    X[rd] = ret;
    if(verbose){
        cout<< dec  <<"xor: rd = " << rd << ", rs1 = "<< rs1 << ", rs2 = " << rs2  <<endl;
    }
    processor->count_cycle(1);
}

void EXE_SRL(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t rs2 = getRS2(ir);
    uint64_t ret = X[rs1] >> X[rs2];
    X[rd] = ret;
    if(verbose){
        cout<< dec  <<"srl: rd = " << rd << ", rs1 = "<< rs1 << ", rs2 = " << rs2  <<endl;
    }
    processor->count_cycle(1);
}

void EXE_SRA(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t rs2 = getRS2(ir);
    uint64_t ret = X[rs1];

    uint64_t shift = X[rs2] % 64;
    uint64_t z = ret & 0x8000000000000000;

    for(uint64_t i=0; i<shift; i++){
        ret >>= 1;
        if(z) ret |= 0x8000000000000000;
    }

    X[rd] = ret;
    if(verbose){
        cout<< dec  <<"sra: rd = " << rd << ", rs1 = "<< rs1 << ", rs2 = " << rs2  <<endl;
    }
    processor->count_cycle(1);
}

void EXE_OR(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t rs2 = getRS2(ir);
    uint64_t ret = X[rs1] | X[rs2];
    X[rd] = ret;
    if(verbose){
        cout<< dec  <<"or: rd = " << rd << ", rs1 = "<< rs1 << ", rs2 = " << rs2  <<endl;
    }
    processor->count_cycle(1);
}

void EXE_AND(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t rs2 = getRS2(ir);
    uint64_t ret = X[rs1] & X[rs2];
    X[rd] = ret;
    if(verbose){
        cout<< dec  <<"and: rd = " << rd << ", rs1 = "<< rs1 << ", rs2 = " << rs2  <<endl;
    }
    processor->count_cycle(1);
}

void EXE_ADDIW(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    uint64_t rd= getRD(ir);
    uint64_t rs1 = getRS1(ir);
    uint64_t imm = getImm_I(ir);

    if((imm & 0x0000000000000800)== 0x0000000000000800){
        imm = imm | 0xfffffffffffff000;
        //imm = imm | 0x0000000000000fff;
    }

    if(verbose){
        cout << dec <<"addi: rd = " <<rd<< ", rs1 = " << rs1 << ", immed_I = " << hex <<imm<<endl;
    }

    if((imm & 0x0000000000000800) == 0x00000800){
        imm = ~imm;
		imm += 1;
        processor->set_reg(rd,processor->get_reg(rs1) - imm);
    }else {
        processor->set_reg(rd,processor->get_reg(rs1) + imm);
    }
	
	uint64_t old = processor->get_reg(rd);
	
	if((old & 0x0000000080000000) == 0x0000000080000000){
		processor->set_reg(rd,((old | 0xffffffff00000000)& 0xffffffffffffffff));
	}else{
		processor->set_reg(rd,(old & 0x00000000ffffffff));
	}
	
    processor->count_cycle(1);
}

void EXE_SLLIW(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
	uint64_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
	if((imm & 0x0000000000000800)== 0x0000000000000800){
        imm = imm | 0xfffffffffffff000;
    }
	if(verbose){
        cout << dec <<"slli: rd = " <<rd<< ", rs1 = " << rs1 << ", shamt = " << dec << imm << endl;
    }
	uint64_t ret = X[rs1] << getImm_I(ir);
	
	if((ret & 0x0000000080000000) == 0x0000000080000000){
		ret = (ret | 0xffffffff00000000);
	}
	else{
		ret = (ret & 0x00000000ffffffff);
	}
	
	X[rd] = ret;
	processor->count_cycle(1);
}

void EXE_SRLIW(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
	uint64_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);

    if(verbose){
        cout<< dec  <<"srli: rd = " <<rd << ", rs1 = "<< rs1  << ", shamt = " << dec<<imm<<endl;
    }

    uint64_t ret = (X[rs1] & 0x00000000ffffffff);
    uint64_t bs = imm % 32;
    uint64_t s = ret & 0x00000000800000000;

    for(uint64_t i=0;i<bs;i++){
        ret >>= 1;
        if(s){
            ret |= 0x0000000080000000;
        }
    }

    if((ret & 0x0000000080000000) == 0x0000000080000000){
        ret = (ret | 0xffffffff00000000);
    }else{
        ret = (ret & 0x00000000ffffffff);
    }

    X[rd] = ret;
    processor->count_cycle(1);
}

void EXE_SRAIW(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
	uint64_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
	
	uint64_t ret = X[rs1] & 0x00000000ffffffff;
	uint64_t shift = imm % 32;
	uint64_t z = ret & 0x0000000080000000;
	
	for(uint64_t i=0; i<shift; i++){
		ret >>= 1;
		if(z){
			ret |= 0x0000000080000000;
		}
	}
	
	if((ret & 0x0000000080000000) == 0x0000000080000000){
		ret = (ret | 0xffffffff00000000);
	}
	else{
		ret = (ret & 0x00000000ffffffff);
	}
	
	X[rd] = ret;
	
	if(verbose){
        cout << dec <<"srai: rd = " <<rd<< ", rs1 = " << rs1 << ", shamt = " << dec << (imm & 0x00000000000000ff) << endl;
    }
	processor->count_cycle(1);
}

void EXE_ADDW(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
	//uint64_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
	uint64_t rs2 = getRS2(ir);
	uint64_t ret = X[rs1] + X[rs2];
	
	if((ret & 0x0000000080000000) == 0x0000000080000000){
		ret = (ret | 0xffffffff00000000);
	}
	else{
		ret = (ret & 0x00000000ffffffff);
	}
	
	X[rd] = ret;
	
	if(verbose){
		cout << dec << "add: rd = " << rd << ", rs1 = " << rs1 << ", rs2 = " << rs2 << endl;
	}
	processor->count_cycle(1);
}

void EXE_SUBW(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
	//uint64_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
	uint64_t rs2 = getRS2(ir);
	uint64_t ret = X[rs1] - X[rs2];
	
	if((ret & 0x0000000080000000) == 0x0000000080000000){
		ret = (ret | 0xffffffff00000000);
	}
	else{
		ret = (ret & 0x00000000ffffffff);
	}
	
	X[rd] = ret;
	
	if(verbose){
		cout << dec << "sub: rd = " << rd << ", rs1 = " << rs1 << ", rs2 = " << rs2 << endl;
	}
	processor->count_cycle(1);
}

void EXE_SLLW(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
	//uint64_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
	uint64_t rs2 = getRS2(ir);
	uint64_t Lshift = (X[rs2] & 0x00000000000001f);
	uint64_t ret = (X[rs1] & 0x00000000ffffffff);
	
	for(uint64_t i=0; i<Lshift; i++){
		ret <<= 1;
		ret = (ret & 0x00000000ffffffff);
	}

	if((ret & 0x0000000080000000) == 0x0000000080000000){
		ret = (ret | 0xffffffff00000000);
	}
	else{
		ret = (ret & 0x00000000ffffffff);
	}

	X[rd] = ret;
	
	if(verbose){
		cout << dec << "sll: rd = " << rd << ", rs1 = " << rs1 << ", rs2 = " << rs2 << endl;
	}
	processor->count_cycle(1);
}

void EXE_SRLW(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
	int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
	uint64_t rs2 = getRS2(ir);
	uint64_t Rshift = (X[rs2] & 0x000000000000001f);
	uint64_t ret = (X[rs1] & 0x00000000ffffffff);
	
	for(uint64_t i=0; i<Rshift; i++){
		ret >>= 1;
	}
	
	if((ret & 0x0000000080000000) == 0x0000000080000000){
		ret = (ret | 0xffffffff00000000);
	}
	else{
		ret = (ret & 0x00000000ffffffff);
	}
	
	X[rd] = ret;
	if(verbose){
		cout << dec << "sra: rd = " << rd << ", rs1 = " << rs1 << ", rs2 = " << rs2 << endl;
	}
	processor->count_cycle(1);
}

void EXE_SRAW(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
	//uint64_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint64_t rs1 = getRS1(ir);
	uint64_t rs2 = getRS2(ir);
	uint64_t ret = (X[rs1] & 0x00000000ffffffff);
	
	uint64_t shift = (X[rs2] & 0x000000000000001f);
	uint64_t z = ret & 0x0000000080000000;
	
	for(uint64_t i=0; i<shift; i++){
		ret >>= 1;
		if(z){
			ret |= 0x0000000080000000;
		}
	}
	
	if((ret & 0x0000000080000000) == 0x0000000080000000){
		ret = (ret | 0xffffffff00000000);
	}
	else{
		ret = (ret & 0x00000000ffffffff);
	}
		
	X[rd] = ret;
	if(verbose){
		cout << dec << "sra: rd = " << rd << ", rs1 = " << rs1 << ", rs2 = " << rs2 << endl;
	}
	processor->count_cycle(1);
}

void EXE_FENCE(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    if(verbose){
        cout <<" fence: no operation "<<endl;
    }
    processor->count_cycle(1);
}

void EXE_FENCE_DOT_I(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    if(verbose){
        cout<<" fence.i: no operation "<<endl;
    }
    processor->count_cycle(1);
}

void EXE_ECALL(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    processor->ebreak = true;
    if(processor->prv_num==0){
        processor->ebreak_cause_code = 8;

        processor->mcause_reg = 0x0000000000000008;
        processor->mepc_reg = PC;
        processor->mstatus_reg = 0x0000000200000000;
        processor->mtval_reg = 0x0000000000000000;
        processor->set_pc(processor->get_csr(0x305));
    }

    else if(processor->prv_num==3){
        processor->ebreak_cause_code = 11;

        processor->mcause_reg = 0x000000000000000B;
        processor->mepc_reg = PC;
        processor->mstatus_reg = 0x0000000200001800;
        processor->mtval_reg = 0x0000000000000000;
        processor->set_pc(processor->get_csr(0x305));
    }

    processor->instruction_count--;
    // processor->count_cycle(1);
}


void EXE_MRET(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){
    processor->ebreak = true;

    // user mode
    if(processor->prv_num==0){
        // Illegal instruction:
        processor->set_csr(0x341,PC);
        processor->set_csr(0x342,0x0000000000000002);
        processor->set_csr(0x343,ir);
        processor->set_csr(0x300,0x0000000200000000);
        processor->set_pc(processor->get_csr(0x305));

        processor->instruction_count--;

    }

    // machine mode
    else if(processor->prv_num==3){

        processor->set_pc(processor->mepc_reg);
        // mpp = 0 (user)
        if(processor->mstatus_reg==0x0000000200000000){
            processor->mpp = 0;
            processor->set_csr(0x300,0x0000000200000080);
            processor->set_prv(0);
        }
        // mpp = 3 (machine)
        else if(processor->mstatus_reg==0x0000000200001800){
            processor->mpp = 3;
            processor->set_csr(0x300,0x0000000200000080);
        }
        // mpie = 1, mie = 0
        else if(processor->mstatus_reg==0x0000000200001880){
            processor->mpie = 1;
            processor->mie = 0;
            processor->set_csr(0x300,0x0000000200000088);
        }
        // mpie = 0, mie = 1
        else if(processor->mstatus_reg==0x0000000200001808){
            processor->mpie = 0;
            processor->mie = 1;
            processor->set_csr(0x300,0x0000000200000080);
        }
        // mpie = 1, mie = 1
        else if(processor->mstatus_reg==0x0000000200001888){
            processor->mpie = 1;
            processor->mie = 1;
            processor->set_csr(0x300,0x0000000200000088);
        }

    }
    //processor->ebreak = true;
}


void EXE_EBREAK(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){

    processor->ebreak_cause_code = 3;
    //cout <<"ebreak: unimplemented instruction"<<endl;

    // user mode
    if(processor->prv_num==0){
        // mpie = 0;  mie = 1
        if(processor->mstatus_reg==0x0000000200000008){
            processor->mpie = 0;
            processor->mie = 1;
            processor->set_csr(0x300,0x0000000200000080);
        }
        // mpie = 1; mie = 1
        else if(processor->mstatus_reg==0x0000000200000088){
            processor->mpie = 1;
            processor->mie = 1;
            processor->set_csr(0x300,0x0000000200000080);
        }
        else{
            processor->mpie = 0;
            processor->mie = 0;
            processor->set_csr(0x300,0x0000000200000000);
        }

    }

    // machine mode
    else if(processor->prv_num==3){
        // mpie = 0;  mie = 1
        if(processor->mstatus_reg==0x0000000200000008){
            processor->mpie = 0;
            processor->mie = 1;
            processor->set_csr(0x300,0x0000000200001880);
        }
        // mpie = 1; mie = 1
        else if(processor->mstatus_reg==0x0000000200000088){
            processor->mpie = 1;
            processor->mie = 1;
            processor->set_csr(0x300,0x0000000200001880);
        }
        // mie = 1
        else if(processor->mstatus_reg==0x0000000200000008){
            processor->mie = 1;
            processor->set_csr(0x300,0x0000000200001880);
        }

        else{
            processor->set_csr(0x300,0x1800);

            processor->set_csr(0x341,(PC & 0xfffffffffffffffc));
            processor->set_pc((processor->mtvec_reg & 0xfffffffffffffffe));

            if((processor->get_csr(0x305) & 0x01)==0x01){
                // if mie.usie=1, mip.usip=1
                if(processor->get_csr(0x304)==0x01 && processor->get_csr(0x344)==0x01){
                    if(processor->prv_num==3){
                        // Vectored mode
                        processor->set_pc(((processor->mtvec_reg & 0xfffffffffffffffe)+4*0));
                    }
                }
                // if mie.utie=1, mip.utip=1
                else if(processor->get_csr(0x304)==0x10 && processor->get_csr(0x344)==0x10){
                    if(processor->prv_num==3){
                        // Vectored mode
                        processor->set_pc(((processor->mtvec_reg & 0xfffffffffffffffe)+4*4));
                    }
                }
                // if mie.usie=1, mip.usip=1
                else if(processor->get_csr(0x304)==0x100 && processor->get_csr(0x344)==0x100){
                    if(processor->prv_num==3){
                        // Vectored mode
                        processor->set_pc(((processor->mtvec_reg & 0xfffffffffffffffe)+4*8));
                    }            }
                // if mie.msie=1, mip.msip=1
                else if(processor->get_csr(0x304)==0x08 && processor->get_csr(0x344)==0x08){
                    if(processor->prv_num==3){
                        // Vectored mode
                        processor->set_pc(((processor->mtvec_reg & 0xfffffffffffffffe)+4*3));
                    }
                }
                // if mie.mtie=1, mip.mtip=1
                else if(processor->get_csr(0x304)==0x80 && processor->get_csr(0x344)==0x80){
                    if(processor->prv_num==3){
                        // Vectored mode
                        processor->set_pc(((processor->mtvec_reg & 0xfffffffffffffffe)+4*7));
                    }
                }
                // if mie.meie=1, mip.meip=1
                else if(processor->get_csr(0x304)==0x800 && processor->get_csr(0x344)==0x800){
                    if(processor->prv_num==3){
                        // Vectored mode
                        processor->set_pc(((processor->mtvec_reg & 0xfffffffffffffffe)+4*11));
                    }
                }
            }

            processor->set_csr(0x342,(ir & 0x0f));


        }
    }

    processor->instruction_count--;
    processor->set_prv(3);
    processor->ebreak = true;
    //processor->count_cycle(1);
}

void EXE_CSRRW(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){

    uint64_t csr = getCSR(ir);
    uint64_t rd = getRD(ir);

    if(processor->prv_num==0 || csr!=0x340){
        if(getCSR(ir)!=0x340){
            processor->ebreak = true;
            // Illegal instruction:
            processor->set_csr(0x341,PC);
            processor->set_csr(0x342,0x0000000000000002);
            processor->set_csr(0x343,ir);
            processor->set_csr(0x300,0x0000000200001800);
            processor->set_pc(processor->get_csr(0x305));
            processor->instruction_count--;

        }else
            EXE_MRET(mem,processor,ir,X,PC,verbose);


    }else{
        if(rd==0x00){
            processor->set_csr(csr,processor->get_reg(getRS1(ir)));
        }else{
            processor->set_reg(getRD(ir),processor->mscratch_reg);
            processor->set_csr(csr,processor->get_reg(getRS1(ir)));
        }
    }
}

void EXE_CSRRS(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){

    uint64_t csr = getCSR(ir);
    uint64_t rs1 = getRS1(ir);

    if(csr==0xF11||csr==0xF12||csr==0xF13||csr==0xF14){
        //processor->set_reg(getRD(ir),processor->get_csr(csr));
        if(getRS1(ir)==0x00){
            processor->set_reg(getRD(ir),processor->get_csr(csr));
        }

        else if(processor->prv_num==0 || csr!=0x340){
            if(getCSR(ir)!=0x340){
                processor->ebreak = true;
                // Illegal instruction:
                processor->set_csr(0x341,PC);
                processor->set_csr(0x342,0x0000000000000002);
                processor->set_csr(0x343,ir);
                processor->set_csr(0x300,0x0000000200001800);
                processor->set_pc(processor->get_csr(0x305));
                processor->instruction_count--;

            }else
                EXE_MRET(mem,processor,ir,X,PC,verbose);
        }

    }
    else if(csr==0x344){

        if(processor->mip_reg==0x00){
            // for msip
            if(processor->get_reg(rs1)==0x0000000000000008){
                ;
            }
            // for mtip
            else if(processor->get_reg(rs1)==0x0000000000000080){
                ;
            }
            // for meip
            else if(processor->get_reg(rs1)==0x0000000000000800){
                ;
            }
            // for usip
            else if(processor->get_reg(rs1)==0x0000000000000001){
                processor->set_csr(0x344,processor->get_reg(rs1));
            }
            // for utip
            else if(processor->get_reg(rs1)==0x0000000000000010){
                processor->set_csr(0x344,processor->get_reg(rs1));
            }
            // for ueip
            else if(processor->get_reg(rs1)==0x0000000000000100){
                processor->set_csr(0x344,processor->get_reg(rs1));
            }

        }


        else{
            processor->set_reg(getRD(ir),processor->get_csr(0x344));
            processor->set_csr(0x344,0x00);
        }
    }
    else{
        if(rs1==0x00){
            processor->set_csr(csr,(processor->get_reg(getRS1(ir))|processor->mscratch_reg));
        }else{
            processor->set_reg(getRD(ir),processor->mscratch_reg);
            processor->set_csr(csr,(processor->get_reg(getRS1(ir))|processor->mscratch_reg));
        }
    }
}

void EXE_CSRRC(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){

    uint64_t csr = getCSR(ir);
    uint64_t rs1 = getRS1(ir);

    //cout << "csrrc " << hex << csr << " " << rs1 << endl;
    if(csr==0xF11||csr==0xF12||csr==0xF13||csr==0xF14){
        if(rs1==0x00){
            processor->set_reg(getRD(ir),processor->get_csr(csr));
        }

        else if(processor->prv_num==0 || csr!=0x340){
            if(getCSR(ir)!=0x340){
                processor->ebreak = true;
                // Illegal instruction:
                processor->set_csr(0x341,PC);
                processor->set_csr(0x342,0x0000000000000002);
                processor->set_csr(0x343,ir);
                processor->set_csr(0x300,0x0000000200001800);
                processor->set_pc(processor->get_csr(0x305));
                processor->instruction_count--;

            }else
                EXE_MRET(mem,processor,ir,X,PC,verbose);
        }
    }
    else if(csr==0x344){

        if(processor->mip_reg==0x00){

            // for msip
            if(processor->get_reg(rs1)==0x0000000000000008){
                ;
            }
            // for mtip
            else if(processor->get_reg(rs1)==0x0000000000000080){
                ;
            }
            // for meip
            else if(processor->get_reg(rs1)==0x0000000000000800){
                ;
            }
            // for usip
            else if(processor->get_reg(rs1)==0x0000000000000001){
                processor->set_csr(0x344,processor->get_reg(rs1));
            }
            // for utip
            else if(processor->get_reg(rs1)==0x0000000000000010){
                processor->set_csr(0x344,processor->get_reg(rs1));
            }
            // for ueip
            else if(processor->get_reg(rs1)==0x0000000000000100){
                processor->set_csr(0x344,processor->get_reg(rs1));
            }
        }

        else{
            processor->set_reg(getRD(ir),processor->get_csr(0x344));
            processor->set_csr(0x344,0x00);
        }


    }else{
        if(rs1==0x00){
            processor->set_csr(csr,(processor->mscratch_reg - (processor->get_reg(getRS1(ir)) & processor->mscratch_reg)));
        }else{
            processor->set_reg(getRD(ir),processor->mscratch_reg);
            processor->set_csr(csr,(processor->mscratch_reg - (processor->get_reg(getRS1(ir)) & processor->mscratch_reg)));
        }
    }
}

void EXE_CSRRWI(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){

    uint64_t csr = getCSR(ir);
    uint64_t rd = getRD(ir);

    if(rd==0x00){
        processor->set_csr(csr,getRS1(ir));
    }else{
        processor->set_reg(getRD(ir),processor->mscratch_reg);
        processor->set_csr(csr,getRS1(ir));
    }
}

void EXE_CSRRSI(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){

    uint64_t csr = getCSR(ir);
    uint64_t rs1 = getRS1(ir);

    if(csr==0xF11||csr==0xF12||csr==0xF13||csr==0xF14){
        processor->set_reg(getRD(ir),processor->get_csr(csr));
    }else{
        if(rs1==0x00){
            processor->set_reg(getRD(ir),processor->mscratch_reg);
        }else{
            processor->set_reg(getRD(ir),processor->mscratch_reg);
            processor->set_csr(csr,(getRS1(ir)|processor->mscratch_reg));
        }
    }
}

void EXE_CSRRCI(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){

    uint64_t csr = getCSR(ir);
    uint64_t rs1 = getRS1(ir);

    if(csr==0xF11||csr==0xF12||csr==0xF13||csr==0xF14){
        processor->set_reg(getRD(ir),processor->get_csr(csr));
    }else{
        if(rs1==0x00){
            processor->set_reg(getRD(ir),processor->mscratch_reg);
        }else{
            processor->set_reg(getRD(ir),processor->mscratch_reg);
            processor->set_csr(csr,(processor->mscratch_reg - (getRS1(ir) & processor->mscratch_reg)));
        }
    }
}

void EXE_PI(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){

}

void EXE_PD(memory* mem,processor* processor,uint64_t ir, uint64_t* X,uint64_t& PC,bool verbose){

}

uint64_t checkInstruction(memory* memory,processor* processor,uint64_t* X,uint64_t& PC,uint64_t ir,bool verbose){
    
	uint64_t temp = ir;
	
	if(checkVisit(PC)){
		ir = (temp & 0x00000000ffffffff);
		test = temp;
	}
	else{
		ir = (temp & 0xffffffff00000000) >> 32;
	}
	
	if(verbose){
        cout << "Memory read word: address = " << hex << setw(16) << setfill('0') << PC  << ", data = " << setw(16) << setfill('0') << hex << memory->read_doubleword(PC) << endl;
        cout << "Fetch: pc = " <<  setw(16) << setfill('0') << hex << PC << ", ir = " <<  setw(16) << setfill('0') << hex << ir << endl;

    }
	
	if(processor->mstatus_reg!=0x0000000200000000 ||
            (processor->mstatus_reg==0x0000000200000000 && processor->get_csr(0x304)==processor->get_csr(0x344)&&processor->prv_num==0&&processor->get_csr(0x304)!=0x00)){


        if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x00 && getFUNC7(ir) == 0x00 && getRS2(ir) == 0x01){
            EXE_EBREAK(memory,processor,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x00 && getFUNC7(ir) == 0x00 && getRS2(ir) == 0x00){
            EXE_ECALL(memory,processor,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x01){
            EXE_CSRRW(memory,processor,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x02){
            EXE_CSRRS(memory,processor,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x03){
            EXE_CSRRC(memory,processor,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x05){
            EXE_CSRRWI(memory,processor,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x06){
            EXE_CSRRSI(memory,processor,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x07){
            EXE_CSRRCI(memory,processor,ir,X,PC,verbose);
        }else if(ir==0x30200073){
            EXE_MRET(memory,processor,ir,X,PC,verbose);
        }

        else{

            if((processor->get_csr(0x304)==0x01 && processor->get_csr(0x344)==0x01)){
                if(processor->prv_num==0){
                    if(processor->mstatus_reg==0x0000000200000008){
                        processor->set_csr(0x300,0x0000000200000080);
                    }
                    processor->set_prv(3);
                }
                else if(processor->prv_num==3){
                    if(processor->mstatus_reg==0x0000000200000008){
                        processor->set_csr(0x300,0x0000000200001880);
                    }
                }
            }else if((processor->get_csr(0x304)==0x08 && processor->get_csr(0x344)==0x08)){
                if(processor->prv_num==0){
                    if(processor->mstatus_reg==0x0000000200000008){
                        processor->set_csr(0x300,0x0000000200000080);
                    }
                    processor->set_prv(3);
                }
                else if(processor->prv_num==3){
                    if(processor->mstatus_reg==0x0000000200000008){
                        processor->set_csr(0x300,0x0000000200001880);
                    }
                }
            }else if((processor->get_csr(0x304)==0x10 && processor->get_csr(0x344)==0x10)){
                if(processor->prv_num==0){
                    if(processor->mstatus_reg==0x0000000200000008){
                        processor->set_csr(0x300,0x0000000200000080);
                    }
                    processor->set_prv(3);
                }
                else if(processor->prv_num==3){
                    if(processor->mstatus_reg==0x0000000200000008){
                        processor->set_csr(0x300,0x0000000200001880);
                    }
                }
            }else if((processor->get_csr(0x304)==0x80 && processor->get_csr(0x344)==0x80)){
                if(processor->prv_num==0){
                    if(processor->mstatus_reg==0x0000000200000008){
                        processor->set_csr(0x300,0x0000000200000080);
                    }
                    processor->set_prv(3);
                }
                else if(processor->prv_num==3){
                    if(processor->mstatus_reg==0x0000000200000008){
                        processor->set_csr(0x300,0x0000000200001880);
                    }
                }

            }
            else if((processor->get_csr(0x304)==0x100 && processor->get_csr(0x344)==0x100)){
                if(processor->prv_num==0){
                    if(processor->mstatus_reg==0x0000000200000008){
                        processor->set_csr(0x300,0x0000000200000080);
                    }
                    processor->set_prv(3);
                }
                else if(processor->prv_num==3){
                    if(processor->mstatus_reg==0x0000000200000008){
                        processor->set_csr(0x300,0x0000000200001880);
                    }
                }
            }else if((processor->get_csr(0x304)==0x800 && processor->get_csr(0x344)==0x800)){
                if(processor->prv_num==0){
                    if(processor->mstatus_reg==0x0000000200000008){
                        processor->set_csr(0x300,0x0000000200000080);
                    }
                    processor->set_prv(3);
                }
                else if(processor->prv_num==3){
                    if(processor->mstatus_reg==0x0000000200000008){
                        processor->set_csr(0x300,0x0000000200001880);
                    }
                }
            }

            else{
                // mpie = 0, mie = 1
                if(processor->mstatus_reg==0x0000000200000008){
                    processor->set_csr(0x300,0x0000000200001880);
                }

            }
            /* mcause   */
            // if mie.usie=1, mip.usip=1
            if(processor->get_csr(0x304)==0x01 && processor->get_csr(0x344)==0x01){
                processor->set_csr(0x342,0x8000000000000000);
            }
            // if mie.utie=1, mip.utip=1
            else if(processor->get_csr(0x304)==0x10 && processor->get_csr(0x344)==0x10){
                processor->set_csr(0x342,0x8000000000000004);
            }
            // if mie.usie=1, mip.usip=1
            else if(processor->get_csr(0x304)==0x100 && processor->get_csr(0x344)==0x100){
                processor->set_csr(0x342,0x8000000000000008);
            }
            // if mie.msie=1, mip.msip=1
            else if(processor->get_csr(0x304)==0x08 && processor->get_csr(0x344)==0x08){
                processor->set_csr(0x342,0x8000000000000003);
            }
            // if mie.mtie=1, mip.mtip=1
            else if(processor->get_csr(0x304)==0x80 && processor->get_csr(0x344)==0x80){
                processor->set_csr(0x342,0x8000000000000007);
            }
            // if mie.meie=1, mip.meip=1
            else if(processor->get_csr(0x304)==0x800 && processor->get_csr(0x344)==0x800){
                processor->set_csr(0x342,0x800000000000000B);
            }



            /* mcause ends here */



            // mepc
            processor->set_csr(0x341,0x0000000000001000);
            processor->set_pc(processor->get_csr(0x305));

            if((processor->get_csr(0x305) & 0x01)==0x01){
                // if mie.usie=1, mip.usip=1
                if(processor->get_csr(0x304)==0x01 && processor->get_csr(0x344)==0x01){
                    if(processor->prv_num==3){
                        // Vectored mode
                        processor->set_pc(((processor->mtvec_reg & 0xfffffffffffffffe)+4*0));
                    }
                }
                // if mie.utie=1, mip.utip=1
                else if(processor->get_csr(0x304)==0x10 && processor->get_csr(0x344)==0x10){
                    if(processor->prv_num==3){
                        // Vectored mode
                        processor->set_pc(((processor->mtvec_reg & 0xfffffffffffffffe)+4*4));
                    }
                }
                // if mie.usie=1, mip.usip=1
                else if(processor->get_csr(0x304)==0x100 && processor->get_csr(0x344)==0x100){
                    if(processor->prv_num==3){
                        // Vectored mode
                        processor->set_pc(((processor->mtvec_reg & 0xfffffffffffffffe)+4*8));
                    }            }
                // if mie.msie=1, mip.msip=1
                else if(processor->get_csr(0x304)==0x08 && processor->get_csr(0x344)==0x08){
                    if(processor->prv_num==3){
                        // Vectored mode
                        processor->set_pc(((processor->mtvec_reg & 0xfffffffffffffffe)+4*3));
                    }
                }
                // if mie.mtie=1, mip.mtip=1
                else if(processor->get_csr(0x304)==0x80 && processor->get_csr(0x344)==0x80){
                    if(processor->prv_num==3){
                        // Vectored mode
                        processor->set_pc(((processor->mtvec_reg & 0xfffffffffffffffe)+4*7));
                    }
                }
                // if mie.meie=1, mip.meip=1
                else if(processor->get_csr(0x304)==0x800 && processor->get_csr(0x344)==0x800){
                    if(processor->prv_num==3){
                        // Vectored mode
                        processor->set_pc(((processor->mtvec_reg & 0xfffffffffffffffe)+4*11));
                    }
                }
            }
        }


    }
	else{
   
    if(1){
        if	   (getOPCODE(ir) == 0x0000000000000013 && getFUNC3(ir) == 0x0000000000000000){
            EXE_ADDI(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x0000000000000063 && getFUNC3(ir) == 0x0000000000000001){
            EXE_BNE(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x0000000000000067 && getFUNC3(ir) == 0x0000000000000000){
            EXE_JALR(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x0000000000000037){
            EXE_LUI(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x0000000000000017){
            EXE_AUIPC(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x6f){
            EXE_JAL(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x63 && getFUNC3(ir) == 0x0000000000000000){
            EXE_BEQ(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x63 && getFUNC3(ir) == 0x04){
            EXE_BLT(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x63 && getFUNC3(ir) == 0x05){
            EXE_BGE(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x63 && getFUNC3(ir) == 0x06){
            EXE_BLTU(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x63 && getFUNC3(ir) == 0x07){
            EXE_BGEU(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x03 && getFUNC3(ir) == 0x00){
            EXE_LB(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x03 && getFUNC3(ir) == 0x01){
            EXE_LH(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x03 && getFUNC3(ir) == 0x02){
            EXE_LW(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x03 && getFUNC3(ir) == 0x04){
            EXE_LBU(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x03 && getFUNC3(ir) == 0x05){
            EXE_LHU(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x23 && getFUNC3(ir) == 0x00){
            EXE_SB(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x23 && getFUNC3(ir) == 0x01){
            EXE_SH(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x23 && getFUNC3(ir) == 0x02){
            EXE_SW(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x13 && getFUNC3(ir) == 0x02){
            EXE_SLTI(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x13 && getFUNC3(ir) == 0x03){
            EXE_SLTIU(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x13 && getFUNC3(ir) == 0x04){
            EXE_XORI(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x13 && getFUNC3(ir) == 0x06){
            EXE_ORI(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x13 && getFUNC3(ir) == 0x07){
            EXE_ANDI(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x13 && getFUNC3(ir) == 0x01 && (getFUNC7(ir) == 0x00 || getFUN7_64(ir) == 0x00)){
            EXE_SLLI(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x13 && getFUNC3(ir) == 0x05 && (getFUNC7(ir) == 0x00 || getFUN7_64(ir) == 0x00)){
            EXE_SRLI(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x13 && getFUNC3(ir) == 0x05 && (getFUNC7(ir) == 0x20 || getFUN7_64(ir) == 0x20)){
            EXE_SRAI(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x33 && getFUNC3(ir) == 0x00 && getFUNC7(ir) == 0x00){
            EXE_ADD(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x33 && getFUNC3(ir) == 0x00 && getFUNC7(ir) == 0x20){
            EXE_SUB(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x33 && getFUNC3(ir) == 0x01 && getFUNC7(ir) == 0x00){
            EXE_SLL(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x33 && getFUNC3(ir) == 0x02 && getFUNC7(ir) == 0x00){
            EXE_SLT(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x33 && getFUNC3(ir) == 0x03 && getFUNC7(ir) == 0x00){
            EXE_SLTU(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x33 && getFUNC3(ir) == 0x04 && getFUNC7(ir) == 0x00){
            EXE_XOR(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x33 && getFUNC3(ir) == 0x05 && getFUNC7(ir) == 0x00){
            EXE_SRL(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x33 && getFUNC3(ir) == 0x05 && getFUNC7(ir) == 0x20){
            EXE_SRA(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x33 && getFUNC3(ir) == 0x06 && getFUNC7(ir) == 0x00){
            EXE_OR(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x33 && getFUNC3(ir) == 0x07 && getFUNC7(ir) == 0x00){
            EXE_AND(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x0f && getFUNC3(ir) == 0x00){
            EXE_FENCE(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x0f && getFUNC3(ir) == 0x01){
            EXE_FENCE_DOT_I(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x00 && getFUNC7(ir) == 0x00 && getRS2(ir) == 0x00){
            EXE_ECALL(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x00 && getFUNC7(ir) == 0x00 && getRS2(ir) == 0x01){
            EXE_EBREAK(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x01){
            EXE_CSRRW(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x02){
            EXE_CSRRS(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x03){
            EXE_CSRRC(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x05){
            EXE_CSRRWI(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x06){
            EXE_CSRRSI(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x07){
            EXE_CSRRCI(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x1b && getFUNC3(ir) == 0x00){
            EXE_ADDIW(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x1b && getFUNC3(ir) == 0x01 && getFUNC7(ir) == 0x00){
            EXE_SLLIW(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x1b && getFUNC3(ir) == 0x05 && getFUNC7(ir) == 0x00){
            EXE_SRLIW(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x1b && getFUNC3(ir) == 0x05 && getFUNC7(ir) == 0x20){
            EXE_SRAIW(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x3b && getFUNC3(ir) == 0x00 && getFUNC7(ir) == 0x00){
            EXE_ADDW(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x3b && getFUNC3(ir) == 0x00 && getFUNC7(ir) == 0x20){
            EXE_SUBW(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x3b && getFUNC3(ir) == 0x01 && getFUNC7(ir) == 0x00){
            EXE_SLLW(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x3b && getFUNC3(ir) == 0x05 && getFUNC7(ir) == 0x00){
            EXE_SRLW(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x3b && getFUNC3(ir) == 0x05 && getFUNC7(ir) == 0x20){
            EXE_SRAW(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x03 && getFUNC3(ir) == 0x06){
            EXE_LWU(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x03 && getFUNC3(ir) == 0x03){
            EXE_LD(memory,processor,ir,X,PC,verbose);
        }
		else if(getOPCODE(ir) == 0x23 && getFUNC3(ir) == 0x03){
            EXE_SD(memory,processor,ir,X,PC,verbose);
        }
		else if(ir == 0x30200073){
			EXE_MRET(memory,processor,ir,X,PC,verbose);
		}
        else{
            cout << "Illegal instruction: " << setw(8) << setfill('0') << ir  <<  endl;
        }
    }
	else{
        cout << "error instruction" << endl;
    }
	}
}