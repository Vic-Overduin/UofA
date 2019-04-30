/*                                                                *
 *	OPERATING SYSTEMS ASSIGNMENT 2 - PAGE REPLACEMENT ALGORITHMS  *
 *              Student 1 Name: 	Zac Colombo					  *
 *				Student 1 ID:		a1706304				 	  *
 *				Student 2 Name: 	Victor Overduin				  *
 *				Student 2 ID:		a1653894					  *
 * 6/10/2018                                                      */

 // Compile:	gcc -std=c11 main.c
 // Arguments:  ./a.out    inputX.trace  <pageSize>   <numPages>    <algorithm>       <interval>
 //               		      INPUT       SIZE OF	   NUMBER OF      SC, ESC,     FOR BIT SHIFTING
 //                           FILE         PAGES        PAGES        ARB, EARB     IN ARB AND EARB

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

 // To Print Detailed Execution
#define printlog true
 // To Print the Web-Sub Results
#define websub true
 // Max Value for ARB and EARB
#define MAX_VAL 999999999999999999


 // This struct defines a page
typedef struct Node_ {
	 // Initialising Standard Vars
	char readWrite[1];
	unsigned long long memAdd;
	unsigned long long pageID;
	int refBit;
	//Additional Vars for ARB and EARB
	char arbHex[8];
	unsigned long long arbDec;
} Node;

 // Global Variables
int tableLength = 0; // length of the page table
int shift = 0;		 // shift counter for the arb byte-shifting
int pageSize = 0;	 // input arg, size of pages
int numPages = 0;	 // input arg, number of pages that can fit in the page table (frames)
char* algorithm;	 // input arg, specified algorithm to execute using
int interval = 0;	 // input arg, interval value for arb
int countEvents = 0; // total events in trace, output value
int reads = 0;		 // total disk reads, output value 
int writes = 0;		 // total disk writes, output value

 // Page Table (128 is max possible size)
Node pages[128];

 // Print the page table in order (most recently added at the bottom)
void printTable() {
	if(strcmp(algorithm, "ARB") == 0 || strcmp(algorithm, "EARB") == 0) {
		printf("///////////////////////////\n");
		for(int i = 0; i < tableLength; ++i) {
			printf("Page %llu\t%llu\t%d\t%s\t%s\n", pages[i].pageID, pages[i].memAdd, pages[i].refBit, pages[i].arbHex, pages[i].readWrite);
		}
		printf("///////////////////////////\n");
	} else {
		printf("///////////////////////////\n");
		for(int i = 0; i < tableLength; ++i) {
			printf("Page %llu\t%llu\t%d\n", pages[i].pageID, pages[i].memAdd, pages[i].refBit);
		}
		printf("///////////////////////////\n");
	}	
}

 // Converts a hex value to the equivalent decimal value
 // Unisgned long long incase of any extremely large hex values, too large for an integer.
unsigned long long hexToDecimal(char hex[]) {
	int length = strlen(hex);
	unsigned long long base = 1;
	unsigned long long result = 0;
	for(int i = length-1; i >= 0; i--) {
		 // for number characters
		if(hex[i] >= '0' && hex[i] <= '9') {
			result += (hex[i] - 48) * base;
			base = base * 16;
		 // for capital letters A-F
		} else if(hex[i] >= 'A' && hex[i] <= 'F') {
			result += (hex[i] - 55) * base;
			base = base * 16;
		 // for lower case letters a-f (test 4)
		} else if(hex[i] >= 'a' && hex[i] <= 'f') {
			result += (hex[i] - 87) * base;
			base = base * 16;
		}
	}
	return result;
}

 // Checks if the page table contains the new page
bool checkIfPage(Node page) {
	for(int i = 0; i < tableLength; ++i) {
		if(pages[i].pageID == page.pageID) {
			return true;
		}
	}
	return false;
}

 // Shifts the arb reference byte to the right and adds the new bit infront
 // Input the index of the page from the page table
void shiftBit(int index) {
	char newByte[8];
	char newBit;
	 // determine what the new head bit will be
	if(pages[index].refBit == 0) {
		newBit = '0';
	} else {
		newBit = '1';
		pages[index].refBit = 0;
	}
	 // copy the bits in the new correct order
	for(int i = 0; i < 8; i++) {
		if(i == 0) {
			newByte[i] = newBit;
		} else {
			newByte[i] = pages[index].arbHex[i-1];
		}
	}
	strcpy(pages[index].arbHex, newByte);
	pages[index].arbDec = hexToDecimal(pages[index].arbHex);
}

 // Replacement for ESC algorithm (shifting multiple pages to the back of the table)
 // Input index i which needs to be shifted, and the page that needs to be added
void replaceAtIndex(int i, Node page) {
	 // base case, only need to move the front element, like in SC
	if(i == 0) {
		for(int j = i; j < tableLength-1; j++) {
			pages[j] = pages[j+1];
		}
		pages[tableLength-1] = page;
	 // more than 1 page needs to be shifted
	} else {
		Node temp[128];
		Node newPages[128];
		 // saves a temp of all pages BEFORE the one being replaced
		for(int j = 0; j < i; j++) {
			temp[j] = pages[j];
		}
		int newPagesIndex = 0;
		 // starts constructing the front of the newly ordered page table
		 // new order starts from the page AFTER the one being replaced 
		for(int j = i+1; j < tableLength; j++) {
			newPages[newPagesIndex] = pages[j];
			newPagesIndex++;
		}
		int tempIndex = 0;
		 // adds in the stored pages to the new page table
		for(int j = newPagesIndex; j < tableLength-1; j++) {
			newPages[j] = temp[tempIndex];
			tempIndex++;
		}
		 // add in new page on the end
		newPages[tableLength-1] = page;
		 // copy the new page table into the old page table (updating global page table)
		for(int j = 0; j < tableLength; j++) {
			pages[j] = newPages[j];
		}
	}
}

 // Function to check if the page table contains only "write" pages
bool checkPageW() {
	for(int i = 0; i < tableLength; i++) {
		if(strcmp(pages[i].readWrite, "R") == 0) {
			return false;
		}
	}
	return true;
}

 // Function to check if the page table contains only "read" pages
bool checkPageR() {
	for(int i = 0; i < tableLength; i++) {
		if(strcmp(pages[i].readWrite, "W") == 0) {
			return false;
		}
	}
	return true;
}

 // Second Chance Page Replacement Algorithm
 // Input is a line from file, in format "W 0400AD2E" or "R 0400AEF2"
void sc(char* line){
	 // initialises new page to be inserted into the page table
	Node page;
	char* token;
	token = strtok(line, " ");
	strcpy(page.readWrite, token);
	token = strtok(NULL, " ");
	page.memAdd = hexToDecimal(token);
	unsigned long long num = page.memAdd;
	page.pageID = num/pageSize;
	page.refBit = 1;
	 // checks if the new page is contained in the page table
	bool isPage = checkIfPage(page);
	 // new page is in page table
	if(isPage == true) {
		if(printlog) { printf("HIT:\t\tpage %llu\n", page.pageID); }
		 // finds the page in the table and updates its reference bit to 1, and readWrite value
		for(int i = 0; i < tableLength; ++i) {
			if(pages[i].pageID == page.pageID) {
				if(strcmp(page.readWrite, "W") == 0) {
					strcpy(pages[i].readWrite, "W");
				}
				pages[i].refBit = 1;
				break;
			}
		}
	 // new page is not in page table
	} else {
		 // reads is incremented
		reads++;
		if(printlog) { printf("MISS:\t\tpage %llu\n", page.pageID); }
		 // if the table is empty, insert new page to front
		if(tableLength == 0) {
			pages[0] = page;
			tableLength++;
		 // if the table is not yet full, insert new page to back
		} else if(tableLength < numPages) {
			int iter = 0;
			while(pages[iter].pageID != 0) {
				iter++;
			}
			pages[iter] = page;
			tableLength++;
		 // if the table is full, we need to replace a page to fit the new page
		} else {
			int iter = 0;
			 // loops through table until it finds a suitable replacement
			while(true) {
				 // if the current page's reference bit is 1, we give this page a second chance
					 // reset reference bit to 0
					 // move page to the back of the table
				if(pages[iter].refBit == 1) {
					pages[iter].refBit = 0;
					Node temp = pages[iter];
					for(int i = iter; i < tableLength-1; i++) {
						pages[i] = pages[i+1];
					}
					pages[tableLength-1] = temp;
					 // head of table is different now, we need to go back one to check it again
					iter--;
				 // if the current pages reference bit is 0, we can replace this page with the new page
				} else if(pages[iter].refBit == 0) {
					if(strcmp(pages[iter].readWrite, "W") == 0) {
						 // dirty case, if the page being replaced is a Write page, we increment the writes
						writes++;
						if(printlog) { printf("REPLACE:\tpage %llu (DIRTY)\n", pages[iter].pageID); }
					} else {
						if(printlog) { printf("REPLACE:\tpage %llu\n", pages[iter].pageID); }
					}
					 // breaks because we are currently pointing to the page we need to replace
					break;
				}
				iter++;
				 // this will maintain a circular search through the table
				if(iter == numPages) {
					iter = 0;
				}
			}
			 // adds in new page to table
			for(int i = iter; i < tableLength-1; i++) {
				pages[i] = pages[i+1];
			}
			pages[tableLength-1] = page;
		}
	}
	//printTable();
}

 // Enhanced Second Chance Page Replacement Algorithm
 // Input is a line from file, in format "W 0400AD2E" or "R 0400AEF2"
void esc(char* line){
	 // initialises new page to be inserted into the page table
	Node page;
	char* token;
	token = strtok(line, " ");
	strcpy(page.readWrite, token);
	token = strtok(NULL, " ");
	page.memAdd = hexToDecimal(token);
	unsigned long long num = page.memAdd;
	page.pageID = num/pageSize;
	page.refBit = 1;
	 // checks if the new page is contained in the page table
	bool isPage = checkIfPage(page);
	 // new page is in page table
	if(isPage == true) {
		if(printlog) { printf("HIT:     page %llu\n", page.pageID); }
		 // finds the page in the table and updates its reference bit to 1, and readWrite value
		for(int i = 0; i < tableLength; ++i) {
			if(pages[i].pageID == page.pageID) {
				if(strcmp(page.readWrite, "W") == 0) {
					strcpy(pages[i].readWrite, "W");
				}
				pages[i].refBit = 1;
				break;
			}
		}
	 // new page is not in page table
	} else {
		 // reads is incremented
		reads++;
		if(printlog) { printf("MISS:    page %llu\n", page.pageID); }
		 // if the table is empty, insert new page to front
		if(tableLength == 0) {
			pages[0] = page;
			tableLength++;
		 // if the table is not yet full, insert new page to back
		} else if(tableLength < numPages) {
			int iter = 0;
			while(pages[iter].pageID != 0) {
				iter++;
			}
			pages[iter] = page;
			tableLength++;
		 // if the table is full, we need to replace a page to fit the new page
		} else {
			bool flag = false;
			bool flag2 = false;
			bool flag3 = false;
			 // first pass through the table, in search for a suitable replacement
			for(int i = 0; i < tableLength; i++) {
				 // <0,0> case, searches entire table for this combination
				if(pages[i].refBit == 0 && strcmp(pages[i].readWrite, "R") == 0) {
					if(printlog) { printf("REPLACE: page %llu\n", pages[i].pageID); }
					replaceAtIndex(i, page);
					flag = true;
					flag2 = true;
					flag3 = true;
					break;
				}
			}
			 // second pass through table, first pass was unsucessful (no <0,0>)
			if(!flag) {
				for(int i = 0; i < tableLength; i++) {
					 // <0,1> case, searches for this combination
					 	 // if found, we replace and shift all pages before it, to the back of the table
					if(pages[i].refBit == 0 && strcmp(pages[i].readWrite, "W") == 0) {
						if(printlog) { printf("REPLACE: page %llu (DIRTY)\n", pages[i].pageID); }
						writes++;
						replaceAtIndex(i, page);
						flag2 = true;
						flag3 = true;
						break;
					} else {
						 // resetting all the reference bits that are passed during the search
						pages[i].refBit = 0;
					}
				}
			}
			 // third pass through table, first and second passes were unsucessful (no <0,0> or <0,1>)
			if(!flag2) {
				for(int i = 0; i < tableLength; i++) {
					 // this pass will do the exact same as pass 1, to again check for <0,0> once more
					if(pages[i].refBit == 0 && strcmp(pages[i].readWrite, "R") == 0) {
						if(printlog) { printf("REPLACE: page %llu\n", pages[i].pageID); }
						replaceAtIndex(i, page);
						flag3 = true;
						break;
					}
				}
			}
			 // fourth pass through table (very rare case), getting here means that the table only contains "W" pages
			if(!flag3) {
				for(int i = 0; i < tableLength; i++) {
					 // this pass will do the exact same as pass 2 (this is guarenteed to find a page to replace)
					if(pages[i].refBit == 0 && strcmp(pages[i].readWrite, "W") == 0) {
						if(printlog) { printf("REPLACE: page %llu (DIRTY)\n", pages[i].pageID); }
						writes++;
						replaceAtIndex(i, page);
						break;
					}
				}
			}
		}
	}
	//printTable();
}

 // Additional Reference Bit Page Replacement Algorithm
 // Input is a line from file, in format "W 0400AD2E" or "R 0400AEF2"
void arb(char line[]){
	 // initialises new page to be inserted into the page table
	Node page;
	char* token;
	token = strtok(line, " ");
	strcpy(page.readWrite, token);
	token = strtok(NULL, " ");
	page.memAdd = hexToDecimal(token);
	page.arbDec = 0;
	strcpy(page.arbHex, "00000000");
	unsigned long long num = page.memAdd;
	page.pageID = num/pageSize;
	page.refBit = 1;
	 // checks if the new page is contained in the page table
	bool isPage = checkIfPage(page);
	 // new page is in page table
	if(isPage == true) {
		if(printlog) { printf("HIT:     page %llu\n", page.pageID); }
		 // finds the page in the table and updates its reference bit to 1, and readWrite value
		for(int i = 0; i < tableLength; ++i) {
			if(pages[i].pageID == page.pageID) {
				if(strcmp(page.readWrite, "W") == 0) {
					strcpy(pages[i].readWrite, "W");
				}
				pages[i].refBit = 1;
				break;
			}
		}
	 // new page is not in page table
	} else {
		 // reads is incremented
		reads++;
		if(printlog) { printf("MISS:    page %llu\n", page.pageID); }
		 // if the table is empty, insert new page to front
		if(tableLength == 0) {
			pages[0] = page;
			tableLength++;
		 // if the table is not yet full, insert new page to back
		} else if(tableLength < numPages) {
			int iter = 0;
			while(pages[iter].pageID != 0) {
				iter++;
			}
			pages[iter] = page;
			tableLength++;
		 // if the table is full, we need to replace a page to fit the new page
		} else {
			Node currentMin;
			currentMin.arbDec = MAX_VAL;
			int minIndex = 0;
			 // search table to find the page with the smallest arb reference byte
			for(int i = 0; i < tableLength; ++i) {
				 // page has not been hit in 8 processes, and the reference bit is 0
				 // this is the page we will replace
				if(pages[i].arbDec == 0 && pages[i].refBit == 0) {
					currentMin = pages[i];
					minIndex = i;
					break;
				}
				 // finding the minimum page
				if(pages[i].arbDec < currentMin.arbDec && pages[i].refBit == 0) {
					currentMin = pages[i];
					minIndex = i;
				} else if(pages[i].arbDec == currentMin.arbDec) {}
			}
			if(strcmp(pages[minIndex].readWrite, "W") == 0) {
				 // dirty case, if the page being replaced is a Write page, we increment the writes
				writes++;
				if(printlog) { printf("REPLACE: page %llu (DIRTY)\n", pages[minIndex].pageID); }
			} else {
				if(printlog) { printf("REPLACE: page %llu\n", pages[minIndex].pageID); }
			}
			 // adds in new page to table
			for(int i = minIndex; i < tableLength-1; i++) {
				pages[i] = pages[i+1];
			}
			pages[tableLength-1] = page;
		}
	}
	 // increment the shift (to compare with interval value)
	shift++;
	 // is it time to shift yet?
	if(shift == interval) {
		 // reset shift counter
		shift = 0;
		 // shift bits for all pages replacement bytes
		for(int i = 0; i < tableLength; i++) {
			shiftBit(i);
		}
	}
	//printTable();
}

 // Enhanced Additional Reference Bit Page Replacement Algorithm
 // Input is a line from file, in format "W 0400AD2E" or "R 0400AEF2"
void earb(char* line){
	 // checks the page table, to see whether it contains only "read" pages, OR only "write" pages
	if(checkPageW() == true || checkPageR() == true) {
		 // if it only contains either all "reads" or all "writes"
		 // run regular ARB on the input line
		arb(line);
	 // page table contains mix of "reads" and "writes"
	 // run enhanced ARB on the input line
	} else {
		 // initialises new page to be inserted into the page table
		Node page;
		char* token;
		token = strtok(line, " ");
		strcpy(page.readWrite, token);
		token = strtok(NULL, " ");
		page.memAdd = hexToDecimal(token);
		page.arbDec = 0;
		strcpy(page.arbHex, "00000000");
		unsigned long long num = page.memAdd;
		page.pageID = num/pageSize;
		page.refBit = 1;
		 // checks if the new page is contained in the page table
		bool isPage = checkIfPage(page);
		 // new page is in page table
		if(isPage == true) {
			if(printlog) { printf("HIT:\t\tpage %llu\n", page.pageID); }
			 // finds the page in the table and updates its reference bit to 1, and readWrite value
			for(int i = 0; i < tableLength; ++i) {
				if(pages[i].pageID == page.pageID) {
					if(strcmp(page.readWrite, "W") == 0) {
						strcpy(pages[i].readWrite, "W");
					}
					pages[i].refBit = 1;
					break;
				}
			}
		 // new page is not in page table
		} else {
			 // reads is incremented
			reads++;
			if(printlog) { printf("MISS:\t\tpage %llu\n", page.pageID); }
			 // if the table is empty, insert new page to front
			if(tableLength == 0) {
				pages[0] = page;
				tableLength++;
			 // if the table is not yet full, insert new page to back
			} else if(tableLength < numPages) {
				int iter = 0;
				while(pages[iter].pageID != 0) {
					iter++;
				}
				pages[iter] = page;
				tableLength++;
			 // if the table is full, we need to replace a page to fit the new page
			} else {
				 // initialising temps and vars for replacement
				int replaceIndex = 0;
				Node currentMinR;
				currentMinR.arbDec = MAX_VAL;
				int minIndexR = 0;
				Node currentMinW;
				currentMinW.arbDec = MAX_VAL;
				int minIndexW = 0;
				 // search table to find the page with the smallest arb reference byte
				for(int i = 0; i < tableLength; ++i) {
					 // finding the minimum page (READ)
					if(pages[i].arbDec < currentMinR.arbDec && pages[i].refBit == 0 && strcmp(pages[i].readWrite, "R") == 0) {
						currentMinR = pages[i];
						minIndexR = i;
					}
					 // finding the minimum page (WRITE)
					if(pages[i].arbDec < currentMinW.arbDec && pages[i].refBit == 0 && strcmp(pages[i].readWrite, "W") == 0) {
						currentMinW = pages[i];
						minIndexW = i;
					}
				}
				int firstOneR = -1;
				 // finds the very first '1' bit in smallest non-modified page's byte
				for(int i = 0; i < 8; i++) {
					if(pages[minIndexR].arbHex[i] == '1') {
						firstOneR = i;
						break;
					}
				}
				int firstOneW = -1;
				 // finds the very first '1' bit in smallest modified page's byte
				for(int i = 0; i < 8; i++) {
					if(pages[minIndexW].arbHex[i] == '1') {
						firstOneW = i;
						break;
					}
				}
				 // if there was no '1' bits in the read page, assign accordingly the page which will be replaced
				 // we prioritize choosing the non-modified page
				if(firstOneR == -1) {
					replaceIndex = minIndexR;
				} else {
					 // checks if theres a write that has no '1' bits in the byte
					if(firstOneW == -1) {
						 // checks 1 last time if a read can be replaced instead of a write
						if(firstOneR >= 4) {
							replaceIndex = minIndexR;
						} else {
							 // write MUST be replaced unfortunately
							replaceIndex = minIndexW;
						}
					 // if 1's were found for both, we compare the difference of the indexes between them
					 // difference of > 3 intervals means that the write page must be replaced
					} else {
						if((firstOneW - firstOneR) > 3) {
							replaceIndex = minIndexW;
						} else {
							replaceIndex = minIndexR;
						}
					}
				}
				if(strcmp(pages[replaceIndex].readWrite, "W") == 0) {
					 // dirty case, if the page being replaced is a Write page, we increment the writes
					writes++;
					if(printlog) { printf("REPLACE:\tpage %llu (DIRTY)\n", pages[replaceIndex].pageID); }
				} else {
					if(printlog) { printf("REPLACE:\tpage %llu\n", pages[replaceIndex].pageID); }
				}
				 // adds in new page to table
				for(int i = replaceIndex; i < tableLength-1; i++) {
					pages[i] = pages[i+1];
				}
				pages[tableLength-1] = page;
			}
		}
		 // increment the shift (to compare with interval value)
		shift++;
		 // is it time to shift yet?
		if(shift == interval) {
			 // reset shift counter
			shift = 0;
			 // shift bits for all pages replacement bytes
			for(int i = 0; i < tableLength; i++) {
				shiftBit(i);
			}
		}
	}
	//printTable();
}		


 // This function will take a line from the input file, ignore comments (#...)
 // and determine which algorithm to process the line with
void processLine(char line[]) {
	if(line[0] == '#') {
		//Comment, ignore...
	} else {
		 // increment events in trace
		countEvents++;
		if(strcmp(algorithm, "SC") == 0) {
			 // Second Chance Algorithm
			sc(line);
		} else if(strcmp(algorithm, "ESC") == 0) {
			 // Enhanced Second Chance Algorithm
			esc(line);
		} else if(strcmp(algorithm, "ARB") == 0) {
			 //  Additonal Reference Bits Algorithm
			arb(line);
		} else if(strcmp(algorithm, "EARB") == 0) {
			 // Enhanced Additonal Reference Bits Algorithm
			earb(line);
		}
	}
}

 // Main function, driver for program
int main(int argC,char* argV[]){
	 // Extract Input File
	char* filename = argV[1];
	FILE *inputFile;
	char line[50];
	inputFile = fopen(filename, "r");
	if (inputFile == NULL) {
		fprintf(stderr, "Can't open input file!\n");
		exit(1);
	}

	 // Parse Input Arguments
	pageSize = atoi(argV[2]);
	numPages = atoi(argV[3]);
	algorithm = argV[4];
	if(argV[5] != 0) {
		interval = atoi(argV[5]);
	}

	 // Gets each line from the input file until we reach the end
	while (fgets(line, sizeof(line), inputFile)) {
		processLine(line);
	}

	 // Print for Websubmission
	if(websub) {
		printf("events in trace:    %d\n", countEvents);
		printf("total disk reads:   %d\n", reads);
		printf("total disk writes:  %d\n", writes);
	}
}
